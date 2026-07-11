#!/usr/bin/env python3
"""Drive ONE Duo SSH login, auto-selecting Duo Push.

Spawns `ssh <host> <remote_cmd>` under a pty, types "1" at the Alliance Duo
"Passcode or option" menu to send the push, then waits up to <timeout> seconds
for you to approve on your phone. Mirrors ssh output so the caller can capture
the remote command's result.

Deliberately NO ControlMaster/ControlPersist — that connection-sharing was what
caused overlapping logins and leaked prompts. One clean ssh per call.

Usage: duo_ssh.py <host> <remote_cmd> <timeout_seconds>
Exit:  0 = saw ===BS45END=== (success), 2 = timeout/no approval, other = error.
"""
import os, sys, pty, select, time, re

if len(sys.argv) != 4:
    sys.stderr.write("usage: duo_ssh.py <host> <remote_cmd> <timeout>\n")
    sys.exit(64)

host, cmd, timeout = sys.argv[1], sys.argv[2], float(sys.argv[3])
ssh_args = ["ssh", "-o", "ConnectTimeout=30",
            "-o", "StrictHostKeyChecking=accept-new",
            "-o", "NumberOfPasswordPrompts=1", host, cmd]

pid, fd = pty.fork()
if pid == 0:                      # child -> becomes ssh
    os.execvp("ssh", ssh_args)
    os._exit(127)

buf = b""
sent = False                      # only send "1" once => exactly one push
status = 2
deadline = time.time() + timeout
prompt = re.compile(rb"passcode or option", re.I)
end_marker = b"===BS45END==="

try:
    while True:
        if time.time() > deadline:
            status = 2
            break
        r, _, _ = select.select([fd], [], [], 1.0)
        if fd in r:
            try:
                data = os.read(fd, 4096)
            except OSError:
                break
            if not data:
                break
            buf += data
            try:
                os.write(1, data)          # mirror to caller (captured)
            except OSError:
                pass
            if not sent and prompt.search(buf):
                os.write(fd, b"1\n")       # select Duo Push -> fires phone prompt
                sent = True
            if end_marker in buf:
                status = 0
                break
finally:
    try:
        os.close(fd)
    except OSError:
        pass
    try:
        os.waitpid(pid, 0)
    except OSError:
        pass

sys.exit(status)
