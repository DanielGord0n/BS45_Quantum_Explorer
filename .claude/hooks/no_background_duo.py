#!/usr/bin/env python3
"""PreToolUse hook (2026-09-10): the headless daily agent must never background a
Duo-gated submit. Three runs (08-29, 09-09) ended with 'waiting on a backgrounded
duo_run' and no submit/summary. Deny Bash calls that set run_in_background=true
and touch duo_run.sh / sbatch / ssh; the agent then re-issues them in the foreground."""
import json, sys
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(0)
if data.get("tool_name") != "Bash":
    sys.exit(0)
ti = data.get("tool_input") or {}
cmd = str(ti.get("command", ""))
bg = bool(ti.get("run_in_background"))
if bg and any(k in cmd for k in ("duo_run.sh", "sbatch", "ssh ", "check_all_retry.sh", "git push")):
    print(json.dumps({
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "deny",
            "permissionDecisionReason": "BLOCKED: duo_run/sbatch/ssh/git push must run in the FOREGROUND. "
                "This is a headless -p session: ending your turn ends the run, nothing re-invokes you. "
                "Re-issue the exact same command with run_in_background=false and wait for its output."
        }
    }))
    sys.exit(0)
sys.exit(0)
