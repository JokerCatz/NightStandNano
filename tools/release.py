#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Build a release locally: firmware images, release notes, the GitHub Pages site. Nothing is uploaded.

  tools/release.py 0.1.0               a real release: clean tree, tag v0.1.0 on HEAD
  tools/release.py 0.1.0 --dry-run     any tree, no tag; version string "0.1.0-dryrun", output in release/0.1.0-dryrun/
  options: --web-readonly   do not run `npm ci` / `npm run build` (only `npm test` and `npm run check`); use while the web sources are being edited
           --skip-web       no web steps at all, pages are assembled from the committed dist/ folders

Output (git-ignored):  release/<version>/   NightStandNano-<version>-{full,bootloader,partition-table,app}.bin  SHA256SUMS  NOTES.md  manifest.json
                       release/pages/       the Pages site: web/dist at the root, flash/dist under flash/, .nojekyll
The file names and offsets are what flash/README.md specifies. The firmware is built in app/build-release (app/build is not touched)
with its own sdkconfig generated from sdkconfig.defaults, so the result does not depend on the state of the development build.
Exit code 1 when anything fails; the privacy scan at the end fails a real release, and is a warning in a dry run."""
import argparse, datetime, hashlib, json, os, re, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
APP, REL = os.path.join(ROOT, "app"), os.path.join(ROOT, "release")
BUILD = os.path.join(APP, "build-release")
IDF_ACTIVATE = os.environ.get("IDF_ACTIVATE", os.path.expanduser("~/.espressif/tools/activate_idf_v6.1.sh"))
PAGES_URL = "https://jokercatz.github.io/NightStandNano/"
FLASH_URL = PAGES_URL + "flash/"
REPO_URL = "https://github.com/JokerCatz/NightStandNano"
# (role, name suffix, file inside the build directory, flash offset): flash/README.md, "Release files"
PARTS = [("bootloader", "bootloader.bin", "bootloader/bootloader.bin", 0x2000),
         ("partition-table", "partition-table.bin", "partition_table/partition-table.bin", 0x8000),
         ("app", "app.bin", "nightstand_nano.bin", 0x10000)]


def run(cmd, cwd=ROOT, env=None, shell=False, capture=False):
    print("+", cmd if shell else " ".join(cmd), flush=True)
    r = subprocess.run(cmd, cwd=cwd, env=env, shell=shell, executable="/bin/bash" if shell else None,
                       stdout=subprocess.PIPE if capture else None, stderr=subprocess.STDOUT if capture else None, text=True)
    if r.returncode: sys.exit(f"release: FAILED ({r.returncode}): {cmd if shell else ' '.join(cmd)}\n{r.stdout or ''}")
    return r.stdout


def idf(args):
    """Run idf.py in app/ (inside the IDF environment unless it is already active)."""
    pre = "" if shutil.which("idf.py") else f'source "{IDF_ACTIVATE}" >/dev/null 2>&1 && '
    return run(pre + "cd app && " + args, shell=True)


def git(*a):
    return subprocess.run(["git", "-C", ROOT, *a], capture_output=True, text=True).stdout.strip()


def sha256(p):
    return hashlib.sha256(open(p, "rb").read()).hexdigest()


# ---------------------------------------------------------------------------------------------------------------- firmware
def build_firmware(ver):
    shutil.rmtree(BUILD, ignore_errors=True)
    # explicit absolute SDKCONFIG: a fresh one from sdkconfig.defaults inside the build directory; the version is passed, never guessed
    idf(f'idf.py --preview -B build-release -DSDKCONFIG={BUILD}/sdkconfig -DIDF_TARGET=esp32s31 -DPROJECT_VER={ver} build')
    for _, _, rel, _ in PARTS:
        if not os.path.isfile(os.path.join(BUILD, rel)): sys.exit(f"release: build output missing: {rel}")


def merge(out, files):
    """esptool merge-bin exactly as flash/README.md asks: from offset 0, 0xFF fill, header flash settings kept, no padding after the app."""
    args = " ".join(f"0x{o:x} '{f}'" for o, f in files)
    pre = "" if shutil.which("idf.py") else f'source "{IDF_ACTIVATE}" >/dev/null 2>&1 && '
    run(pre + f"python -m esptool --chip esp32s31 merge-bin -o '{out}' --flash-mode keep --flash-size keep --flash-freq keep {args}", shell=True)


def verify_images(full, parts, ver):
    """Parse the merged image back; the three separate files must be byte-identical to their ranges of it."""
    m = open(full, "rb").read()
    err = []
    app_end = max(o + len(open(f, "rb").read()) for o, f in parts)
    if len(m) != app_end: err.append(f"merged size {len(m)} != end of app {app_end}")
    if m[:0x2000] != b"\xff" * 0x2000: err.append("0x0..0x2000 is not all 0xFF")
    for name, off in (("bootloader", 0x2000), ("app", 0x10000)):
        if m[off] != 0xE9: err.append(f"no image magic 0xE9 at {off:#x} ({name})")
        if int.from_bytes(m[off + 12:off + 14], "little") != 32: err.append(f"{name}: chip id is not 32 (ESP32-S31)")
    if m[0x8000:0x8002] != b"\xaa\x50": err.append("no partition table (AA 50) at 0x8000")
    if m[0x10000 + 0x20:0x10000 + 0x24] != bytes.fromhex("3254cdab"): err.append("no esp_app_desc magic at app+0x20")
    prev_end = 0x2000
    for off, f in sorted(parts):
        b = open(f, "rb").read()
        if m[off:off + len(b)] != b: err.append(f"{os.path.basename(f)} differs from merged[{off:#x}:+{len(b):#x}]")
        if m[prev_end:off] != b"\xff" * (off - prev_end): err.append(f"gap {prev_end:#x}..{off:#x} is not 0xFF")
        prev_end = off + len(b)
    ver_in_app = m[0x10000 + 0x30:0x10000 + 0x50].split(b"\0")[0].decode()
    if ver_in_app != ver: err.append(f"app version string is '{ver_in_app}', expected '{ver}'")
    print("image checks:", "FAILED" if err else "ok (magic, table, 0xFF gaps, nothing after the app, three files = ranges of the merged file, version string)")
    for e in err: print("  -", e)
    if err: sys.exit(1)


# ---------------------------------------------------------------------------------------------------------------- web
def web_steps(mode):
    problems = []
    for d in ("web", "flash"):
        steps = [] if mode == "skip" else ([["npm", "test"], ["npm", "run", "check"]] if mode == "readonly" else
                                           [["npm", "ci"], ["npm", "run", "build"], ["npm", "test"], ["npm", "run", "check"]])
        for s in steps:
            r = subprocess.run(s, cwd=os.path.join(ROOT, d), capture_output=True, text=True)
            print(f"{d}: {' '.join(s)} -> {'ok' if r.returncode == 0 else 'FAILED'}")
            if r.returncode:
                problems.append(f"{d}: {' '.join(s)}")
                print("\n".join("    " + l for l in (r.stdout + r.stderr).strip().splitlines()[-12:]))
    return problems


def assemble_pages():
    pages = os.path.join(REL, "pages")
    shutil.rmtree(pages, ignore_errors=True)
    for src, dst in (("web/dist", pages), ("flash/dist", os.path.join(pages, "flash"))):
        src = os.path.join(ROOT, src)
        if not os.path.isfile(os.path.join(src, "index.html")): sys.exit(f"release: {src}/index.html missing (npm run build)")
        shutil.copytree(src, dst, dirs_exist_ok=True)
    open(os.path.join(pages, ".nojekyll"), "w").close()
    return pages


# ---------------------------------------------------------------------------------------------------------------- privacy scan
MAC = re.compile(rb'(?<![0-9A-Fa-f:])(?!00(?::00){5}|ff(?::ff){5}|FF(?::FF){5})[0-9A-Fa-f]{2}(?::[0-9A-Fa-f]{2}){5}(?![0-9A-Fa-f:])')
HOME = re.compile(rb'/home/[A-Za-z0-9_.-]+/|/Users/[A-Za-z0-9_.-]+/|/media/[A-Za-z0-9_.-]+/|[A-Za-z]:\\{1,2}Users|[A-Za-z]:/Users/')
TEXT = [("home-directory path", HOME), ("MAC address", MAC),
        ("USB serial / by-id path", re.compile(rb'usb-[A-Za-z0-9]+_[A-Za-z0-9_.-]+_[A-Za-z0-9:]{8,}-if\d\d|/dev/serial/by-id|/dev/tty(?:ACM|USB)\d')),
        ("private key marker", re.compile(rb'-----BEGIN [A-Z ]*PRIVATE KEY|ssh-rsa AAAA|AKIA[0-9A-Z]{16}')),
        ("private IP address", re.compile(rb'(?<![\d.])(?:192\.168|10\.\d{1,3}|172\.(?:1[6-9]|2\d|3[01])|169\.254)\.\d{1,3}\.\d{1,3}(?![\d.])')),
        ("claude.ai session URL", re.compile(rb'claude\.ai/code/'))]
BIN = [("home-directory path", HOME), ("this repository's absolute path", re.compile(re.escape(ROOT.encode()))),
       ("the home directory", re.compile(re.escape(os.path.expanduser("~").encode()))),
       ("USB serial / by-id path", TEXT[2][1])]    # no key-marker rule here: mbedTLS's PEM parser carries "-----BEGIN ... PRIVATE KEY-----" as string literals


def scan(path, rules):
    data, out = open(path, "rb").read(), []
    for what, rx in rules:
        for m in rx.finditer(data):
            line = data.count(b"\n", 0, m.start()) + 1
            s = m.group(0).decode(errors="replace")
            out.append(f"{os.path.relpath(path, ROOT)}:{line}: {what}: {s[:3]}...{s[-3:] if len(s) > 6 else ''} ({len(s)} chars)")
    return out


def privacy(pages, outdir):
    hits = []
    for base, _, names in os.walk(pages):
        for n in sorted(names):
            if n != ".nojekyll": hits += scan(os.path.join(base, n), TEXT)
    for n in sorted(os.listdir(outdir)):
        p = os.path.join(outdir, n)
        hits += scan(p, BIN if n.endswith(".bin") else TEXT)
    print(f"privacy scan: {len(hits)} finding(s)")
    for h in hits: print("  -", h)
    return hits


# ---------------------------------------------------------------------------------------------------------------- notes
def third_party_table():
    t = open(os.path.join(ROOT, "THIRD_PARTY_NOTICES.md"), encoding="utf-8").read()
    m = re.search(r"<!-- release-notes:begin -->\n(.*?)<!-- release-notes:end -->", t, re.S)
    if not m: sys.exit("release: THIRD_PARTY_NOTICES.md has no release-notes markers")
    table = m.group(1).strip()
    origin = open(os.path.join(ROOT, "components", "ORIGIN.md"), encoding="utf-8").read()
    names = re.findall(r"^\| (\w+) \| \d+ -> \d+ \|", origin, re.M)
    missing = [n for n in names if n not in table]
    if missing: sys.exit(f"release: components in ORIGIN.md but not in THIRD_PARTY_NOTICES.md: {missing}")
    return table


def write_notes(path, ver, commit, files, flash_args):
    ea = flash_args["extra_esptool_args"]; wa = " ".join(flash_args["write_flash_args"])
    base = f"python -m esptool --chip {ea['chip']} -b 460800 --before {ea['before']} --after {ea['after']}" + (" --no-stub" if not ea.get("stub", True) else "")
    n = lambda s: f"NightStandNano-{ver}-{s}"
    sep = "".join(f" 0x{o:x} {n(s)}" for s, o in (("bootloader.bin", 0x2000), ("partition-table.bin", 0x8000), ("app.bin", 0x10000)))
    text = f"""# NightStand Nano {ver}

Released {datetime.date.today().isoformat()} from commit `{commit}`. Source: {REPO_URL}

A headless ESP32-S31 audio box: TF card, internet radio or phone streaming in; a USB-C DAC, Bluetooth headphones (LDAC / AAC / SBC) or an
Auracast broadcast out; controlled from a web page over Bluetooth LE. Licence LGPL-2.1-or-later. README and known limits: {REPO_URL}#readme

## Flash it (中文：用燒錄頁，或 esptool)

1. Download `{n('full.bin')}` (or the three separate files `{n('bootloader.bin')}`, `{n('partition-table.bin')}`, `{n('app.bin')}`).
2. Open the flasher page in desktop Chrome or Edge: {FLASH_URL} , connect the board's UART bridge port, pick the file, flash.
   The page keeps your settings (Wi-Fi, pairings) unless you tick the erase option.
3. Open the control page: {PAGES_URL}

Command line instead (esptool 5.x; this erases the settings area, because the merged image covers it):

```
{base} write-flash {wa} 0x0 {n('full.bin')}
```

or, keeping the settings (three files):

```
{base} write-flash {wa}{sep}
```

Check the download: `sha256sum -c SHA256SUMS`.

## Files

| file | flash offset | what |
|---|---|---|
""" + "".join(f"| `{f['name']}` | {f['offset']} | {f['what']} |\n" for f in files) + f"""
## Third-party components

Details, licence texts and where they are in the repository: `THIRD_PARTY_NOTICES.md`.

{third_party_table()}

Fetched at build time and linked into the binaries: ESP-IDF (Apache-2.0), `espressif/usb` (Apache-2.0), `espressif/led_strip` (Apache-2.0),
`espressif/esp_audio_codec` (Espressif Modified MIT, "for use exclusively with Espressif Systems products").
"""
    open(path, "w", encoding="utf-8").write(text)


# ---------------------------------------------------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("version"); ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--web-readonly", action="store_true"); ap.add_argument("--skip-web", action="store_true")
    a = ap.parse_args()
    if not re.fullmatch(r"\d+\.\d+\.\d+([-.][0-9A-Za-z.]+)?", a.version): sys.exit("release: version looks like 0.1.0")
    commit, dirty = git("rev-parse", "--short=12", "HEAD"), bool(git("status", "--porcelain"))
    if not a.dry_run:
        if dirty: sys.exit("release: the working tree is not clean (commit first, or --dry-run)")
        if git("rev-parse", f"v{a.version}^{{commit}}") != git("rev-parse", "HEAD"): sys.exit(f"release: tag v{a.version} does not point at HEAD (git tag v{a.version}, or --dry-run)")
    ver = a.version + "-dryrun" if a.dry_run else a.version
    outdir = os.path.join(REL, ver)
    shutil.rmtree(outdir, ignore_errors=True); os.makedirs(outdir)

    build_firmware(ver)
    n = lambda s: f"NightStandNano-{ver}-{s}"
    parts = []
    for role, suffix, rel, off in PARTS:
        dst = os.path.join(outdir, n(suffix)); shutil.copyfile(os.path.join(BUILD, rel), dst); parts.append((off, dst))
    full = os.path.join(outdir, n("full.bin"))
    merge(full, parts)
    verify_images(full, parts, ver)

    names = [n("full.bin")] + [n(p[1]) for p in PARTS]
    what = {"full": "everything in one image (merged by esptool merge-bin)", "bootloader": "bootloader", "partition-table": "partition table", "app": "application"}
    flist = [dict(name=n("full.bin"), offset="0x0", what=what["full"])] + [dict(name=n(p[1]), offset=f"0x{p[3]:x}", what=what[p[0]]) for p in PARTS]
    with open(os.path.join(outdir, "SHA256SUMS"), "w") as f:
        for x in sorted(names): f.write(f"{sha256(os.path.join(outdir, x))}  {x}\n")
    flash_args = json.load(open(os.path.join(BUILD, "flasher_args.json")))
    desc = json.load(open(os.path.join(BUILD, "project_description.json")))
    idf_ver = open(parts[-1][1], "rb").read()[0x20 + 112:0x20 + 144].split(b"\0")[0].decode()     # esp_app_desc_t.idf_ver
    for x in flist: p = os.path.join(outdir, x["name"]); x["size"] = os.path.getsize(p); x["sha256"] = sha256(p)
    json.dump(dict(version=ver, source_version=a.version, dry_run=a.dry_run, git_commit=git("rev-parse", "HEAD"), git_dirty=dirty, idf_version=idf_ver,
                   target=desc.get("target"), flash_settings=flash_args["flash_settings"], files=flist),
              open(os.path.join(outdir, "manifest.json"), "w"), indent=2); open(os.path.join(outdir, "manifest.json"), "a").write("\n")
    write_notes(os.path.join(outdir, "NOTES.md"), ver, commit + ("-dirty" if dirty else ""), flist, flash_args)

    problems = web_steps("skip" if a.skip_web else "readonly" if a.web_readonly else "full")
    pages = assemble_pages()
    hits = privacy(pages, outdir)

    print(f"\nrelease {ver}: {outdir}")
    for x in flist: print(f"  {x['name']:52} {x['size']:>9}  {x['sha256']}")
    if problems: print("WEB STEPS FAILED:", "; ".join(problems))
    if hits: print(f"PRIVACY: {len(hits)} finding(s) above")
    if (problems or hits) and not a.dry_run: sys.exit("release: not releasable, fix the above")


main()
