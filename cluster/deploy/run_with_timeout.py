#!/usr/bin/env python3
"""run_with_timeout.py <seconds> <cmd> [args...] — run a command in its own process group,
kill the whole group if it exceeds <seconds>, exit 124 on timeout (else the command's code).
2026-09-19: the headless daily agent ran 4-6 h and died on API timeouts; normal runs take
20-60 min. macOS has no coreutils `timeout`, hence this helper."""
import os, sys, signal, subprocess, time
if len(sys.argv) < 3:
    sys.stderr.write("usage: run_with_timeout.py <seconds> <cmd> [args...]\n"); sys.exit(64)
limit = float(sys.argv[1]); cmd = sys.argv[2:]
p = subprocess.Popen(cmd, start_new_session=True)
try:
    sys.exit(p.wait(timeout=limit))
except subprocess.TimeoutExpired:
    sys.stderr.write(f"[run_with_timeout] {limit:.0f}s exceeded — killing process group\n")
    try: os.killpg(os.getpgid(p.pid), signal.SIGTERM)
    except Exception: pass
    time.sleep(10)
    try: os.killpg(os.getpgid(p.pid), signal.SIGKILL)
    except Exception: pass
    sys.exit(124)
