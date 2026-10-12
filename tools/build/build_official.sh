#!/usr/bin/env bash
# Offline build only; never invoke upstream Makefile or device actions.
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
source_root=${1:-/workspace/work/official/embedded-app}
output_root=${2:-/workspace/work/official-build}
tools_root=${3:-/workspace/toolchains}
[[ $(git -C "$source_root" rev-parse HEAD) == 7dcf0850daa5cec076c8d68e2f56cf413ff707aa ]] || { echo 'Unexpected official source revision' >&2; exit 1; }
[[ $(git -C "$tools_root/esp-idf-v5.3.2" rev-parse HEAD) == 9d7f2d69f50d1288526d4f1027108e314e8c879f ]] || { echo 'Unexpected IDF revision' >&2; exit 1; }
export IDF_TOOLS_PATH="$tools_root/idf-tools"
export IDF_PATH="$tools_root/esp-idf-v5.3.2"
export IDF_PYTHON_ENV_PATH="$IDF_TOOLS_PATH/python_env/idf5.3_py3.12_env"
export XDG_CACHE_HOME=/workspace/work/cache
export PATH="$IDF_PYTHON_ENV_PATH/bin:$IDF_TOOLS_PATH/tools/cmake/3.30.2/bin:$IDF_TOOLS_PATH/tools/ninja/1.12.1:$IDF_TOOLS_PATH/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin:$IDF_TOOLS_PATH/tools/esp32ulp-elf/2.38_20240113/esp32ulp-elf/bin:$PATH"
cd -- "$source_root"
"$IDF_PYTHON_ENV_PATH/bin/python" "$IDF_PATH/tools/idf.py" \
 -B "$output_root" -D 'SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.glasses-release' \
 -D IDF_TARGET=esp32s3 reconfigure
exec ninja -j4 -C "$output_root"
