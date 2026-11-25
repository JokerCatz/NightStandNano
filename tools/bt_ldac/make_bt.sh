#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-2.1-or-later
# Patched Bluedroid for LDAC / aptX: a fresh copy of $IDF_PATH/components/bt in <repo>/.local/bt_ldac/bt (git-ignored) with
# ldac.patch applied. app/CMakeLists.txt runs this script at configure time and adds the copy as a project component named bt,
# which replaces IDF's. A second run is a no-op while the stamp matches (the patch, this script and the IDF checkout are unchanged);
# `make_bt.sh --force` regenerates anyway. Needs git (the copy gets its own git repo: host/bluedroid tracked, pristine commit first),
# so `git diff` there regenerates the patch after editing:  (cd .local/bt_ldac/bt && git diff) > tools/bt_ldac/ldac.patch
# (Origin: JukeDial tools/bt_ldac, copied 2026-11-25; the patch file is unchanged. See components/ORIGIN.md.)
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
dest="${BT_LDAC_DEST:-$root/.local/bt_ldac/bt}"   # override for a trial run
patch="$here/ldac.patch"
IDF_PATH="${IDF_PATH:-$HOME/.espressif/v6.1/esp-idf}"   # this project: IDF v6.1
src="$IDF_PATH/components/bt"
[ -d "$src/host/bluedroid" ] || { echo "make_bt: no Bluedroid in $src" >&2; exit 1; }
# stamp = hash of (ldac.patch, this script, the IDF checkout's commit / version): the copy is rebuilt only when it is missing
# or the stamp differs (patch edited, IDF upgraded). The stamp sits beside the copy (.local/bt_ldac/.stamp), outside the bt component.
idf_id=$(cd "$IDF_PATH" && { git rev-parse HEAD 2>/dev/null || cat version.txt 2>/dev/null || echo unknown; })
stamp=$(cat "$patch" "$0" | { cat; echo "$idf_id"; } | sha256sum | cut -d' ' -f1)
stamp_file="$(dirname "$dest")/.stamp"
if [ "${1:-}" != "--force" ] && [ -e "$dest/CMakeLists.txt" ] && [ "$(cat "$stamp_file" 2>/dev/null)" = "$stamp" ]; then exit 0; fi
echo "make_bt: patched Bluedroid copy missing or stale (patch / script / IDF changed) or --force: regenerating"
rm -rf "$dest"
mkdir -p "$(dirname "$dest")"
cp -a "$src" "$dest"
cd "$dest"
git init -q
git add host/bluedroid
git -c user.name=make_bt -c user.email=make_bt@local commit -qm "pristine $(cd "$IDF_PATH" && git describe --always --dirty 2>/dev/null || echo IDF) of the IDF bt component"
if ! git apply --check "$patch"; then
    echo "make_bt: FAILED - ldac.patch does not apply to $src (IDF changed?). See tools/bt_ldac/README.md" >&2
    exit 1
fi
git apply "$patch"
# bt/Kconfig sources Bluedroid's options from $IDF_PATH: read the patched copy instead (it allows 5 A2DP endpoints)
sed -i 's|source "$IDF_PATH/components/bt/host/bluedroid/Kconfig.in"|rsource "host/bluedroid/Kconfig.in"|' Kconfig
grep -q 'rsource "host/bluedroid/Kconfig.in"' Kconfig || { echo "make_bt: FAILED - bt/Kconfig does not source Bluedroid's Kconfig.in as expected" >&2; exit 1; }
echo "$stamp" > "$stamp_file"
echo "make_bt: patched Bluedroid ready in $dest ($(git diff --shortstat))"
