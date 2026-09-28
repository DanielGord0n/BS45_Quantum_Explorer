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
# 2026-09-27: wall-clock deadline. Popen.wait(timeout) counts monotonic time, which stops while
# the Mac sleeps: a 5400 s cap ran 9 h across sleep/dark-wake cycles. time.time() keeps counting.
deadline = time.time() + limit
try:
    while True:
        try:
            sys.exit(p.wait(timeout=min(30, max(0.1, deadline - time.time()))))
        except subprocess.TimeoutExpired:
            if time.time() >= deadline:
                raise
except subprocess.TimeoutExpired:
    sys.stderr.write(f"[run_with_timeout] {limit:.0f}s exceeded — killing process group\n")
    try: os.killpg(os.getpgid(p.pid), signal.SIGTERM)
    except Exception: pass
    time.sleep(10)
    try: os.killpg(os.getpgid(p.pid), signal.SIGKILL)
    except Exception: pass
    sys.exit(124)
