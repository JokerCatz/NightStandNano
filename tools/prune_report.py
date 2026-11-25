#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Which source files of components/ contribute bytes to the linked firmware?

The way to prune components/ (and to check later that nothing dead has crept back in):

    tools/prune_report.py                       # table per component: every compiled source, bytes it puts into the ELF
    tools/prune_report.py --unused              # only the sources that contribute 0 bytes (removable)
    tools/prune_report.py --files               # also: files in components/ that no kept source reaches (header include
                                                #   closure) and that are not build / licence / notice files
    tools/prune_report.py --summary             # per component: files and lines now

Input is the last build in app/build (nightstand_nano.map, compile_commands.json); build first
(cd app && idf.py --preview build). A source "contributes" when at least one input section of its object is placed
in an output section of the ELF (-ffunction-sections / -fdata-sections / --gc-sections drop the rest, so an object can
be pulled in by the linker and still give 0 bytes). Archive members are matched by lib<component>.a(<object>); two
sources of one component with the same object name cannot be told apart in the map and are reported together.
Sources that are in components/ but not compiled at all are listed as "not built".
Standard library only.
"""
import argparse, json, os, re, sys

ROOT = os.path.realpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
SRC_EXT = ('.c', '.cc', '.cpp', '.cxx', '.S', '.s')
HDR_EXT = ('.h', '.hh', '.hpp', '.hxx', '.inc', '.inl', '.def', '.tbl')
# files that stay whatever the include closure says: build system, licences, notices, credits, provenance (patches/ = the record of
# what was changed against upstream; the build does not apply them)
KEEP_RE = re.compile(r'(^|/)(CMakeLists\.txt|Kconfig[^/]*|idf_component\.yml|linker\.lf|ORIGIN\.md|SOURCE(\.md)?|NOTICE|README[^/]*|readme[^/]*|'
                     r'LICEN[CS]E[^/]*|licen[cs]e[^/]*|COPYING[^/]*|COPYRIGHT[^/]*|AUTHORS[^/]*|CREDITS[^/]*)$|\.cmake$|\.lf$|(^|/)patches/')


def parse_map(path):
    """-> {(archive basename, member): bytes} for the placed input sections of the final link."""
    contrib = {}
    in_map = False
    out_addr = 0
    pending = None
    rx_out = re.compile(r'^(\.\S+|\*\S+)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)')
    rx_in = re.compile(r'^ (\S+)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+(\S.*)$')
    rx_cont = re.compile(r'^\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+(\S.*)$')
    rx_name = re.compile(r'^ (\.\S+)\s*$')
    rx_mem = re.compile(r'^(.*?)\(([^()]+)\)$')
    with open(path, errors='replace') as f:
        for line in f:
            if not in_map:
                in_map = line.startswith('Linker script and memory map')
                continue
            line = line.rstrip('\n')
            m = rx_out.match(line)
            if m:
                out_addr = int(m.group(2), 16)
                pending = None
                continue
            if out_addr == 0:
                continue               # .debug_*, .comment ...: not part of the image
            m = rx_in.match(line)
            if m:
                size, obj = int(m.group(3), 16), m.group(4)
            else:
                m = rx_name.match(line)
                if m:
                    pending = True
                    continue
                m = rx_cont.match(line)
                if not (m and pending):
                    continue
                pending = None
                size, obj = int(m.group(2), 16), m.group(3)
            pending = None
            mm = rx_mem.match(obj)
            if not mm or size == 0:
                continue
            key = (os.path.basename(mm.group(1)), mm.group(2))
            contrib[key] = contrib.get(key, 0) + size
    return contrib


def count_lines(p):
    try:
        with open(p, 'rb') as f:
            return f.read().count(b'\n')
    except OSError:
        return 0


def walk(d):
    for r, dirs, files in os.walk(d):
        dirs.sort()
        for n in sorted(files):
            yield os.path.join(r, n)


def include_closure(roots, search_dirs_by_file, all_files):
    """Text-level #include closure from the root files. Unresolvable names (IDF / libc headers) are ignored."""
    rx = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.M)
    by_base = {}
    for p in all_files:
        by_base.setdefault(os.path.basename(p), []).append(p)
    seen, todo = set(), list(roots)
    while todo:
        p = todo.pop()
        if p in seen:
            continue
        seen.add(p)
        try:
            txt = open(p, errors='replace').read()
        except OSError:
            continue
        dirs = [os.path.dirname(p)] + search_dirs_by_file.get(p, []) + search_dirs_by_file.get('*', [])
        for inc in rx.findall(txt):
            hit = None
            for d in dirs:
                c = os.path.normpath(os.path.join(d, inc))
                if os.path.isfile(c):
                    hit = c
                    break
            if hit:
                todo.append(hit)
                continue
            # not found along the compiler's include path: a header of the same relative tail elsewhere in the tree
            # (a conditional include of another configuration) is kept too, conservatively
            for c in by_base.get(os.path.basename(inc), []):
                if c.replace(os.sep, '/').endswith(inc.lstrip('./')):
                    todo.append(c)
    return seen


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--build', default=os.path.join(ROOT, 'app', 'build'))
    ap.add_argument('--components', default=os.path.join(ROOT, 'components'))
    ap.add_argument('--component', action='append', help='only this component (repeatable)')
    ap.add_argument('--unused', action='store_true', help='print only sources with 0 bytes and sources not built')
    ap.add_argument('--files', action='store_true', help='list files nothing reaches (after dropping unused sources)')
    ap.add_argument('--summary', action='store_true', help='per component: files and lines in the tree now')
    a = ap.parse_args()

    comps = os.path.realpath(a.components)
    names = sorted(n for n in os.listdir(comps) if os.path.isdir(os.path.join(comps, n)))
    if a.component:
        names = [n for n in names if n in a.component]

    if a.summary:
        tf = tl = 0
        print('%-14s %7s %9s' % ('component', 'files', 'lines'))
        for n in names:
            fl = list(walk(os.path.join(comps, n)))
            ln = sum(count_lines(p) for p in fl)
            tf += len(fl); tl += ln
            print('%-14s %7d %9d' % (n, len(fl), ln))
        print('%-14s %7d %9d' % ('total', tf, tl))
        return 0

    mp = os.path.join(a.build, 'nightstand_nano.map')
    cc = os.path.join(a.build, 'compile_commands.json')
    if not (os.path.isfile(mp) and os.path.isfile(cc)):
        sys.exit('no %s / %s: build first (cd app && idf.py --preview build)' % (mp, cc))
    contrib = parse_map(mp)
    cmds = json.load(open(cc))

    # compiled sources per component: file -> (object basename, include dirs)
    built = {}
    for e in cmds:
        f = os.path.realpath(os.path.join(e['directory'], e['file']))
        if not f.startswith(comps + os.sep):
            continue
        comp = os.path.relpath(f, comps).split(os.sep)[0]
        cmd = e.get('command') or ' '.join(e['arguments'])
        incs = re.findall(r'(?:^|\s)-I\s*(\S+)', cmd)
        built.setdefault(comp, {})[f] = (os.path.basename(e['output']), incs)

    used_src, report = set(), {}
    for comp in names:
        arch = 'lib%s.a' % comp
        rows = []
        objcount = {}
        for f, (obj, _) in built.get(comp, {}).items():
            objcount[obj] = objcount.get(obj, 0) + 1
        for f, (obj, _) in sorted(built.get(comp, {}).items()):
            b = contrib.get((arch, obj), 0)
            rows.append((os.path.relpath(f, comps), b, objcount[obj] > 1))
            if b > 0:
                used_src.add(f)
        allsrc = [p for p in walk(os.path.join(comps, comp)) if p.endswith(SRC_EXT)]
        for p in allsrc:
            if p not in built.get(comp, {}):
                rows.append((os.path.relpath(p, comps), None, False))
        report[comp] = rows

    tot_used = tot_unused = tot_nb = 0
    for comp in names:
        rows = report[comp]
        u = [r for r in rows if r[1]]
        z = [r for r in rows if r[1] == 0]
        nb = [r for r in rows if r[1] is None]
        tot_used += len(u); tot_unused += len(z); tot_nb += len(nb)
        print('== %s: %d sources compiled, %d contribute bytes (%d B), %d contribute 0, %d not built'
              % (comp, len(u) + len(z), len(u), sum(r[1] for r in u), len(z), len(nb)))
        for rel, b, dup in rows:
            if a.unused and b:
                continue
            tag = 'not built' if b is None else ('%8d B' % b)
            print('   %-60s %s%s' % (rel, tag, '  (object name shared with another source: counted together)' if dup else ''))
    print('\ntotal: %d used, %d compiled with 0 bytes, %d not built' % (tot_used, tot_unused, tot_nb))

    if a.files:
        # roots: used sources; plus the project's own sources (app/main) which may include component headers
        roots = set(used_src)
        search = {}
        for comp in names:
            for f, (_, incs) in built.get(comp, {}).items():
                search[f] = incs
        dirs_all = set()
        for e in cmds:
            cmd = e.get('command') or ' '.join(e['arguments'])
            for i in re.findall(r'(?:^|\s)-I\s*(\S+)', cmd):
                dirs_all.add(os.path.realpath(os.path.join(e['directory'], i)))
            f = os.path.realpath(os.path.join(e['directory'], e['file']))
            if f.startswith(os.path.join(ROOT, 'app') + os.sep) and '/managed_components/' not in f:
                roots.add(f)
                search[f] = [os.path.realpath(os.path.join(e['directory'], i)) for i in re.findall(r'(?:^|\s)-I\s*(\S+)', cmd)]
        search['*'] = sorted(d for d in dirs_all if d.startswith(ROOT))
        everything = [p for p in walk(ROOT) if '/.git/' not in p and '/app/build' not in p and '/.local/' not in p
                      and '/managed_components/' not in p and '/node_modules/' not in p
                      and (p.endswith(SRC_EXT + HDR_EXT))]
        closure = include_closure(roots, search, everything)
        print('\nfiles under components/ that nothing kept reaches (sources with 0 bytes and unreachable headers / data):')
        n = 0
        for comp in names:
            for p in walk(os.path.join(comps, comp)):
                rel = os.path.relpath(p, comps)
                if KEEP_RE.search(rel) or p in closure:
                    continue
                n += 1
                print('   ' + rel)
        print('%d files' % n)
    return 0


if __name__ == '__main__':
    sys.exit(main())
