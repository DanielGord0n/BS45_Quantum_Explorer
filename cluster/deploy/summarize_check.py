#!/usr/bin/env python3
"""summarize_check.py <latest_check.txt> — deterministic, LLM-free digest of a checker run
(2026-09-20): sent to the phone right after every check and used as the fallback text
whenever the agent fails, so Daniel always gets numbers, never just "see the log".
The checker already hides processed (excluded) outputs, so every complete read shown here
is NEW. Output <= ~900 chars."""
import re, sys
from collections import Counter
t = open(sys.argv[1], encoding="utf-8", errors="replace").read()
# banners only outside the checker's own exclusion header (which quotes old FOUND text)
body = "\n".join(l for l in t.splitlines() if not l.startswith("--- FIRSTHIT PROBES"))
found = len(re.findall(r"FOUND \*\*\*", body))
summ = re.search(r"^Summary:(.*)$", t, flags=re.M)
missed = set(re.findall(r"\w+", (re.search(r"missed:([^;(]*)", summ.group(1)).group(1) if summ and "missed:" in summ.group(1) else "")))
parts = []
secs = re.split(r"^════+ (\w+) ════+$", t, flags=re.M)
for i in range(1, len(secs), 2):
    c, b = secs[i], secs[i + 1]
    head = "\n".join(b.splitlines()[:4])          # the checker's own marker lines come first
    if "listed outage" in head: parts.append(f"{c}: outage (no push)"); continue
    if c in missed or "no approval" in head: parts.append(f"{c}: Duo missed"); continue
    q = re.findall(r"^\s+\d+\s+(\S+)\s+(PD|R)\s", b, flags=re.M)
    st = Counter(s for _, s in q)
    files = re.split(r"=== firsthit_output_\d+\.txt ===", b)[1:]
    done, hdr, tested, ab, cells = 0, 0, [], [], []
    for bb in files:
        h = re.search(r"arms_with_hits=(\d+)", bb)
        if not h: hdr += 1; continue
        done += 1
        te = re.search(r" tested=(\d+)", bb); a = re.search(r" aborted=(\d+)", bb); cd = re.search(r"cells_done_sum=(\d+)", bb)
        if te: tested.append(int(te.group(1)) / 1e6)
        if te and a and int(te.group(1)): ab.append(100 * int(a.group(1)) / int(te.group(1)))
        if cd: cells.append(int(cd.group(1)))
    s = f"{c}: {st.get('R',0)}R/{st.get('PD',0)}PD"
    if done:
        s += f", {done} new read{'s' if done>1 else ''}"
        if tested: s += f" {min(tested):.0f}-{max(tested):.0f}M"
        if ab: s += f", aborts {min(ab):.0f}-{max(ab):.0f}%"
        if cells: s += f", cells {min(cells)}-{max(cells)}"
    else: s += ", no new reads"
    if hdr: s += f", {hdr} running"
    if st.get('PD', 0) < 8: s += " (REFILL: pending<8)"
    parts.append(s)
head = f"FOUND banners: {found} — VERIFY NOW. " if found else "No FOUND banners. "
print((head + "; ".join(parts))[:900])
