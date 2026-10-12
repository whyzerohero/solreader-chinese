#!/usr/bin/env python3
"""Pinned-source fault injection only. Never imports serial/esptool or opens a device."""
import argparse
import ast
import json
import subprocess
import tempfile
from pathlib import Path
from types import SimpleNamespace

APP_SHA = "7dcf0850daa5cec076c8d68e2f56cf413ff707aa"
IDF_SHA = "9d7f2d69f50d1288526d4f1027108e314e8c879f"
USB_SHA = "20389182dbfd49f600f9fe56b70485f244b48087"


def require(ok, message):
    if not ok:
        raise RuntimeError(message)


def pinned(root, sha):
    actual = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    require(actual == sha, f"Unexpected source HEAD: {root}")
    require(not subprocess.check_output(["git", "-C", str(root), "status", "--porcelain", "--untracked-files=no"], text=True).strip(),
            f"Tracked source changed: {root}")


def function(source, signature):
    start = source.index(signature)
    body = source.index("{", start)
    depth = 1
    end = body + 1
    while depth:
        require(end < len(source), "Unterminated source function")
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


def selected_python(path, names, env):
    tree = ast.parse(path.read_text())
    selected = [node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name in names]
    require(len(selected) == len(names), "Missing audited Python function")
    # No module-level imports or main(), and all I/O globals are supplied mocks.
    exec(compile(ast.Module(body=selected, type_ignores=[]), str(path), "exec"), env)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", type=Path, required=True)
    parser.add_argument("--idf", type=Path, required=True)
    parser.add_argument("--tinyusb", type=Path, required=True)
    args = parser.parse_args()
    for root, sha in [(args.app, APP_SHA), (args.idf, IDF_SHA), (args.tinyusb, USB_SHA)]:
        pinned(root, sha)
    serial = (args.app / "src/Communications/USBSerial.cpp").read_text()
    tiny = (args.tinyusb / "device/esp_tinyusb/tinyusb.c").read_text()
    rollback = (args.app / "src/Misc/BootLoopDetection.cpp").read_text()
    functions = "\n".join([
        function(tiny, "esp_err_t tinyusb_driver_uninstall()"),
        function(serial, "void USBSerial::end()"),
        function(serial, "void USBSerial::initJtag()"),
        function(rollback, "auto PartitionIsBootable("),
    ])
    harness = r'''
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#define CONFIG_USE_TINYUSB_SERIAL 1
#define SOC_USB_SERIAL_JTAG_SUPPORTED 1
#include "hal/usb_phy_types.h"
using esp_err_t=int; using usb_phy_handle_t=std::uintptr_t;
constexpr int ESP_OK=0, ESP_ERR_NO_MEM=0x101, ESP_ERR_INVALID_STATE=0x103;
constexpr int TINYUSB_CDC_ACM_0=0,TINYUSB_CDC_ACM_1=1,ENCRYPTED_BLOCK_SIZE=16,ESP_IMAGE_HEADER_MAGIC=0xe9;
struct usb_phy_config_t {
 usb_phy_controller_t controller; usb_phy_target_t target; usb_otg_mode_t otg_mode;
 usb_phy_speed_t otg_speed; const void*ext_io_conf; const void*otg_io_conf;
};
std::vector<int> trace; bool cdc[2]={true,true}, task_alive=true,phy_in_use=true,config_valid=false;
int delete_result=0,new_result=0,observed_new=-1,read_result=0; bool header_magic=true;
usb_phy_handle_t phy_hdl=1;
void tinyusb_free_descriptors(){trace.push_back(3);} // Invocation only; S3 cleanup is not an HS heap free.
int usb_del_phy(usb_phy_handle_t){trace.push_back(4);if(delete_result==0)phy_in_use=false;return delete_result;}
int tusb_cdc_acm_deinit(int i){trace.push_back(i+1);cdc[i]=false;return 0;}
int usb_new_phy(const usb_phy_config_t*c,usb_phy_handle_t*h){
 trace.push_back(5);config_valid=c->target==USB_PHY_TARGET_INT&&c->controller==USB_PHY_CTRL_SERIAL_JTAG&&c->otg_mode==USB_PHY_MODE_DEFAULT&&c->otg_speed==USB_PHY_SPEED_UNDEFINED;
 observed_new=phy_in_use?ESP_ERR_INVALID_STATE:new_result;
 if(observed_new==0){*h=2;phy_in_use=true;}return observed_new;
}
struct USBSerial {void end();void initJtag();};
struct esp_partition_t{};
int esp_partition_read(const esp_partition_t*,int,void*buf,int n){
 std::memset(buf,0,n);static_cast<std::uint8_t*>(buf)[0]=header_magic?ESP_IMAGE_HEADER_MAGIC:0;return read_result;
}
void check(bool b){if(!b)throw std::runtime_error("fault-injection assertion failed");}
void reset(){trace.clear();cdc[0]=cdc[1]=true;phy_in_use=task_alive=true;delete_result=new_result=0;config_valid=false;}
'''
    checks = r'''
int main(){
 USBSerial u;esp_partition_t part;unsigned tests=0;
 reset();u.initJtag();check(config_valid&&observed_new==0);++tests;
 check(trace==std::vector<int>({1,2,3,4,5}));++tests;
 check(!cdc[0]&&!cdc[1]&&task_alive);++tests; // Uninstall did not stop the modeled running task.
 reset();delete_result=ESP_ERR_INVALID_STATE;u.initJtag();check(observed_new==ESP_ERR_INVALID_STATE&&!cdc[0]);++tests;
 reset();new_result=ESP_ERR_NO_MEM;u.initJtag();check(observed_new==ESP_ERR_NO_MEM&&!cdc[0]&&!phy_in_use);++tests;
 check(!PartitionIsBootable(nullptr));++tests;
 read_result=ESP_OK;header_magic=true;check(!PartitionIsBootable(&part));++tests;
 read_result=1;header_magic=true;check(PartitionIsBootable(&part));++tests; // Mock fills buffer on error: not a hardware claim.
 read_result=1;header_magic=false;check(!PartitionIsBootable(&part));++tests;
 std::cout<<"PASS: "<<tests<<" pinned C++ fault-injection checks; no device I/O\n";
}
'''
    with tempfile.TemporaryDirectory(prefix="solreader-usb-audit-") as temp:
        cpp = Path(temp) / "audit.cpp"
        binary = Path(temp) / "audit"
        cpp.write_text(harness + functions + checks)
        subprocess.run(["c++", "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
                        "-fsanitize=address,undefined", "-fno-pie", "-no-pie",
                        "-I", str(args.idf / "components/hal/include"), str(cpp), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)

    ports = []
    clock = SimpleNamespace(value=0.0)
    def tick(interval):
        clock.value += interval
    env = {"Optional": __import__("typing").Optional,
           "platform": SimpleNamespace(system=lambda: "Windows"),
           "ListPortInfo": object,
           "list_ports": SimpleNamespace(comports=lambda: ports),
           "time": SimpleNamespace(time=lambda: clock.value, sleep=tick)}
    selected_python(args.app / "scripts/port_tool.py",
                    {"port_by_index", "find_ports_matching", "find_ports_by_hwid", "find_ports_by_device_name",
                     "find_usb_jtag_ports", "find_usb_jtag_port", "wait_for_not_none"}, env)
    def fake_port(hwid, name="Mock COM"):
        return SimpleNamespace(device=name,hwid=hwid,description="USB JTAG/serial debug unit")
    ports[:] = [fake_port("USB VID:PID=303A:1001")]
    require(env["find_usb_jtag_port"]() == "Mock COM", "Live COM is discovered")
    ports[:] = []  # Separate mocked PnP node has no COM binding and cannot enter this list.
    require(env["find_usb_jtag_port"]() is None, "No COM cannot establish absence of a USB PnP node")
    ports[:] = [fake_port("USB VID:PID=303A:4002")]
    require(env["find_usb_jtag_port"]() is None, "Runtime port is not JTAG")
    ports[:] = [fake_port("USB VID:PID=303a:1001")]
    require(env["find_usb_jtag_port"]() is None, "HWID match is case sensitive; mock only")
    clock.value = 0.0
    late = lambda: "Mock COM" if clock.value >= 4 else None
    require(env["wait_for_not_none"](late,3) is None, "Three-second wait misses delayed enumeration")
    clock.value = 0.0
    require(env["wait_for_not_none"](late,10) == "Mock COM", "Longer mocked observation finds it")
    print("PASS: 6 original discovery/wait checks on synthetic ports and clock")

    # Official desktop verification contains an erase: run only with mocked CLI and file inputs.
    events = []
    class FakeProcess:
        stdout=SimpleNamespace(read=lambda:"mock")
        stderr=SimpleNamespace(read=lambda:"mock")
        def wait(self):return 0
    desktop = Path("/workspace/work/official/desktop-app")
    pinned(desktop,"650a93af3472d868543fc2e792ea722f02b061d5")
    env = {"parse_flash_script":lambda _: ["esptool.py","write_flash","0x0","bootloader.bin","0xe000","ota_data_initial.bin"],
           "os":SimpleNamespace(path=SimpleNamespace(getsize=lambda _:8192)),
           "esptool_erase_region":lambda *a: events.append(a),
           "logger":SimpleNamespace(info=lambda *_:None,debug=lambda *_:None,error=lambda *_:None),
           "pycli_runner":SimpleNamespace(run_cli=lambda _:FakeProcess()),
           "sys":SimpleNamespace(argv=[]), "esptool":SimpleNamespace(_main=lambda:None)}
    selected_python(desktop / "src/client/firmware.py", {"esptool_verify"}, env)
    require(env["esptool_verify"]("Mock COM","mock.sh"),"Mock verification completed")
    require(events == [("Mock COM","0xe000",8192,"no_reset")],"Verification first erases otadata")
    print("PASS: desktop verification erase demonstrated using mocks only")
    print("TOTAL: 16 checks passed; USB electrical behavior, Windows drivers, eFuse and real device NOT TESTED")

if __name__ == "__main__":
    main()
