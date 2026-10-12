#!/usr/bin/env bash
# Compile public glue against an explicitly supplied, audited PRIVATE adaptation.
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
ref=${1:?Usage: test_reference.sh /path/to/audited/private/adaptation [output-dir]}
out=${2:-/workspace/work/fix6-adapted-tests}
python3 "$root/tools/build/verify_reference.py" "$ref"
mkdir -p "$out"
flags=(-std=c++17 -Wall -Wextra -Werror -pedantic -O1 -g)
if [[ ${SANITIZE:-0} == 1 ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie); fi
core=(paginator navigation_job navigation encoding progress)
sources=()
for unit in "${core[@]}"; do sources+=("$ref/src/reader/$unit.cpp"); done
"${CXX:-c++}" "${flags[@]}" -I "$root/firmware/solreader_custom/include" -I "$ref/src" \
 "$root/firmware/solreader_custom/src/reference_reader.cpp" "$root/firmware/solreader_custom/src/txt_reader.cpp" \
 "${sources[@]}" "$root/tests/reference_reader_test.cpp" -o "$out/reference-tests"
"$out/reference-tests"
if [[ -n ${FONT_SMOKE_FONT:-} ]]; then
    "${CXX:-c++}" "${flags[@]}" -I "$root/firmware/solreader_custom/include" -I "$ref/src" \
      "$root/firmware/solreader_custom/src/reference_reader.cpp" "$root/firmware/solreader_custom/src/txt_reader.cpp" \
      "${sources[@]}" "$root/tests/reference_font_smoke.cpp" -o "$out/font-smoke"
    python3 "$root/tools/build/font_smoke.py" --font "$FONT_SMOKE_FONT" --binary "$out/font-smoke" --output "$out/font-preview"
fi
