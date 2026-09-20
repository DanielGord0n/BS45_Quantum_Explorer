#!/usr/bin/env python3
"""spawn_detached.py <cmd> [args...] — start a command in a NEW SESSION with stdin/stdout/
stderr detached, so it survives its parent's exit (launchd kills a finished job's process
group; the hourly reminder spawned by the 1pm run died instantly on 2026-09-20)."""
import os, subprocess, sys
if len(sys.argv) < 2: sys.stderr.write("usage: spawn_detached.py <cmd> [args...]\n"); sys.exit(64)
p = subprocess.Popen(sys.argv[1:], start_new_session=True, stdin=subprocess.DEVNULL,
                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, close_fds=True)
print(p.pid)
