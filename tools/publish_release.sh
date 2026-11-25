#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-2.1-or-later
# Create the GitHub release v<version> from release/<version>/ (made by tools/release.py): the four .bin files, SHA256SUMS and manifest.json
# are attached, NOTES.md is the body. The tag must exist and be pushed. A version with a "-" (0.2.0-rc1) is marked as a pre-release.
#   tools/publish_release.sh <version> [--dry-run]     --dry-run: print the gh command, change nothing
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
ver=${1:?usage: tools/publish_release.sh <version> [--dry-run]}; dry=0; [ "${2:-}" = "--dry-run" ] && dry=1
dir=$root/release/$ver
[ -f "$dir/NOTES.md" ] || { echo "publish_release: $dir/NOTES.md missing (run tools/release.py $ver)" >&2; exit 1; }
[ ! -f "$dir/manifest.json" ] || ! grep -q '"dry_run": true' "$dir/manifest.json" || { echo "publish_release: $dir is a dry-run build" >&2; exit 1; }
files=("$dir"/NightStandNano-"$ver"-{full,bootloader,partition-table,app}.bin "$dir/SHA256SUMS" "$dir/manifest.json")
for f in "${files[@]}"; do [ -f "$f" ] || { echo "publish_release: missing $f" >&2; exit 1; }; done
(cd "$dir" && sha256sum -c --quiet SHA256SUMS) || { echo "publish_release: SHA256SUMS does not match the files" >&2; exit 1; }
git -C "$root" rev-parse -q --verify "refs/tags/v$ver" >/dev/null || { echo "publish_release: no tag v$ver (git tag v$ver)" >&2; exit 1; }
cmd=(gh release create "v$ver" "${files[@]}" --title "NightStand Nano $ver" --notes-file "$dir/NOTES.md" --verify-tag)
case "$ver" in *-*) cmd+=(--prerelease);; esac
printf '%q ' "${cmd[@]}"; echo
[ "$dry" = 1 ] && { echo "dry run: nothing created"; exit 0; }
git -C "$root" remote get-url origin >/dev/null 2>&1 || { echo "publish_release: no remote named origin" >&2; exit 1; }
"${cmd[@]}"
