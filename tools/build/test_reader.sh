#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
out=${1:-/workspace/work/reader-build}
mkdir -p "$out"
flags=(-std=c++17 -Wall -Wextra -Werror -pedantic -g -O1)
if [[ ${SANITIZE:-0} == 1 ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie); fi
"${CXX:-c++}" "${flags[@]}" -I "$root/firmware/solreader_custom/include" \
  "$root/firmware/solreader_custom/src/txt_reader.cpp" "$root/tests/txt_reader_test.cpp" -o "$out/txt-reader-tests"
"$out/txt-reader-tests" "$out/progress-test.txt"
