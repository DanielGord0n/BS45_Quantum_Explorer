#!/usr/bin/env python3
"""Generate docs/plans/launch/passh_step*_<cluster>.sh for the Pass H launch.

step1_<cluster>.sh: sha-checked redeploy of src/solver/wz_match.cpp (patch from the sha the
cluster currently holds, given as --from-commit, to HEAD) and of cluster_firsthit_probe.sh, then
scancel every PENDING Pass G job (F44*/R44*/N44*/T44*; checkpoints untouched), then queue counts.
step2_<cluster>_<k>.sh: the H submit lines in chunks of 100 (ssh argument limit), each echoing
"<lane>: Submitted batch job <id>". Run each with cluster/deploy/duo_run.sh (one tap each).
Usage: python3 tools/passh_launch_scripts.py --from-commit f611904
"""
import argparse
import base64
import hashlib
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PREFIX = {'fir': 'F44', 'rorqual': 'R44', 'nibi': 'N44', 'trillium': 'T44'}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--from-commit', required=True, help='commit whose solver the clusters currently run')
    a = ap.parse_args()
    assert not subprocess.run(['git', 'status', '--porcelain', 'src/solver/wz_match.cpp', 'cluster/deploy/cluster_firsthit_probe.sh'],
                              capture_output=True, text=True, cwd=ROOT).stdout.strip(), 'commit the solver/driver first'
    from_sha = hashlib.sha256(subprocess.run(['git', 'show', f'{a.from_commit}:src/solver/wz_match.cpp'], capture_output=True, cwd=ROOT).stdout).hexdigest()
    solver_sha = hashlib.sha256((ROOT / 'src/solver/wz_match.cpp').read_bytes()).hexdigest()
    drv = (ROOT / 'cluster/deploy/cluster_firsthit_probe.sh').read_bytes()
    drv_sha = hashlib.sha256(drv).hexdigest()
    patch = subprocess.run(['git', 'diff', a.from_commit, 'HEAD', '--', 'src/solver/wz_match.cpp'], capture_output=True, cwd=ROOT).stdout
    pb, db = base64.b64encode(patch).decode(), base64.b64encode(drv).decode()
    out = ROOT / 'docs/plans/launch'
    out.mkdir(exist_ok=True)
    for old in out.glob('passh_step*.sh'):
        old.unlink()
    head = subprocess.run(['git', 'rev-parse', '--short', 'HEAD'], capture_output=True, text=True, cwd=ROOT).stdout.strip()
    for c, pre in PREFIX.items():
        lines = (ROOT / f'docs/plans/passh_submit_{c}.txt').read_text().strip().split('\n')
        s1 = (f'cd $SCRATCH/bs45 && X=$(sha256sum src/solver/wz_match.cpp | cut -c1-64) && echo "{c} before=$X" && '
              f'[ "$X" = {from_sha} ] && cp src/solver/wz_match.cpp src/solver/wz_match.cpp.bak-{a.from_commit} && '
              f'echo {pb} | base64 -d | patch -p1 --no-backup-if-mismatch -s && Y=$(sha256sum src/solver/wz_match.cpp | cut -c1-64) && [ "$Y" = {solver_sha} ] && '
              f'cp cluster_firsthit_probe.sh cluster_firsthit_probe.sh.bak-{a.from_commit} && echo {db} | base64 -d > cluster_firsthit_probe.sh && '
              f'Z=$(sha256sum cluster_firsthit_probe.sh | cut -c1-64) && [ "$Z" = {drv_sha} ] && echo "{c} DEPLOYED {head} solver=$Y driver=$Z" && '
              f'P=$(squeue -u dangord -h -t PD -o "%i %j" | grep -E " {pre}" | awk \'{{print $1}}\') && echo "cancelling $(echo $P | wc -w) pending G jobs" && '
              f'[ -n "$P" ] && scancel $P; squeue -u dangord -h -o "%T" | sort | uniq -c | tr "\\n" " "; echo')
        (out / f'passh_step1_{c}.sh').write_text(s1 + '\n')
        for k in range(0, len(lines), 100):
            chunk = lines[k:k + 100]
            s2 = 'cd $SCRATCH/bs45 && ' + ' ; '.join('echo "%s: $(%s)"' % (l.split(' -J ')[1].split()[0], l) for l in chunk)
            (out / f'passh_step2_{c}_{k // 100 + 1}.sh').write_text(s2 + '\n')
        print(f'{c}: step1 {len(s1)} bytes; {len(lines)} submit lines in {(len(lines) + 99) // 100} chunk(s)')
    print(f'launch scripts for build {head} (patch from {a.from_commit}, solver sha {solver_sha[:12]}..)')


if __name__ == '__main__':
    main()
