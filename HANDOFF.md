# CP493 — BS(45) Solver Project Handoff

**Date**: 2026-06-03 (project moved to ~/Projects; cluster reality + combo-294887 finding — read TOP OF MIND first)
**Student**: Daniel Gordon (dangord on Alliance clusters)
**Supervisor account**: def-ikotsire (Nibi: `def-ikotsire_cpu`)
**Goal**: Find BS(45,44) δ-codes — a world record. Currently validating with BS(43,42).

---

## ⚡ TOP OF MIND — 2026-06-04: v4 rate measured (fix WORKED ~10×), now bandwidth-bound → v5 (opt C)

**v4's contention fix worked.** First real v4 cluster rates (my deploys):
- Fir 42950724: task 7 = **151.6M/s**, task 8 = 74.4M/s
- Trillium 1703944 (still running): task 4 = 98.8M/s, task 6 = **118.4M/s**
vs v2's 13.5M/s → **~7-11× faster**. The thread-local counters removed the atomic
serialization, exactly as designed.

**But it's now memory-bandwidth bound, not compute bound.** 151M/s ÷ 192 = ~790k/core
(lower tasks ~390-620k/core), still **5-10× below the uncontended single-core 3.95M/s**.
The per-node 2KB snapshot-restore `memcpy` (Dnpaf+Kund, 256 ints each) at ~115M nodes/s ×
192 cores ≈ **230 GB/s** — at/above node memory bandwidth. **→ v5 / optimization C:**
bound the snapshot+restore to the live `[0,n]` range (≈344 B, ~6× less traffic).
Implemented 2026-06-04 in `wz_exact_t23.cpp` (`SNAP_BYTES`), verified: BS(7,6) and
BS(11,10) sig (5,1,4,0) reproduce + pass `verify_npaf.py`. **v5 source is UNCOMMITTED.**

**Operational reality from the run:** at v4 speed a 24h job does only **~745 real
(non-sym-skipped) combos/task** — it doesn't even finish one WAVE (~4369 combos × 1/4
real ≈ 1092). So exhaustion needs many wave-resubmits, and per-combo subtrees are 10B+
nodes (load imbalance). This is what optimization **D** (deepen split 3→4 layers, finer
combos) is for — still pending, do after confirming C's rate.

**Job state right now:** Fir empty (job hit 24h walltime, ~64% through wave 0, cancelled).
Trillium 1703944 still running v4 (~7h left). **Nibi 15526970 still PD — never ran** —
so the cluster that owns combo 294887 hasn't started. Local single-combo reproduction
**died** (laptop sleep) at ~8B+ nodes, no solution reached yet.

**→ Fastest reliable reproduction now:** dedicated combo-294887 job on a cluster node
(`repro294887.sh`, v5) — single core grinds it unattended (won't die from laptop sleep);
also gives a 192-core C-rate reading.

**OPTIMIZATION D BUILT + a key negative result (2026-06-04).** Added env `WZ_SPLIT`
(default 3 = unchanged; clamped to [1,half-1]) — the combo index now fixes the first
WZ_SPLIT layers, search() recurses from there. total_combos = 1<<(7+6*(SPLIT-1)).
Verified: split=3 byte-identical behavior; split=4 reproduces BS(11,10). `find_combo_index.py`
now prints the deep-split combo index of the BS(43,42) solution for K=4..12.

**The negative result that matters:** I hypothesized deep-split → fast *targeted*
reproduction (jump straight to the solution's branch). **It does not.** Pointing split=10
exactly at the solution's 10-layer prefix (combo 1454118867509739495) still explored
**720M+ nodes without reaching the solution** — the *searched suffix* (layers 10..20) is
itself a huge tree because the solution's per-layer comb choices are late in DFS order.
**Conclusion: the bottleneck is search-TREE SIZE (prune strength), not infrastructure or
parallelism.** Per combo the residual tree is ~1e9-1e10 nodes even with all current prunes
(sum+bounds cut ~1e19→~1e9). So exhausting one sig is days-to-weeks at v5 speed, and the
real lever for BS(45) is **stronger/earlier pruning** (tighter NPAF bound, incremental
T23 (P,Q)→(K,R) filtering during CD placement, partial spectral bound before d==half-1,
extra symmetry) — all algorithmic, careful, and a wrong bound silently kills real
solutions (the prior double-counting bug class). D's real value is finer work units for
**load balancing** the monster combos, not reproduction speed.

**What's still well-validated regardless:** the published BS(43,42) tuple fits all 21
layers with NPAF≡0 and sig (7,11,0,0) (`find_combo_index.py`), `verify_npaf.py` confirms
it, and the solver reproduces BS(7,6)/BS(11,10). So solver+encoding correctness does NOT
depend on a blind n=42 reproduction — that's nice-to-have, not a correctness gate.

**PRUNE RESEARCH — profiling result (2026-06-04).** Added `-DINSTRUMENT` per-layer
profiler (zero cost in prod build; `INSTR()` macro + `g_layer_{nodes,bounds,sum}`).
Profiled BS(19,18) (also reproduced it at sigs (7,5,0,0) and (7,3,4,0) — more validation):

```
layer 3:  23K  survive=31% | 4: 461K 21% | 5: 6.1M 10% | 6: 38M 5.3% | 7: 130M 2.7% | 8(=half-1): 226M, bounds kills 99.7%
```

The tree explodes through the MIDDLE layers and only collapses at d==half-1, because the
decisive CD constraints (tight bounds once C,D complete, `hall_ok` spectral, T23 lookup)
all fire only at the end. **→ Highest-leverage prune: pull a SOUND CD constraint earlier
into the mid layers.** Candidates (both soundness-critical, both expensive-per-node — the
real trade-off):
1. **Incremental T23 (P,Q) reachability** — partial (Ppar,Qpar) must still reach some valid
   (P,Q) key in the filter given remaining CD capacity. SOUND (a true solution's (P,Q) is
   always a valid key). Risk: cheap looser version (per-class sets) fired ~0% before (why
   v4 removed it); tight joint-key version is a 6-D range query (expensive). Need a cheap
   sufficient check.
2. **Partial-CD spectral bound** — min achievable |DFT_C|²+|DFT_D|² over undetermined ±1
   already exceeding 4n+2 ⇒ prune. SOUND but ~200 freq × n ops/node.
Method: implement candidate → verify BS(7,6)/(11,10)/(19,18) still reproduce + verify_npaf
→ A/B node-count vs slowdown with the `-DINSTRUMENT` profiler → deploy only if net win.
A wrong bound silently kills real solutions, so soundness proof before every deploy.

**PRUNE CANDIDATE #1 — NEGATIVE RESULT (2026-06-06).** Implemented the incremental T23
(P,Q) reachability prune (sound; `T23Filter::pq_reachable`, env `WZ_PQPRUNE`, off by
default). Soundness verified (reproduces BS(7,6)/(11,10)/(19,18) + verify_npaf). A/B on
BS(19,18) (7,5,0,0) combos[0,1100): nodes 5,392,560 → 5,350,832 = **0.77% cut**, fired
560×, and wall-time got *worse*. Dead for the same reason v4 removed the class prune:
middle-layer `rem` is large so partial (P,Q) almost always reaches some valid key; it's
even weaker at n=42. Code left in, off by default, as a documented dead end.

**Bigger finding: `t23_prunes=0` always — the existing d==half T23 lookup also never
fires.** The real workhorses are the NPAF **bounds prune** + **sum prune**. The Thm-2.3
residue *lookup* contributes ~nothing to pruning; the value of the T23 approach is the
**sig-targeting** (searching only one signature), not the residue filter. So prune
headroom via the residue angle is exhausted. Remaining honest options, in priority order:
1. **Lean into exhaustion + throughput** — C done; deploy **D deeper-split** for load
   balancing (now worth it since prune research stalled). Realistic path to BS(43) in days.
2. **BS(45) sig-selection strategy** — which sig(s) to exhaust matters more than prunes.
3. Riskier research bets (uncertain): partial-CD spectral bound at every layer (expensive
   ~200·n/node); CD-first restructure; DFS reordering for faster *finds* (helps only if a
   solution exists in the searched space — a gamble for BS(45)).

**SPLIT=4 EXHAUSTION SCRIPTS READY (2026-06-07).** All 4 `*_bs43_exact_t23.sh` updated to
`export WZ_SPLIT=4` with recomputed quarters of the 33,554,432-combo space:
Fir [0,8388608) · Rorqual [8388608,16777216) · Nibi [16777216,25165824) · Trillium
[25165824,33554432). Validated in Python: the 4×10task×3wave ranges tile [0,33554432)
EXACTLY (no gaps/overlaps), and the BS(43,42) solution — combo **18644967** at split=4 —
is covered by **Nibi task 2 WAVE 0** (at split=3 it was WAVE 1, i.e. skipped by the default
deploy; split=4 fixes that). Solver builds with cluster flags and reproduces BS(19,18) at
both WZ_SPLIT=4 and default. This is the redeploy to run (scancel -u dangord first; current
jobs are mostly PD so ~no progress lost). Switching split=3→4 restarts exhaustion in the
finer space — fine, since little had run.

**SPLIT=4 DEPLOYED 2026-06-07 (all 4 clusters):** Fir 43373087 · Rorqual 13930953 ·
Nibi 15735364 (solution combo 18644967 in its task-2 WAVE-0) · Trillium 1727347. All PD,
WZ_SPLIT=4, clean tiling. (Cancelled the running Nibi split=3 WAVE=0 job 15663068 — ~5h,
but WAVE=0 structurally can't reach the split=3 solution which sat in WAVE=1.)

**C was ~marginal on cluster (honest):** v5/C ran 83-149M/s vs v4's 74-151M/s — same range.
The 2KB memcpy wasn't the cap; per-placement cost is dominated by `update_bounds_pos`
(O(8n)/node). C is sound, kept, but not the win the bandwidth estimate predicted. split=4's
value is correctness (solution in default wave) + load-balancing, not rate.

**Remaining levers (honest, none deployed):** (a) `update_bounds_pos` rewrite to iterate
only filled positions — biggest throughput lever but the bug-prone area (double-count bug
lived here); (b) more symmetry — C↔D swap is sound (both sum 0, NPAF invariant) for ~2×,
reversal for ~2×, but fiddly; (c) **BS(45) sig-selection from the repo's construction
papers (Sarukhanian_construction.pdf, A_NOTE_ON_CONSTRUCTION_OF_delta_CODES_Rus.pdf)** —
the actual world-record lever, no cluster/solver change needed. Recommend (c) next.

**Uncommitted this session:** C + D + instrumentation + pq-prune(off) in `wz_exact_t23.cpp`,
`find_combo_index.py` (deep-split indices), `repro294887.sh`, split=4 SLURM scripts.

---

## ⚡ TOP OF MIND — 2026-06-03: project moved, cluster reality, combo-294887 finding

**Project was renamed/moved.** New root: `/Users/danielgordon/Projects/BS45_Quantum_Explorer`
(solver + scripts live in the `BS45_Quantum_Explorer/` subdir). Old `School/CP468/...`
paths in this doc were updated. Git history intact; v4 solver builds and reproduces
BS(7,6) after the move (verified locally 2026-06-03).

**Cluster status today (from the user's checker run):**
- **Fir** — queue EMPTY. Output shown was the *legacy* `bs43_exact_*` job killed
  2026-05-27 at walltime, NOT the t23 solver. Needs v4 redeploy.
- **Rorqual** — OFFLINE. Maintenance extended for firmware updates (notice 2026-06-02).
  Login refused. Can't deploy until it's back.
- **Nibi** — queue EMPTY. Needs v4 redeploy.
- **Trillium** — one t23 array job PENDING (`BS43_t23_trilli`, PD/Priority). Only live job.

**Checker-glob bug (was hiding all real data):** the user's checker greps
`bs4*_exact_*output*.txt`, which matches only the dead May-27 legacy jobs, NOT the
current `bs43_t23_*output*.txt`. Every "LATEST" block looked stale because of this.
Use the corrected checker in the "Checker script" section (greps `bs43_t23_*`).

**Why there has been no progress (root cause, now data-backed):**
1. **~56× atomic contention.** Local single-core v4 runs at **3.93M nodes/s**; the v2
   cluster logged **70k nodes/core** (13.5M/s ÷ 192). Cores were serializing on the
   shared `g_nodes` atomic. v4's thread-local counters target exactly this and should
   restore per-core rate toward the uncontended ~3-4M/s → **~40-50× aggregate**, but
   **v4 has never run on a cluster.** Measuring its `rate` is the #1 next action.
2. **Jobs died mid-slice.** Per-combo subtrees are enormous (see below), so 24h jobs
   hit walltime having exhausted only a sliver, then weren't always resubmitted.

**Combo-294887 finding (validation lever).** The published BS(43,42) solution (hardcoded
in `src/verifier/verify_bs43.cpp`) was decoded into the wz_exact_t23 combo encoding by
`BS45_Quantum_Explorer/find_combo_index.py`:
- All 21 layers fit the Wang-Zhu encoding; sig confirmed (7,11,0,0), a²+b²+c²+d²=170, NPAF≡0.
- It maps to **combo index 294887**, which lies in **Nibi's slice [262144,393216)**.
- C[0]=D[0]=+1, so it IS the symmetry-pin canonical representative (not skipped).
- Reproduce with: `./wz_exact_t23 42 7 11 0 0 294887 294888`.

Running that single combo locally (single-thread) confirms the solver is *pointed at the
right place*, but the subtree is **>2.7B nodes** — single-core at 3.93M/s it takes many
minutes/hours of DFS to reach the solution's leaf (192 cores do NOT speed up one combo;
each combo is one thread). The empirical local reproduction is in progress; the encoding
proof already guarantees the solution survives all (sound) prunes and will be found.
**Implication:** when Nibi redeploys v4 and works its slice, it contains the known
solution — Nibi is the cluster most likely to print `REPRODUCTION CONFIRMED` first.

**v4 DEPLOYED 2026-06-03 (first time v4 hits a cluster).** Clean-slate `scancel -u dangord`
then fresh v4 on all reachable clusters:
| Cluster | Job ID | Slice | Status at deploy |
|---------|--------|-------|------------------|
| Fir | 42950724 | [0,131072) | PD (None) |
| Nibi | 15526970 | [262144,393216) | PD (None) |
| Trillium | 1703944 | [393216,524288) | PD (None) |
| Rorqual | — | [131072,262144) | OFFLINE (maintenance, login refused) — redeploy when back |
All `--array=0-9`, WAVE=0 default. **NOTE:** WAVE=0 covers only the first 1/3 of each
task's slice (NWAVES=3). Combo 294887 is in Nibi task 2 **WAVE 1** ([292730,297100)), so
the default Nibi run does NOT reach the known solution — submit `--export=ALL,WAVE=1` on
Nibi to search it, or rely on the local single-combo reproduction. First action when jobs
flip to R: read `rate=` (v2 was 13.5M/s; v4 target 200-500M/s/node).

**Throughput reframe.** With v4's contention fix (~40-50×) across the ~30-40 schedulable
192-core nodes (Fir+Nibi+Trillium; Rorqual when back), exhausting sig (7,11,0,0) drops
from effectively-never to an estimated low-single-digit days of cluster wall (per-combo
~3-10B nodes × ~131k symmetry-reduced combos ÷ aggregate v4 rate). Not hopeless — v2 was
just running ~50× too slow and dying at walltime. **Deploy v4, read `rate`, decide.**

### Session actions (2026-06-03) — what was actually done
1. **Recovered context after the chat was lost** in the project rename/move. Confirmed
   all state lives in HANDOFF.md + git + source (nothing lost).
2. **Fixed stale paths** in this doc (`School/CP468/...` → `~/Projects/...`, 2 places).
3. **Verified v4 builds + reproduces BS(7,6)** locally after the move (sym_pins active,
   4× reduction; `g++ -O3 -std=c++17`, no `-fopenmp` on macOS).
4. **Wrote `BS45_Quantum_Explorer/find_combo_index.py`** (NEW, uncommitted) — decodes the
   published BS(43,42) solution to **combo 294887** and verifies all 21 layers fit the
   encoding. This is reusable for any future known-solution → combo-index mapping.
5. **Started a local single-combo reproduction** `./wz_exact_t23 42 7 11 0 0 294887
   294888` (background, streaming to `/tmp/repro_294887.txt`). As of last check: **>6.3B
   nodes, single-core 3.95M/s, found=no** — solution leaf is deep in this combo's DFS;
   it WILL land (encoding proof guarantees survival of all sound prunes), just slow.
6. **Diagnosed the contention root cause with hard numbers** (local 3.93M/s/core vs
   cluster 70k/core = 56×).
7. **Clean-slate redeployed v4** to Fir/Nibi/Trillium (`scancel -u dangord` first); job
   IDs in the table above. Rorqual still offline.
8. **Wrote persistent memory** (`~/.claude/projects/.../memory/`): `handoff-is-canonical`,
   `bs45-no-results-root-cause` — so a lost chat can't cost continuity again.

### Exact current state / where to pick up
- **Waiting on two signals** (both resolve on their own):
  (a) cluster `rate=` once jobs flip `PD`→`R` — run the corrected checker, read `rate`;
  (b) local `REPRODUCTION CONFIRMED` from the combo-294887 run (watching it).
- **Pre-committed decision tree:**
  - `rate ≈ 200M+/s/node` → contention fix worked. Green-light **D** (deepen combo split
    3→4 layers, 524k→67M combos, for load-balancing the monster combos) and plan the
    **BS(45) pivot** (needs a target sig from `enum_m3_tuples 44 a b c d`, a²+b²+c²+d²=178).
  - `rate still ≈ 13M/s` → thread-local flush path didn't take effect; debug that FIRST.
  - local run prints CONFIRMED → independently verify with `python3 verify_npaf.py` and
    record the validated BS(43,42) tuple here.
- **Not yet done on purpose:** D (wait for rate, don't optimize blind); B (randomized
  combo order) + C (bound snapshot memcpy to `[0,n]`) — safe wins queued for the next
  build; a Nibi `--export=ALL,WAVE=1` job (default WAVE=0 skips combo 294887) — only
  needed if the local reproduction gets interrupted.
- **Redeploy Rorqual** (`rorqual_bs43_exact_t23.sh`, slice [131072,262144)) when
  maintenance clears (Alliance status incident 1598).

---

## ⚡ TOP OF MIND — 2026-05-31: v2 cluster data → v4 (contention fix + drop class prune)

**First real v2 numbers came back** (Fir 42406091, Rorqual 13564805, Trillium
1683719; Nibi 15244284 still PD). The prunes fire massively but the search is
not covering the space fast enough:

```
fir task 7: 73,500s  combos_done=1006  nodes=993B  rate=13.5M/s
            sum_prunes=102B  class_prunes=809k  t23_prunes=0
```

Three findings from the data:

1. **Atomic contention is the real bottleneck.** 13.5M nodes/s across 192 cores
   = ~70k/core — ~30× below what this integer work should do. Every node did
   `g_nodes.fetch_add(1)` on one shared cache line; 192 cores serialize on it.
   v2 added `g_sum_prunes`/`g_class_prunes` atomics (firing 100B+ times) on top.
   **Fix (v4): thread-local counters, flushed to globals every 2^20 nodes.**
   No atomics in the hot path. Expect a large multiplier on `rate`, not 30%.

2. **The per-class residue prune was net-negative.** `class_prunes` ≈ 0.8–12M
   vs `nodes` ≈ 1000B → fired ~**0.001%** of the time, but `class_reachable`
   (~240 bitset probes) ran on every bounds-surviving node. **Removed from the
   hot path in v4.** (T23Filter bitsets + `class_reachable()` left in the code
   for possible future use; just not called.)

3. **The sum-constraint prune earns its keep** (100B fires, ~12-add check) — kept.

v4 also carries forward v3's **4× symmetry reduction** (pin C[0]=D[0]=+1 for sig
(7,11,0,0)) and the **hall_ok DFT-table** speedup. Net expected effect vs the
running v2: much higher node rate (contention gone) × 4 fewer combos (symmetry)
× lower per-node cost (no class prune). This is the version that should actually
exhaust the BS(43,42) space in reasonable wall-time.

`t23_prunes=0` across all tasks is expected: the sum prune kills branches before
the d==half (P,Q) lookup is ever reached.

Validated locally: BS(7,6) (4× pins), BS(11,10) at (5,1,4,0) (2×) and (3,1,4,4)
(1×) all reproduce and pass verify_npaf.py; `sym_skips` = 3/4 of combos as
expected. Draft commit message:

```
feat: wz_exact_t23 v4 — thread-local counters + drop class prune

First v2 cluster data showed 13.5M nodes/s across 192 cores (~70k/core),
~30x below expected: every node hit the shared g_nodes atomic and the cores
serialized on one cache line. v2's added g_sum_prunes/g_class_prunes atomics
(100B+ fires) made it worse.

- Count nodes/sum_prunes per-thread (thread_local), flush to the global
  atomics every 2^20 nodes and at each combo's end. Removes per-node atomics
  from the hot path.
- Remove the per-class residue prune: it fired ~0.001% of nodes but cost ~240
  bitset probes each (net loss). T23Filter bitset infra + class_reachable()
  remain in the source, just uncalled.
- Keep the cheap, high-yield sum-constraint prune, the v3 symmetry pins
  (C[0]=D[0]=+1 for sig (7,11,0,0) => 4x), and the hall_ok DFT table.

Log lines drop the (now-zero) class_prunes field and add sym_skips.
Verified: BS(7,6), BS(11,10) at (5,1,4,0) and (3,1,4,4) reproduce + pass
verify_npaf.py.
```

**Recommend redeploying v4 to all 4 clusters now** — the running v2 jobs cover
space slower than even the old solver, so there's nothing to protect by letting
them finish. Same 4 deploy commands, same SLURM scripts; only
`src/solver/wz_exact_t23.cpp` changed.

---

## ⚡ TOP OF MIND — 2026-05-30 (later): wz_exact_t23 v3 symmetry-breaking

On top of v2's sum + per-class prunes, added **sound single-sequence-negation
symmetry breaking**. Negating exactly one of A/B/C/D leaves NPAF[s] unchanged
for all s (each NPAF term is a self-product within one sequence) and only flips
that sequence's *sum*. So for any sequence whose **target signature component
is 0**, the ±copies share the same signature and the same NPAF — only one is
worth searching. We pin that sequence's first element to +1.

- For the BS(43,42) target sig **(7,11,0,0)**: c=0 and d=0 → pin C[0]=+1 and
  D[0]=+1 → **clean 4× search reduction**.
- General + automatic: `G_PIN_x0 = (G_SIG_x == 0)`, set in main(). Sigs with no
  zero component (e.g. BS(45) candidates like (3,1,4,4)) get 1× — no loss.
- Implemented as a skip in the combo loop *before* any allocation, so it also
  removes ~3/4 of per-combo memset/state-init overhead for the target sig.
- New `sym_skips` counter in every log line; startup prints
  `sym_pins: A0=.. B0=.. C0=.. D0=..  (=> Nx reduction)`.

Verified after the change: BS(7,6) (4× pins) and BS(11,10) at sigs (5,1,4,0)
(2×) and (3,1,4,4) (1×) all still reproduce and pass verify_npaf.py. The pinned
runs return the canonical C[0]=D[0]=+1 representative — a different-but-equivalent
solution than the un-pinned one (expected, not a bug).

**Also in this batch — pure-perf patch (no behavior change):** `hall_ok`
(Thm 2.4 spectral filter) previously recomputed `cos`/`sin` from scratch on
every call (~800·n transcendental evals; ≈33,600 at n=42), and it runs deep in
the tree. Replaced with a precomputed DFT basis (`G_HALL_COS`/`G_HALL_SIN`,
built once in main via `init_hall_tables()`). Identical math, no trig in the
hot path. Both reproductions still pass — confirms it's behaviorally identical.

Draft commit message:

```
feat: wz_exact_t23 v3 — single-sequence-negation symmetry breaking

Negating exactly one of A/B/C/D leaves NPAF[s] invariant (each term is a
self-product within one sequence) and only flips that sequence's sum. So
for any sequence whose target signature component is 0, both signs share
the same sig and the same NPAF; only one representative needs searching.
Pin that sequence's first element to +1.

For the BS(43,42) target sig (7,11,0,0) this pins C[0] and D[0] => clean
4x reduction. Generalised: G_PIN_x0 = (G_SIG_x == 0), so sigs with no zero
component lose nothing. Pins are applied as a skip in the combo loop before
any allocation, also removing ~3/4 of per-combo state-init overhead.

Adds g_sym_skips counter (in every log line) and a startup line reporting
which pins are active and the resulting reduction factor.

Also precomputes the hall_ok (Thm 2.4) DFT basis once in main instead of
recomputing cos/sin per call — pure speedup, identical results.

Verified: BS(7,6) (4x), BS(11,10) at (5,1,4,0) (2x) and (3,1,4,4) (1x) all
still reproduce and pass verify_npaf.py.
```

**NOT yet deployed** — the v2 jobs are running on all 4 clusters. Deploy v3 on
the next redeploy cycle (when v2 jobs finish, or cut them over now). Same 4
deploy commands, same scripts; only `src/solver/wz_exact_t23.cpp` changed.

---

## ⚡ TOP OF MIND — 2026-05-30: wz_exact_t23 v2 prune pass

Diagnosis of the 2026-05-28 status: Fir + Rorqual queues went empty
(prior 24h wz_exact jobs hit walltime; the new t23 jobs were never
resubmitted there). Nibi and Trillium are still running v1 t23 jobs
(Trillium 1662254 ~9h left, Nibi 15118027 tasks 8–9 ~22h left). The
user's checker was also looking at `bs4*_exact_*output*.txt` only, so
the actual t23 outputs (`bs43_t23_*output*.txt`) weren't visible —
fixed in the "Checker script" section below.

### Two new prunes added to wz_exact_t23.cpp (sound; tested)

1. **Sum-constraint prune** — at every layer d, requires
   `|G_SIG_x − partial_sum_x| ≤ rem_total_x` for x ∈ {A,B,C,D}. Sound
   because each remaining position can shift the partial sum by ±1.
   Most selective near d=half-1 (e.g., at BS(43,42) sig (7,11,0,0),
   layer 20 requires partial sumA ∈ {6,8}, partial sumC = 0, etc.).

2. **Per-class residue prune** — T23Filter now precomputes, for each
   class c ∈ {0,1,2}, a bitset of valid K[c], R[c], P[c], Q[c] values
   over the whole tuple set. At every layer the partial Kpar[c] (and
   R/P/Q) must be reachable to some bitset value within the remaining
   capacity of that class. Sound because true K_final[c] is always in
   the bitset (filter built from all valid tuples for the sig).

Both fire at every layer (vs. T23 lookup which fires only at d=half).
Reproduces BS(7,6) in 22 ms and BS(11,10) in 4 ms on macOS (single-threaded);
sum_prunes grows ~32% per combo at n=10, class_prunes is 0 at small n
(bitsets are loose — expected) but should be selective at n=42 with
the tight sig (7,11,0,0).

### Files touched in this session

- `BS45_Quantum_Explorer/src/solver/wz_exact_t23.cpp` — added globals
  `G_NA_CLASS`, `G_NC_CLASS`, `G_PLACED_A_AFTER`, `G_PLACED_C_AFTER`;
  extended `T23Filter` with `allowed_K_set_[3][64]` etc. + new method
  `class_reachable`; inserted sum + class prune blocks in both
  `search()` and `place_and_check` (combo-loop driver); added
  `g_sum_prunes` / `g_class_prunes` counters and threaded them through
  every progress log and exhaustion message.
- `HANDOFF.md` — this section, updated checker, updated log-line format.

Uncommitted. Draft commit message:

```
feat: wz_exact_t23 v2 — sum-constraint + per-class residue pruning

Adds two layer-wise prunes that fire before the existing d==half T23
lookup, so they kill branches as soon as a partial sum or class
residue diverges from any value reachable to a valid (K,R,P,Q) tuple:

- Sum prune: |sig_x - partial_x| <= n_x - 2*(d+1) for x in {A,B,C,D}
- Class prune: any value reachable from partial Kpar[c] within
  remaining class-c capacity must be in the precomputed bitset of
  valid K[c] across all (K,R,P,Q) tuples in the T23 filter (same
  for R, P, Q).

Both prunes are sound (true solutions always satisfy them) and
fire at every layer rather than only at d==half. Together they
should make the search converge orders of magnitude faster on the
combo subtrees that the v1 t23 was iterating through silently
under bounds-prune masking.

Reproduces BS(7,6) in 22 ms, BS(11,10) in 4 ms on a single core.
```

### What to deploy where

Trillium (10 t23 jobs, ~9 h left) and Nibi (2 t23 jobs, ~22 h left)
are mid-flight on the v1 solver; let them run out — they may still
find a solution. **Deploy v2 only to Fir and Rorqual (empty queues)
this round.** When Trillium / Nibi finish without success, redeploy
v2 there too. Sample deploy command in "Deploy pattern" below works
unchanged — the SLURM scripts didn't need to change; just re-tar the
new `src/solver/wz_exact_t23.cpp`.

---

## ⚡ TOP OF MIND — 2026-05-28: Pivot from SA to exhaustive backtracking with Wang-Zhu Thm 2.3 prune

The SA approach (wz_sa_v8 Commits A–H) **never reproduced BS(43,42)** despite 8 commits of layered improvements (BCD coupled refinement, stall kicks, escalating perturbations, per-sig tracking, 1/2/3-pair polish). Plateaus on BS(43) stayed at coupled cost 32-40. Per the user's authorization ("do whatever needs to be done. i need results" + "if that includes building 2.3 then go ahead"), we pivoted to **Wang-Zhu's actual paper algorithm**: complete exhaustive backtracking with theorem-based pruning. The SA solver is preserved as fallback but is **not the active approach**.

### Three solver generations now exist

| Solver | File | Method | Status |
|--------|------|--------|--------|
| `wz_sa_v8` | `src/solver/wz_sa_v8.cpp` | SA + BCD refinement | **Inactive** — never found BS(43,42) |
| `wz_exact` | `src/solver/wz_exact.cpp` | Joint exhaustive backtracking (all sigs), NPAF bounds prune | **Superseded** (canceled 2026-05-28) — ran ~17h across 4 clusters, no signal |
| **`wz_exact_t23`** | **`src/solver/wz_exact_t23.cpp`** | **Sig-targeted exhaustive backtracking + Thm 2.3 m=3 residue prune + Thm 2.4 spectral filter** | **CURRENT — queued on 4 clusters 2026-05-28** |

### Why wz_exact_t23 is the active approach

1. **Wang-Zhu paper's actual algorithm.** Their result is exhaustive backtracking + Theorem 2.3 m=3 residue-class decomposition + Theorem 2.4 spectral filter (hall_ok). SA was never the right algorithm.
2. **Theorem 2.3 m=3 prune**: For target sig, precompute all valid (K,R,P,Q) m=3 residue-sum 4-tuples; index by (P,Q). At CD-placement (d==half-1), look up compatible (K,R) — empty result prunes the AB subtree outright. For BS(43,42) sig (7,11,0,0): **40,824 valid 4-tuples, 1441 unique (P,Q) keys, avg 28 / max 108 compatible (K,R) per key** → ~100× narrowing of AB search vs. unpruned wz_exact.
3. **Sig-targeted**: Wang-Zhu paper explicitly used sig (7,11,0,0) for BS(43,42). Sig-targeting trades sig-coverage for prune strength. Right call for reproduction; would need broader coverage for BS(45).
4. **Reuses bounds prune from wz_exact** (NPAF Dnpaf/Kund interval arithmetic, Commit `a5335ab`).

### Validation — wz_exact_t23 reproduced BS(7,6) in 23 ms locally

```
$ ./wz_exact_t23 6 5 1 0 0
*** REPRODUCTION CONFIRMED: BS(7,6) FOUND ***
sig = (5,1,0,0)
A = {1,-1,1,1,1,1,1};  B = {1,-1,1,-1,1,1,-1};
C = {-1,-1,-1,1,1,1};  D = {1,-1,-1,1,1,-1};
Time: 0.0228974s
```
At n=12 the solver ran 7800/524288 combos in 15s with `t23_prunes` growing correctly (648k prunes across 22M nodes). Confirms both correctness and that the prune fires.

### Critical bug fixed before deploy (do not regress)

**Double-counting in `update_bounds_pos` when batched after `place_layer`.** Original `wz_exact_t23.cpp` first placed all 8 layer positions, then called `update_bounds_pos` once per position. But `update_bounds_pos` scans **bidirectionally** (forward p+s AND backward p-s), so the within-layer partner term (e.g., A[d]*A[n-d] at shift s=n-2d) got counted twice — once when updating A[d] (which finds A[n-d] forward) and once when updating A[n-d] (which finds A[d] backward). Symptom: `Dnpaf[s]` was 2× correct, `Kund[s]` went negative, layer 0 then pruned **every** combo with `t23_prunes=0`.

Fix: new `place_and_update_layer` helper interleaves set + update one position at a time, mirroring `wz_exact.cpp` lines 213-225. Driver lambda and `search()` recursion both use it. See `BS45_Quantum_Explorer/src/solver/wz_exact_t23.cpp:254-283`.

### Files added in this session (uncommitted as of 2026-05-28)

```
BS45_Quantum_Explorer/
├── src/solver/
│   ├── enum_m3_tuples.cpp           ← Standalone Thm 2.3 m=3 tuple enumerator (validation tool)
│   ├── t23_filter.cpp               ← Standalone T23Filter index test (verifies WZ BS(43) tuple is in the set)
│   └── wz_exact_t23.cpp             ← THE CURRENT SOLVER. Sig-targeted backtracking + T23 prune
├── fir_bs43_exact_t23.sh            ← BS(43,42) sig (7,11,0,0) on Fir, combos [0, 131072)
├── rorqual_bs43_exact_t23.sh        ← Rorqual, combos [131072, 262144)
├── nibi_bs43_exact_t23.sh           ← Nibi, combos [262144, 393216)
└── trillium_bs43_exact_t23.sh       ← Trillium, combos [393216, 524288)
```

### Current job state (2026-05-28, post-deploy)

All four wz_exact (joint, sig-untargeted) jobs were canceled because wz_exact_t23 is strictly better for reproduction (targets known-good sig + adds Thm 2.3 prune on top of bounds prune). Replaced with wz_exact_t23:

| Cluster | Job ID | Status | Sig | Combo range | Account |
|---------|--------|--------|-----|-------------|---------|
| Fir | 41964249 | PD (None) | (7,11,0,0) | [0, 131072) | def-ikotsire |
| Rorqual | 13420400 | PD (None) | (7,11,0,0) | [131072, 262144) | def-ikotsire |
| Nibi | 15118027 | PD (None) | (7,11,0,0) | [262144, 393216) | def-ikotsire_cpu |
| Trillium | 1662254 | PD (Resources) | (7,11,0,0) | [393216, 524288) | def-ikotsire |

Each cluster runs `--array=0-9`, splitting its 131072 combos across 10 tasks. Each task is further sub-divided into 3 non-overlapping `WAVE` sub-ranges (NWAVES=3) — submit wave 1/2 once wave 0 finishes. The OpenMP `schedule(dynamic, 64)` causes wave-overlap in single-wave runs (workers re-do the same first ~30% each time), which is why the WAVE env-var split exists.

### Deploy pattern (mirrors prior tar | ssh, one Duo prompt per cluster)

```bash
cd /Users/danielgordon/Projects/BS45_Quantum_Explorer/BS45_Quantum_Explorer && \
  tar -cf - src/solver/wz_exact_t23.cpp <cluster>_bs43_exact_t23.sh | \
  ssh dangord@<cluster>.alliancecan.ca '
    scancel --user=dangord --name=BS43_exact_<cluster> 2>/dev/null;
    scancel --user=dangord --name=BS43_t23_<cluster> 2>/dev/null;
    cd $SCRATCH/bs45 && tar -xvf - &&
    sbatch <cluster>_bs43_exact_t23.sh &&
    squeue -u dangord --format="%10i %25j %2t %12L %R"'
```
`<cluster>` ∈ {fir, rorqual, trilli, nibi} — note "trilli" not "trillium" for the job-name (the script uses `--job-name=BS43_t23_trilli`). Job names: `BS43_t23_fir`, `BS43_t23_rorqual`, `BS43_t23_nibi`, `BS43_t23_trilli`.

Wave 1/2 resubmits when wave 0 finishes:
```bash
ssh dangord@<cluster>.alliancecan.ca 'cd $SCRATCH/bs45 && sbatch --export=ALL,WAVE=1 <cluster>_bs43_exact_t23.sh'
```

### Checker script (covers both wz_exact and wz_exact_t23 outputs)

The user's original checker used `bs4*_exact_*output*.txt` and missed the new
`bs43_t23_*output*.txt` files. Use this updated version — it shows progress
from t23 jobs in addition to legacy wz_exact, and dumps the last 6 progress
lines (so you can see the `sum_prunes` / `class_prunes` growth from the v2
prune pass).

```bash
for c in fir rorqual nibi trillium; do echo ""; echo "════════ $c ════════"; \
  ssh dangord@${c}.alliancecan.ca "squeue -u dangord --format='%10i %25j %2t %12L %R' 2>/dev/null; \
    cd \$SCRATCH/bs45 2>/dev/null || exit 0; \
    echo '--- SOLUTIONS ---'; \
    grep -l 'REPRODUCTION CONFIRMED\|WORLD RECORD' bs43_exact_*output*.txt bs43_t23_*output*.txt bs4*_exact_t23*output*.txt 2>/dev/null || echo '(none yet)'; \
    echo '--- LATEST t23 (active solver) ---'; \
    for f in \$(ls -t bs43_t23_*output*.txt 2>/dev/null | head -3); do echo \"=== \$f ===\"; tail -6 \"\$f\"; done; \
    echo '--- LATEST exact (legacy) ---'; \
    for f in \$(ls -t bs43_exact_*output*.txt 2>/dev/null | head -2); do echo \"=== \$f ===\"; tail -3 \"\$f\"; done"; \
done
```

### wz_exact_t23 log line format (UPDATED 2026-05-31 for v4)

```
[<t>s] nodes=<n> rate=<r>/s combos_done=<c> t23_prunes=<p> sum_prunes=<sp> sym_skips=<ss> found=<yes|no>
[<t>s] COMBO DONE <c>/<total> nodes=<n> t23_prunes=<p> sum_prunes=<sp> sym_skips=<ss> found=<yes|no>
```

Startup also prints `sym_pins: A0=.. B0=.. C0=.. D0=..  (=> Nx reduction)`.
(v4 dropped the `class_prunes` field — that prune was removed; see top-of-mind.)

- `nodes`: total backtracking nodes explored (NPAF-bounds-passing placements).
  Counted per-thread now, flushed every 2^20, so it lags slightly behind the
  true count between flushes — fine for monitoring.
- `combos_done`: first-3-layer combo iterations finished
- `t23_prunes`: times the (P,Q) lookup at d==half returned an empty (K,R) set
  (expected to stay 0 — the sum prune kills branches before d==half)
- `sum_prunes`: branches killed by the sum-constraint prune (the workhorse prune)
- `sym_skips`: combos skipped by symmetry pins. For sig (7,11,0,0) this should
  be ~3/4 of `combos_done` (C0+D0 pins). If it's 0 at (7,11,0,0) the pin flags
  weren't set — check the `sym_pins:` startup line says `C0=1 D0=1`.
- `rate`: nodes/sec — **the v4 win to watch.** v2 was ~13.5M/s aggregate; v4
  should be much higher once the per-node atomics are gone. If `rate` is still
  ~13M/s on a 192-core node, the contention fix didn't take — investigate.
- `found=YES` and a `*** REPRODUCTION CONFIRMED ***` banner → SUCCESS

### Where to pick up

1. **Monitor**: run the checker above periodically. Wave 0 has 24h walltime; expect first signal within 12–24h.
2. **If solution found**: independently verify with `python3 verify_npaf.py < <output>`. Save the tuple. Then deploy BS(45) variants (sig (7,11,0,0) is BS(43)-specific — for BS(45,44) we need a different sig; user picks from `enum_m3_tuples 44 a b c d` candidates satisfying a²+b²+c²+d²=178).
3. **If wave 0 exhausts with no solution**: resubmit WAVE=1, then WAVE=2. After all 3 waves of all 4 clusters fail, the Wang-Zhu sig (7,11,0,0) under our combo encoding might not match the paper's combo enumeration order — would need to re-derive which combo prefix the paper's solution falls into. (Combo indexing in `wz_exact_t23.cpp:432-437` is `ab0|cd0|ab1|cd1|ab2|cd2` bit-packed.)
4. **If even single-wave runs explode in walltime without finishing**: increase combo split depth from 3 layers to 4 (would give 67M combos instead of 524k — finer per-task slicing). This was already done once (Commit `fea3ae6`, 2 layers → 3 layers).

### Uncommitted work (user has not asked to commit)

All wz_exact_t23 work, the three new src/solver files, and four new SLURM scripts are **uncommitted**. User explicitly said "tell me exactly what to do" before deploying, and we deployed straight to clusters without committing. Per user preference (`feedback_no_local_runs.md`), commit when user explicitly asks. A draft commit message is prepared:

```
feat: wz_exact_t23 — Theorem 2.3 m=3 residue-sum pruning solver

Adds Wang-Zhu Thm 2.3 m=3 residue-class decomposition prune to the exhaustive
backtracker. T23Filter precomputes all valid (K,R,P,Q) m=3 sum 4-tuples for a
target signature and indexes them by (P,Q). At CD placement (d==half-1), the
observed (P,Q) is looked up; empty result prunes the AB subtree outright.

Pipeline: enum_m3_tuples.cpp (validator) → t23_filter.cpp (index test) →
wz_exact_t23.cpp (full solver). Smoke-tested at n=6 (BS(7,6) found in 23ms)
and n=12 (t23_prunes counter grows correctly).

Bug fix: place_and_update_layer interleaves set + update_bounds_pos one
position at a time. Previous batched approach double-counted within-layer
partner terms via bidirectional (forward p+s AND backward p-s) update scan,
making Dnpaf 2x correct and Kund go negative — layer 0 pruned every combo
with t23_prunes=0.

Cluster deploys: 4 new SLURM scripts (fir/rorqual/nibi/trillium) targeting
BS(43,42) sig (7,11,0,0) replace the wz_exact joint-enumeration jobs.
```

### Don't-do additions from this session

- **Don't run wz_exact and wz_exact_t23 in parallel.** wz_exact_t23 is strictly stronger for reproduction (sig-targeted to known-good sig + extra T23 prune). Running both wastes half the cluster compute. Per 2026-05-28 user decision, wz_exact was canceled across all 4 clusters.
- **Don't modify the WZ encoding tables in wz_exact_t23** (comb16, comb8_pos, comb8_neg, comb4) — same load-bearing constraint as wz_sa_v8.
- **Don't compile with `-fopenmp` on macOS** with the default clang — it errors out. Cluster gcc has it. Local compile-check uses plain `g++ -O3 -std=c++17` (loses parallelism but works for smoke tests).
- **Don't use `place_layer` (gone — was a foot-gun)**. Always interleave set+update via `place_and_update_layer` to avoid double-counting.

---

## Solver Validation Status

### BS(28,27) — pipeline sanity check PASSED (2026-05-20)

BS(28,27) is a known easy validation case (Daniel has found it before). The current solver
reproducing it just confirms the wz_sa_v8 Commit C–G pipeline works — it is **not** a result.
Found by Commit G's 3-pair polish (Fir job 40543567 task 0, seed 28700, sig (0,-2,-5,9)),
independently verified with `verify_npaf.py` (NPAF[s]=0 for all s, fits Wang-Zhu encoding).
Tuple kept here only as a pipeline-works proof:

```
A = {-1,-1,1,-1,-1,-1,1,1,-1,-1,-1,1,1,1,1,1,1,-1,1,-1,1,-1,1,1,-1,-1,1,-1}
B = {1,1,-1,-1,1,-1,1,1,1,-1,1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,1,-1,-1,1,-1,-1}
C = {1,1,-1,1,-1,-1,1,1,-1,1,-1,1,1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,1,-1,-1,1}
D = {1,-1,-1,1,1,1,1,-1,1,-1,1,1,1,1,-1,-1,-1,1,1,1,-1,1,1,1,-1,1,1}
```

### BS(43,42), BS(45,44) — NOT yet found

BS(43) SA still plateaus at coupled cost ~28-40 (polish trigger threshold is 16, so polish never
fires on BS(43)). BS(43) is the current gate before BS(45). See "Next Steps".

### Known cosmetic bugs (do not affect solution validity)

1. **stdout interleave**: the success block (`#pragma omp critical`) and the tid==0 progress
   logger are not mutually exclusive — on BS(28) the solution banner got interleaved with a
   progress line. The A/B/C/D arrays still printed on clean lines. Fix: have the tid==0 logger
   skip printing once `g_found` is set.
2. **`g_best_ab_cost` not updated by polish**: when `endgame_polish` finds a solution it bumps
   `g_polish_solutions` but not `g_best_ab_cost`, so the log kept showing `bestAB=8` after the
   real answer (0) was found. Fix: `update_min_atomic(g_best_ab_cost, 0)` in the polish success path.

---

## What Is This Problem?

We are searching for **Balonin-Seberry δ-codes BS(n+1, n)**:
Four ±1 sequences A (length n+1), B (length n+1), C (length n), D (length n) such that their joint **Normalised Periodic Autocorrelation Function (NPAF) = 0 at all nonzero shifts**:

```
NPAF[s] = Σ_{i=0}^{n-s} (A[i]*A[i+s] + B[i]*B[i+s]) + Σ_{i=0}^{n-1-s} (C[i]*C[i+s] + D[i]*D[i+s]) = 0
```

BS(43,42) is a known result we are trying to reproduce as validation. BS(45,44) is an open problem — nobody has found it. If we find it, that's a world record.

**Mathematical structure (Wang-Zhu Theorem 2.4)**: Each pair of sequences is represented as mirror-pair 4-tuples (A[d], B[d], A[n1-1-d], B[n1-1-d]) that obey product/sum constraints:

- AB pair d=0: product = -1 → `comb8_neg[8][4]`
- AB pairs d=1..(n1/2-1): product = +1 → `comb8_pos[8][4]`
- CD pair d=0: free (no product constraint) → `comb16[16][4]`
- CD pairs d=1..(n/2-1): product = +1 → `comb8_pos[8][4]`
- Middle position (odd n only): free ±1 ±1 → `comb4[4][2]`

This encoding constrains the search space from ~2^(4n) to ~8^(n/2). **Independently verified on 2026-05-17 (see "Encoding Verification" section below): both Wang-Zhu BS(43,42) and BS(44,43) tuples from the paper fit our encoding perfectly.** The search space is sound.

---

## CRITICAL FINDING (2026-05-14): The "phased" approach was broken

The original "phased CD-then-AB" approach (`wz_sa_bs43.cpp` and v8 pre-Commit-C) had a **mathematically vacuous Phase 1**. The CD "relaxed cost" penalized `|corr_CD[s]| > 2·(n1-s)` — but `corr_CD[s]` is a sum of `(n-s)` terms each in {-2,0,+2}, so `|corr_CD[s]| ≤ 2(n-s) = 2(n1-s)-2 < 2(n1-s)` **always**. The penalty was identically zero. Phase 1 only matched sums (sum_c=tc, sum_d=td) and then handed AB essentially random sum-correct (C,D) pairs, almost none of which admit a compensating (A,B).

This explains why for ~6 months **the solver never actually found any BS solution** (no git evidence of `REPRODUCTION CONFIRMED` in any historical log).

**The fix (Commit C — see Algorithm Evolution below): block coordinate descent (BCD) on the true coupled objective.** CDState::cost now takes an optional `ab_full` parameter; when provided, the CD cost becomes `Σ|corr_CD[s] + ab_full[s]|` (the coupled NPAF residual). The main loop alternates: freeze AB → CD optimizes to cancel AB → freeze CD → AB optimizes → repeat. Each half-step is guarded so it never regresses coupled cost.

---

## Algorithm Evolution (Commits A → E)

The current solver applies five layered improvements on top of the original phased structure.

| Commit | Date | Change | Effect on cluster plateaus |
|--------|------|--------|----------------------------|
| Pre-A | — | Naive phased (broken Phase 1) | BS(28)=16, BS(34)=32, BS(43)=40-48 |
| A | 2026-05-13 | AB-phase diagnostics (`shifts_top`, `term_hist`, `ABterm`, `ab_resid`) | None (additive only) |
| B | 2026-05-13 | AB champion sharing per sig + multi-AB-per-CD 5→15 | None — proved CD was the bottleneck |
| **C** | **2026-05-14** | **Alternating CD↔AB refinement (BCD on coupled objective)** | **BS(28) 16→8, BS(34) 32→16-24** |
| D | 2026-05-14 | Stall detector + CD perturbation kick (2-3 pairs) between BCD rounds | BS(43) 40-48 → 32-38; BS(28)/BS(34) unchanged |
| E | 2026-05-17 | Escalating kick magnitude (2..7 pairs) + AB-side kicks + per-sig bestAB tracking | Currently running |

### Commit A — Diagnostics (lines ~250-265 and ~880-905)
Added globals: `g_ab_shift_residual[128]`, `g_ab_term_hist[256]`, `g_ab_terminations`, `g_ab_sum_residual`, `g_ab_npaf_residual`. In `solve_AB_SA` before the final return, when `0 < best_cost < 64`, records the per-shift residual, the termination cost bucket, and splits the residual into sum-mismatch vs NPAF-penalty.

### Commit B — AB champion sharing (mirrors `g_cd_champ`)
Added `g_ab_champ` per-sig champion vector. In `solve_AB_SA` on restart>0, 30% chance to warm-start from champion. Champion's `corr` stays valid (AB self-correlation); the cost is recomputed against the current `cd_full` (different per CD success). Multi-AB-per-CD bumped 5→15 with adaptive early-exit (now superseded by Commit C structure).

### Commit C — Alternating refinement (THE structural fix)
- `CDState::cost(tc, td, n1, n, const int *ab_full=nullptr)` — optional coupled objective. When `ab_full` set: `pen = Σ|corr[s] + ab_full[s]|`. Cost=0 ⇒ full NPAF=0 solution.
- `solve_CD_SA(..., int sig_idx, const int *ab_full=nullptr)` — same param threaded through. Champion warm-start recomputes cost in refinement mode.
- Main loop restructured: initial AB pass (4 tries) against warm-start CD; then up to 16 refinement rounds doing freeze-AB→CD-step→freeze-CD→AB-step. Both block-steps guarded by `keep-better-cost`.
- `update_min_atomic(g_best_ab_cost, ...)` from refinement-mode CD solves so `bestAB` in logs reflects best coupled cost from either direction.

### Commit D — Stall + CD perturbation
Tracks `prev_coupled`, `stall_count`. When 2 consecutive rounds fail to improve coupled cost, kicks `best_cd` by resampling 2-3 random pairs from the comb tables (same mechanism as the in-SA k-pair kick, lifted to BCD level). Counters: `g_refine_rounds`, `g_refine_kicks`.

### Commit E — Escalating + AB-side perturbation + per-sig tracking
- Restructured stall check to END-of-round (after both block-steps), using post-round coupled cost. This is what lets AB kicks actually affect the next round's CD-step target.
- 50/50 coin flip between CD-kick and AB-kick.
- `kick_level` grows by 1 per kick (cap 4 → max 7-pair kicks); resets to 0 on any coupled-cost improvement. Counter: `g_refine_kick_escalations` (logged as `kesc`).
- `g_sig_best_ab[1024]` atomic array — tracks lowest coupled cost seen per signature. Updated from both `solve_AB_SA` diagnostic block AND each refinement round. Logged as `sig_best=idx:cost,...` (top-5 lowest).
- `kRefineRounds` 12 → 16.

---

## Current SA Parameters (`SAParams` struct, line ~177)

```
initial_temp     = 50.0
cooling_rate     = 0.9999     (faster cooling for shorter cycles)
iterations       = 500000     (iterations per restart cycle)
restarts         = 20         (restart cycles per epoch)
reheat_threshold = 50000      (reheat if no improvement for this many iters)
reheat_ratio     = 0.25       (reheat to 25% of initial_temp)
```

Plus the in-SA k-pair kick: triggers at `no_improve > 30000 && best_cost > 0`, resamples 2-3 pairs, reheats to 0.5×initial.

---

## The Canonical Solver File

**`BS45_Quantum_Explorer/src/solver/wz_sa_v8.cpp`** (now ~1320 lines after Commits A-E)

This is the only solver being used. All other solver files in the repo are historical or deleted.

### Compilation (used in every SLURM script):
```bash
g++ -O3 -march=native -std=c++17 -fopenmp -o wz_sa_v8 src/solver/wz_sa_v8.cpp
```

### Invocation:
```bash
./wz_sa_v8 <n> [seed_offset] [a,b,c,d]
# e.g., ./wz_sa_v8 42 13900           (BS(43,42), seed offset 13900, random sig)
# e.g., ./wz_sa_v8 27 28500           (BS(28,27))
# e.g., ./wz_sa_v8 42 80000 7,11,0,0  (BS(43,42) LOCKED to known-good sig — diagnostic)
```

The optional 3rd arg `a,b,c,d` locks signature selection to a single sig — used by `rorqual_bs43_debug_lockedsig.sh` to test whether SA can find the known BS(43,42) when given the correct sig.

---

## Encoding Verification (2026-05-17)

Critical sanity check: do the known Wang-Zhu solutions actually live in the search space our solver explores? **YES.** Verified via Python check against the hardcoded sequences in [`src/verifier/verify_bs43.cpp`](BS45_Quantum_Explorer/src/verifier/verify_bs43.cpp):

| Property | BS(43,42) | BS(44,43) |
|----------|-----------|-----------|
| Signature (a,b,c,d) | (7, 11, 0, 0) | (8, -2, 5, 9) |
| a²+b²+c²+d² = 4n+2 | 170 = 170 ✓ | 174 = 174 ✓ |
| Parity match `get_sigs` filter | a,b odd; c,d even ✓ | a,b even; c,d odd ✓ |
| AB pair d=0 product = -1 (comb8_neg) | ✓ | ✓ |
| AB pairs d=1..⌊n1/2⌋ product = +1 | ✓ (all 20) | ✓ (all 21) |
| CD pairs d=1..⌊n/2⌋ product = +1 | ✓ (all 20) | ✓ (all 20) |
| NPAF[s] = 0 for all s | ✓ | ✓ |

**This means**: every commit of compute has been searching the right space. The remaining problem is purely search hardness, not search-space exclusion. The Wang-Zhu encoding does admit real BS solutions.

---

## Log Line Format (current, post-Commit E)

```
[t s] epochs=N speed=S bestCD=X bestAB=Y CDok=A/B ABtry=C ABterm=D ABchamp=E
       ABskip=F refine=G kicks=H kesc=I ab_resid=sum:X/npaf:Y
       shifts_top=s7:240,s3:180,... term_hist=40:34,48:21,...
       sig_best=14:4,32:8,7:8,...
```

Field meanings:
- `bestCD`: best CD cost ever seen (always 0 in normal runs — sum-matching trivial)
- `bestAB`: **best coupled cost ever seen** (from either AB phase or CD-against-AB phase). **0 = full NPAF=0 solution found**
- `CDok / total`: CD successes / total CD attempts. Rate ~0.3% (just hitting sum targets)
- `ABtry`: total AB SA invocations (~15-50× CDok with current settings)
- `ABterm`: AB attempts that terminated in the sampled plateau range (0<cost<64)
- `ABchamp`: warm-starts taken from `g_ab_champ` per-sig pool
- `ABskip`: AB attempts skipped by old adaptive early-exit (Commit B; mostly dead in C/D/E)
- `refine`: total alternating BCD rounds executed
- `kicks`: perturbation kicks fired between BCD rounds (Commits D/E)
- `kesc`: kicks fired with escalated magnitude (`kick_level > 0`, Commit E)
- `ab_resid=sum:X/npaf:Y`: accumulated residual broken into sum-mismatch (X) vs NPAF (Y)
- `shifts_top`: top-5 shifts by accumulated NPAF residual
- `term_hist`: top-5 termination cost buckets by count (modal = where AB usually gets stuck)
- `sig_best`: **top-5 signatures by lowest coupled cost seen** (Commit E key diagnostic — if one sig is far ahead of others, focus future compute there)

---

## Plateau values across commits (empirical data)

| Commit | BS(28) bestAB | BS(34) bestAB | BS(43) bestAB |
|--------|---------------|---------------|---------------|
| Pre-A (vacuous Phase 1) | 16 | 32 | 40-48 |
| B | 16 | 24-32 | 40-48 |
| **C** | **8** | **12-24** | (no clean data — old jobs cancelled mid-run) |
| D | 8 | 16 | **32-38** |
| E | TBD (jobs queued 2026-05-17) | TBD | TBD |

**Solution requires bestAB = 0.** Each halving of the plateau is progress; reaching 0 is the goal.

---

## Cluster Access

```
ssh dangord@fir.alliancecan.ca
ssh dangord@rorqual.alliancecan.ca
ssh dangord@nibi.alliancecan.ca
ssh dangord@trillium.alliancecan.ca
```

**All require Duo MFA** — Daniel must approve each connection manually. Cannot be scripted end-to-end.

Working directory on every cluster: `$SCRATCH/bs45`
The entire `BS45_Quantum_Explorer/` folder is synced there.

### Single-Duo deploy pattern (per cluster)

`tar | ssh` bundles upload + extract + sbatch in one SSH session = **one Duo prompt per cluster**:

```bash
cd /Users/danielgordon/Projects/BS45_Quantum_Explorer/BS45_Quantum_Explorer && \
  tar -cf - src/solver/wz_sa_v8.cpp <script1>.sh <script2>.sh ... | \
  ssh dangord@<cluster>.alliancecan.ca '
    scancel --user=dangord --name=<job_name> 2>/dev/null;
    cd $SCRATCH/bs45 && tar -xvf - &&
    sbatch <script>.sh &&
    squeue -u dangord --format="%10i %25j %2t %12L %R"'
```

`tar` ignores macOS resource forks (`._*` and `LIBARCHIVE.xattr...` warnings are harmless).

---

## Current SLURM Scripts (in `BS45_Quantum_Explorer/`)

All scripts: 192 cores/node, `--account=def-ikotsire` (Nibi: `def-ikotsire_cpu`). **Seed offsets bumped by +100 on every redeploy** to avoid reusing RNG trajectories.

| Script | Cluster | Problem | Current seed range (post-E) | Array | Time |
|--------|---------|---------|------------------------------|-------|------|
| `fir_bs28_v8_test.sh` | Fir | BS(28,27) | 28500–28502 | 0-2 | 2h |
| `fir_bs34_v8_test.sh` | Fir | BS(34,33) | 34500–34502 | 0-2 | 2h |
| `fir_bs43_v8_job.sh` | Fir | BS(43,42) | 12400–12409 | 0-9 | 24h |
| `rorqual_bs43_v8_job.sh` | Rorqual | BS(43,42) | 13900–13909 | 0-9 | 24h |
| `nibi_bs43_v8_job.sh` | Nibi | BS(43,42) | 12600–12609 | 0-9 | 24h |
| `trillium_bs43_v8_job.sh` | Trillium | BS(43,42) | 14000–14009 | 0-9 | 24h |
| `fir_bs45_v8_job.sh` | Fir | BS(45,44) | 45000–45009 | 0-9 | 24h |
| `rorqual_bs45_v8_job.sh` | Rorqual | BS(45,44) | 45100–45109 | 0-9 | 24h |
| `nibi_bs45_v8_job.sh` | Nibi | BS(45,44) | 45200–45209 | 0-9 | 24h |
| `trillium_bs45_v8_job.sh` | Trillium | BS(45,44) | 45300–45309 | 0-9 | 24h |
| `rorqual_bs43_debug_lockedsig.sh` | Rorqual | BS(43,42) | 80000–80002 | 0-2 | 24h |

**DO NOT submit the BS(45) scripts until BS(43,42) has been reproduced** (currently never has). Range allowed per handoff is 50 per cluster (45000-45049, etc.) — leaves room for 4 more 10-task waves once we start.

The `rorqual_bs43_debug_lockedsig.sh` script runs `./wz_sa_v8 42 80000 7,11,0,0` — locks all 192 threads to the known-good BS(43) signature (7,11,0,0). Diagnostic: if it finds the solution under that constraint, SA works given the right sig. If it can't even with the right sig, SA itself is the bottleneck and we need a different algorithm.

---

## Current Job State (as of 2026-05-17, Commit E deploy)

| Cluster | Job ID | Status | What it's running |
|---------|--------|--------|--------------------|
| Fir | 40313876 | PD (None) | BS(28,27), seeds 28500-28502, Commit E |
| Fir | 40313877 | PD (None) | BS(34,33), seeds 34500-34502, Commit E |
| Rorqual | 12546331 | PD (None) | BS(43,42), seeds 13900-13909, Commit E |
| Nibi | 14186890 | PD (None) | BS(43,42), seeds 12600-12609, Commit E |
| Trillium | 1595709 | PD (None) | BS(43,42), seeds 14000-14009, Commit E |

The sig-locked diagnostic (`rorqual_bs43_debug_lockedsig.sh`) has NOT been deployed yet — user has the deploy command and can submit when ready.

---

## Cluster Check Script

```bash
for c in fir rorqual nibi trillium; do echo ""; echo "════════════════════ $c ════════════════════"; ssh dangord@${c}.alliancecan.ca "echo '--- QUEUE ---'; squeue -u dangord --format='%10i %25j %2t %12L %R' 2>/dev/null; echo ''; cd \$SCRATCH/bs45 2>/dev/null || exit 0; echo '--- NEW SOLUTIONS ---'; find . -maxdepth 1 -name '*.txt' -mtime -1 -exec grep -l 'REPRODUCTION CONFIRMED\|WORLD RECORD' {} + 2>/dev/null || echo '(none yet)'; echo ''; echo '--- LATEST PROGRESS (last 5 lines each) ---'; for f in \$(ls -t bs43_v8_*_output_*.txt bs28_v8_*_output_*.txt bs34_v8_*_output_*.txt bs45_v8_*_output_*.txt 2>/dev/null | head -4); do echo \"=== \$f ===\"; tail -5 \"\$f\"; echo; done"; done
```

---

## Independent NPAF Verifier (2026-05-17)

**`BS45_Quantum_Explorer/verify_npaf.py`** — standalone Python script. Independent code path from `wz_sa_v8.cpp::npaf_at` — critical for any world-record claim (don't trust the same code that found it).

Usage:
```bash
# Self-test against Wang-Zhu BS(43) and BS(44)
python3 verify_npaf.py --self-test

# Verify a solver output (paste the A = {...}; B = {...}; C = ...; D = ...; block)
python3 verify_npaf.py < solver_output.txt

# Inline
python3 verify_npaf.py --A "1,-1,1,..." --B "..." --C "..." --D "..."
```

Checks NPAF[s]=0 for all s=1..n+1, the signature equation a²+b²+c²+d²=4n+2, parity, and the Wang-Zhu pair encoding.

---

## How to Interpret Progress Logs

Sample healthy Commit E log line:
```
[7000s] epochs=4416 speed=0.62 bestCD=0 bestAB=8 CDok=141/6482 ABtry=9484
        ABterm=9299 ABchamp=53973 ABskip=0 refine=2232 kicks=557 kesc=120
        ab_resid=sum:8/npaf:248392 shifts_top=s1:11890,s3:11826,s2:11610,s4:11582,s8:11390
        term_hist=24:4885,16:1487,32:1033,52:336,28:312 sig_best=12:8,7:8,...
```

**Good signs**:
- `bestCD=0` always (trivial in v8)
- `bestAB` dropping over time (the only metric that matters for solving)
- `refine` and `kicks` growing (BCD machinery active)
- `sig_best` showing some sigs lower than others → sig selection has room to improve
- `bestAB=0` and `*** REPRODUCTION CONFIRMED ***` → SUCCESS

**Warning signs**:
- `CDok=0/N` after >1000 attempts → CD phase broken (the odd-n bug, should be fixed)
- `bestAB` stuck at exactly the same value across all tasks and many hours → either a structural floor OR sig-selection drowning out good sigs
- All `sig_best` entries clustering at the same value → structural ceiling; SA itself can't go lower
- `term_hist` shows no terminations below `bestAB` → BCD never finds the right basin

---

## Next Steps (decision tree)

### When Commit E logs arrive (~6-12h after first job starts running)

**Branch 1 — Commit E breaks through (bestAB < 8 on BS(28), or any REPRODUCTION CONFIRMED)**
→ Move directly to BS(45). Submit the 4 BS(45) scripts.

**Branch 2 — `sig_best` shows one sig FAR ahead (e.g., one at 0-4, others at 16+)**
→ Build "biased sig selection" (Commit F): instead of uniform random, weight signatures inversely by `g_sig_best_ab` (lower-cost sigs get more compute). Re-deploy.

**Branch 3 — All `sig_best` cluster at the same value (e.g., all 8 ± 2 on BS(28))**
→ Structural ceiling, not search problem. Run the sig-locked diagnostic (`rorqual_bs43_debug_lockedsig.sh`). If it ALSO plateaus at the same value with the known-good sig, SA itself is the bottleneck — need different algorithm (CP/SAT for one block, hybrid, theoretical construction from Wang-Zhu paper).

**Branch 4 — Commit E performs similar to D (no further improvement)**
→ Run the sig-locked diagnostic to separate SA-bottleneck from sig-selection-bottleneck. Path forward depends on outcome.

### If BS(43,42) IS finally reproduced
1. Run `verify_npaf.py` on the output to independently confirm.
2. Save the (A,B,C,D) tuple and signature in this handoff.
3. Deploy the 4 BS(45) SLURM scripts: `fir_bs45_v8_job.sh`, `rorqual_bs45_v8_job.sh`, `nibi_bs45_v8_job.sh`, `trillium_bs45_v8_job.sh` (all ready, seed offsets pre-set at 45000s/45100s/45200s/45300s).

---

## Important Constraints / Don't Do These

- **Do not run locally** — macOS doesn't have 192 cores; benchmarks are meaningless. Deploy straight to clusters. (Compile-checking with `g++ -c` for syntax is fine.)
- **Do not use joint (A,B,C,D) SA** — historical attempts (v4-v7) plateaued at 24/32 past n≈27.
- **Do not revert the alternating refinement (Commit C)** — without it, Phase 1 is mathematically vacuous and the solver finds nothing. The CDState::cost coupled-mode is the load-bearing change.
- **Do not modify the Wang-Zhu encoding** — verified to admit real BS solutions; comb16/comb8_pos/comb8_neg/comb4 are mathematically required.
- **Do not reuse the same seed offsets** across redeploys — increment by 100 each time.
- **Do not remove the odd-n middle position code** — needed for BS(28) n=27 and BS(34) n=33.
- **Always confirm with user before pushing to clusters** — they need to approve Duo MFA for each cluster.
- **Do not commit changes without explicit user request** — user prefers to review commits.
- **Do not speculatively build next improvements before data arrives** — costly lesson from Commit B. Wait for diagnostic signal (sig_best, kicks, kesc, plateau values) before deciding what to build next.

---

## History of What Was Tried and Why It Failed

### Previous solvers (deleted in commit `6df3b1c` "Codebase Cleanup")
- **`wz_sa.cpp`** — original, joint (A,B,C,D) SA, no Wang-Zhu encoding. Plateaued at cost=8 for BS(28).
- **`wz_sa_bs43.cpp`** — correct Wang-Zhu encoding, original "phased CD-then-AB" structure. **Now known to have mathematically vacuous Phase 1.** Never actually reproduced BS(43,42) despite the file name. Source for v8.
- **`wz_sa_trillium.cpp`** (v4–v7) — dropped Wang-Zhu encoding entirely, used unconstrained ±1 flips. Fundamental regression. Plateaued at cost=24/32.

### Critical bugs fixed in v8 lineage
1. **Odd-n CD encoding bug (CRITICAL, pre-Commit-A)**: `for (d=0; d<n/2; d++)` only filled positions 0..12 and 14..26 for n=27, leaving C[13]=D[13]=0. `CDok=0/523076` — CD was never solvable. Fixed with `cd_init_random()` middle-position branch.
2. **Missing `#include <tuple>`** — `std::tie` used without the header. Caught by gcc on clusters. Fixed.
3. **Vacuous CD relaxed cost (Commit C, the big one)** — `|corr_CD[s]| ≤ 2(n1-s)` always true; CD was just sum-matching. Fixed by adding coupled objective + alternating refinement.

### Why joint SA doesn't work (per the original analysis)
The joint (A,B,C,D) cost function has a flat landscape. At cost=24 or cost=32, the "hole" toward cost=0 is surrounded by an exponential number of states at higher cost. The Commit C alternating refinement effectively does joint optimization while exploiting the Wang-Zhu block structure for tractable per-block SA.

---

## Files That Matter

```
BS45_Quantum_Explorer/
├── src/
│   ├── solver/
│   │   └── wz_sa_v8.cpp                ← THE solver (~1320 lines after A-E)
│   └── verifier/
│       └── verify_bs43.cpp             ← C++ verifier with hardcoded BS(43)/BS(44) tuples
├── verify_npaf.py                       ← NEW: standalone Python NPAF verifier (2026-05-17)
├── fir_bs28_v8_test.sh                  ← BS(28) test on Fir
├── fir_bs34_v8_test.sh                  ← BS(34) test on Fir
├── fir_bs43_v8_job.sh                   ← BS(43) on Fir
├── rorqual_bs43_v8_job.sh               ← BS(43) on Rorqual
├── nibi_bs43_v8_job.sh                  ← BS(43) on Nibi
├── trillium_bs43_v8_job.sh              ← BS(43) on Trillium
├── fir_bs45_v8_job.sh                   ← BS(45) on Fir (HOLD until BS(43) reproduced)
├── rorqual_bs45_v8_job.sh               ← BS(45) on Rorqual (HOLD)
├── nibi_bs45_v8_job.sh                  ← BS(45) on Nibi (HOLD)
├── trillium_bs45_v8_job.sh              ← BS(45) on Trillium (HOLD)
└── rorqual_bs43_debug_lockedsig.sh      ← NEW: sig-locked BS(43) diagnostic (2026-05-17)
```

Historical/deleted from git:
- `src/solver/wz_sa_bs43.cpp` — original, even-n only, vacuous Phase 1. In git history at `6df3b1c~1`.
- `src/solver/wz_sa_trillium.cpp` — broken, lost Wang-Zhu encoding. In git history.
- `src/solver/wz_sa_v2.cpp` through `wz_sa_v6.cpp` — historical iterations.

---

## Success Criteria

- **BS(28) found**: output contains `*** REPRODUCTION CONFIRMED: BS(28,27) FOUND ***`
- **BS(43) found**: output contains `*** REPRODUCTION CONFIRMED: BS(43,42) FOUND ***`
- **BS(45) found**: output contains `*** WORLD RECORD DISCOVERY: BS(45,44) FOUND ***`

When any solution is found, the output prints the full A, B, C, D arrays and the signature (a,b,c,d). **Always verify independently**:
1. Run `python3 verify_npaf.py < <output_file>` — independent code path.
2. The output should report `PASS: NPAF[s]=0 for all s=1..n+1` and `PASS: fits Wang-Zhu pair encoding`.
3. Save the (A,B,C,D) tuple, signature, cluster, job ID, and seed offset in this handoff for posterity.

For BS(45,44), the world-record claim requires:
- Independent verification via verify_npaf.py
- Manuscript/publication-ready writeup of (A,B,C,D), the signature, the search method, total compute
- Comparison against the Wang-Zhu paper's open-problem statement

---

## Known Wang-Zhu BS(43,42) signature (target for sig-lock diagnostic)

The Wang-Zhu BS(43,42) sequences (in `src/verifier/verify_bs43.cpp`) have:
- **Signature: (a=7, b=11, c=0, d=0)**
- a²+b²+c²+d² = 49+121+0+0 = 170 = 4·42+2 ✓
- All Wang-Zhu pair-product constraints satisfied

This is what `rorqual_bs43_debug_lockedsig.sh` locks the solver to. If the solver can find BS(43,42) under that single-sig constraint, SA works given the right sig. If not, SA itself is the bottleneck.

The Wang-Zhu BS(44,43) sequences have signature **(a=8, b=-2, c=5, d=9)**, a²+b²+c²+d² = 64+4+25+81 = 174 = 4·43+2 ✓.

---

## Quick Reference: Active Commits in wz_sa_v8.cpp

Key line ranges (approximate, post-Commit E; may shift with edits):
- Lines 23-37: includes, namespace
- Lines 42-60: globals + `update_min_atomic` helper
- Lines 62-88: Wang-Zhu comb tables
- Lines 100-175: Sig struct + `get_sigs()` signature enumeration
- Lines 177-184: `SAParams` struct
- Lines 191-208: `CDState` + `CDState::cost(...)` (with optional `ab_full` from Commit C)
- Lines 213-241: `cd_init_random`
- Lines 244-265: `CDChampion`, `ABChampion` declarations + diagnostic globals (Commits A, B, D, E)
- Lines 263-540: `solve_CD_SA(..., int sig_idx, const int *ab_full=nullptr)`
- Lines 540-560: `ABState` + `ABState::cost`
- Lines 560-902: `solve_AB_SA(..., int sig_idx)` (champion sharing + k-pair kick + diagnostic)
- Lines 905-925: `main()` arg parsing (incl. `--lock-sig` from sig-lock CLI)
- Lines 935-970: signature load, champion init, per-sig array init
- Lines 970-1140: thread loop (initial AB + alternating refinement with kicks)
- Lines 1140-1235: solution verification and output
- Lines 1237-1320: tid==0 log block (all the new fields)
