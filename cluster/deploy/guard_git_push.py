#!/usr/bin/env python3
"""PreToolUse guard: ask before any git push that LANDS on main or master.

Installed as a Claude Code hook (matcher: Bash) in ~/.claude/settings.json.
Reads the tool-call JSON on stdin. Prints an "ask" permission decision when the
command would push to a protected branch; prints nothing otherwise.
Deterministic protection — works no matter which model is driving.

--- 2026-07-11 changes -------------------------------------------------------
1. REFSPEC LOOPHOLE CLOSED. The old version matched only `git push ... main` or
   a *bare* `git push`. A headless agent pushed `git push origin HEAD` while on
   main — neither pattern matched, so the guard stayed silent and the push went
   through. We now RESOLVE what the push actually lands on (bare / HEAD /
   HEAD:branch / branch / src:dst) and compare that to PROTECTED.

2. NARROW BOOKKEEPING EXCEPTION (BS45 autonomous loop only). The daily loop must
   push its own bookkeeping unattended. So for BS45_Quantum_Explorer ONLY, a
   push to main is allowed silently IF every file in the outgoing commits is
   bookkeeping (HANDOFF, seed ledger, checker command, results/). If ANY other
   path is touched — src/, tools/, any driver script — it still asks.
   Fails CLOSED: if the outgoing file list can't be determined, it asks.
"""
import json, os, re, subprocess, sys

PROTECTED = ("main", "master")

# --- narrow exception scope --------------------------------------------------
EXCEPTION_REPO = "BS45_Quantum_Explorer"
BOOKKEEPING = (
    "HANDOFF.md",
    "cluster/deploy/seed_ledger.txt",
    "cluster/deploy/checker_cmd.txt",
    "results/",              # prefix
)


def _git(*args, timeout=5):
    try:
        out = subprocess.run(["git", *args], capture_output=True, text=True, timeout=timeout)
        return out.stdout.strip() if out.returncode == 0 else None
    except Exception:
        return None


def current_branch():
    return _git("symbolic-ref", "--short", "HEAD") or ""


def repo_name():
    top = _git("rev-parse", "--show-toplevel")
    return os.path.basename(top) if top else ""


def push_targets(seg):
    """Given ONE `git push ...` command segment, return the branch names it lands on."""
    toks = seg.split()
    if len(toks) < 2 or toks[0] != "git" or toks[1] != "push":
        return []
    args = [t for t in toks[2:] if not t.startswith("-")]  # drop flags (-f, -u, --tags…)
    # Drop the remote (first non-flag arg) if a refspec follows it.
    refspecs = args[1:] if len(args) >= 2 else []
    if not refspecs:
        # bare `git push` (possibly `git push origin`) -> pushes the CURRENT branch
        return [current_branch()]
    targets = []
    for r in refspecs:
        dst = r.split(":")[-1]                      # src:dst -> dst ; plain -> itself
        dst = re.sub(r"^refs/heads/", "", dst)
        if dst in ("HEAD", ""):                     # `HEAD` or `HEAD:` -> current branch
            dst = current_branch()
        targets.append(dst)
    return targets


def outgoing_files(branch):
    """Files changed in commits that this push would send.
    None = unknown (fail closed) · [] = nothing to push (harmless no-op)."""
    base = f"origin/{branch}"
    if _git("rev-parse", "--verify", base) is None:
        return None                                  # no upstream ref -> can't tell
    # Zero outgoing commits => the push is a no-op. Don't prompt for nothing.
    count = _git("rev-list", "--count", f"{base}..HEAD")
    if count is None:
        return None
    if count.strip() == "0":
        return []
    files = _git("diff", "--name-only", f"{base}..HEAD")
    if files is None:
        return None
    return [f for f in files.splitlines() if f.strip()]


def is_bookkeeping_only(files):
    if files == []:
        return True                                  # nothing to push -> harmless no-op
    if not files:
        return False
    for f in files:
        if not any(f == b or f.startswith(b) for b in BOOKKEEPING):
            return False
    return True


def main():
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return
    cmd = (payload.get("tool_input") or {}).get("command", "")
    if "git push" not in cmd:
        return

    # Examine each `git push` inside a possibly-compound command.
    for seg in re.split(r"[|;&]+", cmd):
        seg = seg.strip()
        if not seg.startswith("git push"):
            continue
        for target in push_targets(seg):
            if target not in PROTECTED:
                continue

            # --- narrow bookkeeping exception (BS45 loop only) ---
            if repo_name() == EXCEPTION_REPO:
                files = outgoing_files(target)
                if files is not None and is_bookkeeping_only(files):
                    return                            # allow silently: bookkeeping only
                detail = ("could not determine the outgoing files"
                          if files is None else
                          "these commits touch: " + ", ".join(files[:6]))
                print(json.dumps({"hookSpecificOutput": {
                    "hookEventName": "PreToolUse",
                    "permissionDecision": "ask",
                    "permissionDecisionReason": (
                        f"Push lands on '{target}'. The BS45 loop may push BOOKKEEPING only "
                        f"(HANDOFF, seed ledger, checker_cmd, results/), but {detail}. "
                        "Code changes need a human. Confirm this is intentional."),
                }}))
                return

            print(json.dumps({"hookSpecificOutput": {
                "hookEventName": "PreToolUse",
                "permissionDecision": "ask",
                "permissionDecisionReason": (
                    f"This command pushes to '{target}' — a protected branch that often "
                    "triggers a LIVE production deploy. Confirm this is intentional."),
            }}))
            return


if __name__ == "__main__":
    main()
