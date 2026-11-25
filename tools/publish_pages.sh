#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-2.1-or-later
# Push release/pages/ (made by tools/release.py) to the gh-pages branch of `origin`: one new commit per publish on top of the
# branch (an orphan branch is created the first time), message "Publish pages from <source commit>". Nothing else is touched.
#   tools/publish_pages.sh [--dry-run]     --dry-run: list what would be pushed, no network, nothing written
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd); pages=$root/release/pages
dry=0; [ "${1:-}" = "--dry-run" ] && dry=1
[ -f "$pages/index.html" ] && [ -f "$pages/flash/index.html" ] && [ -f "$pages/.nojekyll" ] || { echo "publish_pages: $pages is not a complete site (run tools/release.py)" >&2; exit 1; }
url=$(git -C "$root" remote get-url origin 2>/dev/null) || { echo "publish_pages: no remote named origin (git remote add origin ...)" >&2; exit 1; }
src=$(git -C "$root" rev-parse --short=12 HEAD)$( [ -n "$(git -C "$root" status --porcelain)" ] && echo -dirty || true )
msg="Publish pages from $src"
echo "remote: $url   branch: gh-pages   commit message: $msg"
(cd "$pages" && find . -type f | sort | sed 's|^\./||' | while read -r f; do printf '  %9s  %s\n' "$(stat -c %s "$f")" "$f"; done)
[ "$dry" = 1 ] && { echo "dry run: nothing pushed"; exit 0; }
case "$src" in *-dirty) echo "publish_pages: the working tree is dirty; commit first" >&2; exit 1;; esac
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
git -C "$tmp" init -q -b gh-pages
git -C "$tmp" remote add origin "$url"
if git -C "$tmp" fetch -q origin gh-pages 2>/dev/null; then git -C "$tmp" reset -q --soft FETCH_HEAD; git -C "$tmp" rm -rq --cached . || true; fi
find "$tmp" -mindepth 1 -maxdepth 1 -not -name .git -exec rm -rf {} +
cp -a "$pages/." "$tmp/"
git -C "$tmp" add -A
if git -C "$tmp" diff --cached --quiet; then echo "gh-pages already has exactly this content"; exit 0; fi
git -C "$tmp" -c user.name="$(git -C "$root" config user.name)" -c user.email="$(git -C "$root" config user.email)" commit -q -m "$msg"
git -C "$tmp" push origin gh-pages
