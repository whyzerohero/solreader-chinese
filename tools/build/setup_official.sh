#!/usr/bin/env bash
# Reproduce toolchain/research prerequisites; no firmware or device action.
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
tool_root=/workspace/toolchains
idf="$tool_root/esp-idf-v5.3.2"
mkdir -p "$tool_root" /workspace/work/official
if [[ ! -e "$idf" ]]; then
    git clone --branch v5.3.2 --depth 1 --shallow-submodules --recursive https://github.com/espressif/esp-idf.git "$idf"
fi
[[ $(git -C "$idf" rev-parse HEAD) == 9d7f2d69f50d1288526d4f1027108e314e8c879f ]] || { echo 'Unexpected IDF version; preserving checkout' >&2; exit 1; }
source_root=/workspace/work/official/embedded-app
if [[ ! -e "$source_root" ]]; then
    python3 "$root/tools/build/fetch_official.py" --repo embedded-app --destination /workspace/work/official
fi
[[ $(git -C "$source_root" rev-parse HEAD) == 7dcf0850daa5cec076c8d68e2f56cf413ff707aa ]] || { echo 'Unexpected official version; preserving checkout' >&2; exit 1; }
[[ -z $(git -C "$source_root" status --porcelain --untracked-files=no) ]] || { echo 'Official tracked files changed; preserving work' >&2; exit 1; }
git -C "$source_root" -c url.https://github.com/.insteadOf=git@github.com: submodule update --init --recursive
export IDF_TOOLS_PATH="$tool_root/idf-tools"
export PIP_CACHE_DIR=/workspace/work/cache/pip
python3 "$idf/tools/idf_tools.py" install cmake ninja xtensa-esp-elf esp32ulp-elf
python3 "$idf/tools/idf_tools.py" install-python-env
