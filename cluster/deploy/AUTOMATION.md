# BS45 autonomous 1pm loop — setup & operations

Fully hands-off daily loop: **check → interpret → act (refill / code change) →
commit + push → text you a summary.** You only tap Duo. Two of your own rules are
wired in as hard rails (see below) — they don't reduce automation, they make the
robot follow your process.

## The chain

`launchd` (1pm) → `daily_auto.sh`:
1. runs `check_all_retry.sh` — ONE Duo push per cluster, one at a time; a cluster you
   don't approve within `PUSH_WAIT` (180 s) is skipped, not re-pushed (re-run to pick
   it up). If NO cluster answered, the run aborts before invoking the agent.
2. saves combined output to `results/latest_check.txt`,
3. runs **headless Claude** (`claude-fable-5`; falls back to `claude-opus-4-8` only on
   a model-unavailable error) with `auto_prompt.md` — it interprets, decides, refills
   idle clusters with deterministic seeds from `next_seeds.sh`, may edit code,
   updates HANDOFF, commits, and pushes. A Claude usage/session limit defers the run
   (30 min × up to 8 retries), retrying ONLY if the agent provably did nothing (no
   summary + HEAD unchanged + clean working tree) — a partial run is never retried.
4. texts you `results/last_summary.txt` via ntfy.

There is deliberately **no SSH connection sharing** (ControlMaster leaked Duo prompts —
see `duo_ssh.py`): every remote command, including each agent submit via `duo_run.sh`,
is its own SSH login and its own Duo tap.

## The two rails (in `auto_prompt.md`)

- **R1 validate-before-ship:** a code change must compile + pass local small-n
  validation THIS run before it deploys; otherwise it lands on branch
  `auto/<date>` and you get a `⚠️ needs you` text.
- **R2 verify-before-claim:** a `FOUND` banner is not committed/announced as real
  until `tools/verify_npaf.py` passes and a champion file is written. Only then do
  you get the `🏆 verified result` text.

## One-time setup on your Mac

1. **Headless Claude CLI, authenticated, on PATH.** `daily_auto.sh` calls `claude`.
   Confirm `which claude` works in a plain login shell (`bash -lc 'which claude'`).
   If it lives somewhere launchd won't see, set `CLAUDE_BIN` to the full path at
   the top of `daily_auto.sh`, or add its dir to PATH in the plist.
   - It must be able to run tools without interactive prompts. Default is
     `--dangerously-skip-permissions` (via `CLAUDE_ARGS` in `daily_auto.sh`): the agent
     can run ANY command in this repo context. Containment = the push-guard hook
     (`~/.claude/tools/guard_git_push.py`), the R1–R3 rails in `auto_prompt.md`, the
     per-submit Duo taps, the kill switch, and reading the logs.
2. **git push auth non-interactive.** The agent runs `git push origin main`. Make
   sure pushing needs no password prompt (SSH key in agent, or cached credential
   helper). Test: `git push` by hand once.
3. **Confirm the seed ledger.** Open `seed_ledger.txt` and set `NEXT_BASE` to the
   true next-free base (as of HANDOFF 2026-07-08: 60M; verify nothing newer ran).
   `N` is the current rung (32).
4. **Load the schedule** (already pointed at `daily_auto.sh`):
   ```bash
   cp cluster/deploy/com.dangord.bs45check.plist ~/Library/LaunchAgents/
   launchctl unload ~/Library/LaunchAgents/com.dangord.bs45check.plist 2>/dev/null
   launchctl load  ~/Library/LaunchAgents/com.dangord.bs45check.plist
   ```

## Controls

- **Kill switch (no unloading needed):** `touch cluster/deploy/AUTOPILOT_OFF`
  disables the run and texts you that it's off; `rm` it to re-enable.
- **Run it now to test** (does everything for real — only when you have jobs to act on):
  ```bash
  ./cluster/deploy/daily_auto.sh
  ```
- **Dry the agent without acting:** set the kill switch, run the checker alone
  (`./cluster/deploy/check_all_retry.sh`), read `results/latest_check.txt`.
- **Logs:** `results/auto_<date>.log` (full agent transcript),
  `results/launchd.{out,err}` (scheduler).

## ⚠️ Watch the first several runs

This is ambitious and inherently fragile: it edits code and pushes to `main`
unattended, and depends on headless auth, non-interactive git, and Duo timing. The
rails stop the worst outcomes (shipping code that didn't build; claiming an
unverified record), but a model acting alone can still make a wrong-but-valid call
that spends a cluster round. Read the summary texts and `auto_<date>.log` for the
first week, and keep `AUTOPILOT_OFF` handy. If anything smells off, flip the switch
and fall back to running `/daily-loop` by hand.
