#!/bin/sh
set -eu

CC=${CC:-cc}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP=${TMPDIR:-/tmp}/nolockscreen-tests-$$
CFLAGS="-std=c11 -Wall -Wextra -Werror -I$ROOT"

if [ "${SANITIZE:-0}" = 1 ]; then
  CFLAGS="$CFLAGS -fsanitize=address,undefined -fno-omit-frame-pointer"
fi

mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT HUP INT TERM

$CC $CFLAGS "$ROOT/pattern.c" "$ROOT/tests/pattern_test.c" -o "$TMP/pattern_test"
$CC $CFLAGS "$ROOT/config.c" "$ROOT/tests/config_test.c" -o "$TMP/config_test"
$CC $CFLAGS "$ROOT/config.c" "$ROOT/config_io.c" "$ROOT/tests/config_io_test.c" -o "$TMP/config_io_test"

"$TMP/pattern_test"
"$TMP/config_test"
"$TMP/config_io_test"
