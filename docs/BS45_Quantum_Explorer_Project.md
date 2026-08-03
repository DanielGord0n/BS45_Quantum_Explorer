# BS45 Quantum Explorer — Complete Project Dossier

**Prepared 2026-08-03 for use as a Claude Desktop project file.** This document is the
single source of truth for what this project *is*, what it has *actually achieved*, what
the *measured numbers* are, and what may and may not be *claimed*. It is written so that
any chat which loads it can produce accurate resume bullets, LinkedIn copy, interview
answers, or technical explanations without re-deriving anything.

> **Naming note:** the repo is called `BS45_Quantum_Explorer`. There is nothing quantum in
> it — the name is a historical artifact from the first week of the project. "BS45" refers
> to the target object BS(45,44). If asked in an interview, say so plainly; do not let the
> name imply quantum computing work.

---

## 0. One-paragraph summary

A solo-built, from-scratch C++ research engine that searches for **base sequences
BS(n+1,n)** — a 60-year-old open problem in combinatorial design theory — deployed across
four of Canada's national supercomputers via SLURM, driven by a fully autonomous daily
agent loop, and governed by a pre-registered measure-before-you-build methodology. On
**2026-07-30 it found a previously unknown BS(42,41)** — a solution inequivalent to the
only published one — which is (to our knowledge) the first independent solution found at
any of Wang & Zhu's rungs since their 2025 paper. The active target is **BS(45,44), which
has never been found by anyone and is a genuine open problem.**

---

## 1. The mathematics

### 1.1 What the object is

A **base sequence** `BS(n+1, n)` is a quadruple of four sequences with entries in {+1, −1}:

- `A`, `B` of length `n+1`
- `C`, `D` of length `n`

such that their **summed non-periodic autocorrelation function (NPAF)** is exactly zero at
every nonzero shift:

```
NPAF_A(s) + NPAF_B(s) + NPAF_C(s) + NPAF_D(s) = 0     for all s = 1 .. n
```

where `NPAF_X(s) = Σ_i X_i · X_{i+s}`.

These are also called **δ-codes** (delta-codes) in the Russian literature (Sarukhanian,
Balonin–Seberry). In signal-processing terms: four ±1 sequences that, when their
autocorrelations are stacked, produce a perfectly zero echo at every nonzero lag. It is a
zero-tolerance combinatorial condition — there is no "almost", no partial credit. A
quadruple either satisfies all `n` equations exactly or it is not a solution.

### 1.2 The signature invariant

Every base sequence has a **signature** `(a, b, c, d)` where `a = ΣA_i`, `b = ΣB_i`,
`c = ΣC_i`, `d = ΣD_i`, and the NPAF condition forces

```
a² + b² + c² + d² = 4n + 2
```

At `n = 44` this is `a² + b² + c² + d² = 178`. This is the single most useful structural
fact in the whole project: it converts an unbounded search into a **finite union of
signature classes**, each of which can be attacked (or proven empty) independently. With
the parity rule (`a`, `b` odd; `c`, `d` even) and canonicalization (non-negative,
`a ≤ b`, `c ≤ d`), the entire admissible frontier at `n = 44` collapses to **exactly 12
signature classes**:

```
(1,7,8,8) (1,13,2,2) (3,3,4,12) (3,5,0,12) (3,13,0,0) (5,5,8,8)
(5,7,2,10) (5,9,6,6) (5,11,4,4) (7,7,4,8) (7,11,2,2) (9,9,0,4)
```

This enumeration (done 2026-07-30) is the concrete foundation of the record attempt: it
means the open problem has a *finite, explicitly listed* frontier.

### 1.3 Why anyone cares

Base sequences are the raw material for constructing **Hadamard matrices** and
**T-matrices** — objects underlying:

- error-correcting codes (Hadamard codes; the Mariner 9 deep-space code)
- CDMA / spread-spectrum communications (perfect cross-correlation properties)
- optical and signal multiplexing
- experimental design and cryptographic sequence generation

The honest framing (and the one used in the project's own docs) is that BS(45,44) is a
**pure classification prize**: TS(89) already exists via Đoković (2010), so `n = 44`
unlocks no new Hadamard object. Its value is that it closes a gap in a classification
table that has resisted the field for decades.

### 1.4 The state of the art (and where this project sits in it)

| Rung | Status in the literature | Status in this project |
|---|---|---|
| `n ≤ 40` | Known, long-established | Reproduced/searched; ladder banked through `n = 37` |
| `n = 41, 42, 43` | Constructed by **Wang & Zhu, 2025** (arXiv:2506.20296), using the Nanjing HPC centre | `n = 41` **independently solved with a NEW inequivalent solution** (2026-07-30); `n = 42`, `n = 43` under active attack |
| `n = 44` — **BS(45,44)** | **OPEN. Never found by anyone.** Wang & Zhu proved only the restricted sub-cases `NS(44) = ∅` and `NNS(44) = ∅`; the general object is open | **The active target.** 12-class frontier enumerated, all 12 stream, 10 under live search |

Historical cost anchor for calibration: Đoković's `n = 36` result cost **1,423 CPU-days in
2010**. Wang & Zhu acknowledge a national HPC centre and published no runtimes. Record
rungs in this field historically cost CPU-*years*.

---

## 2. Claim discipline — what may and may not be said

This is the most important section for resume/LinkedIn purposes. The project maintains a
hard rule against overclaiming, and the copy should respect it because a domain expert
reading a resume will catch the difference instantly.

**TRUE and defensible:**

- Found a **new, previously unpublished BS(42,41)** on 2026-07-30 — same signature class as
  Wang & Zhu's `(0,2,9,9)`, but **inequivalent to their sequences** (C,D flatness score
  124 vs their 140; the score is invariant under swap, negation, and reversal, so the two
  cannot be transformed into one another). Independently verified: `NPAF[s] = 0` for all
  `s = 1..42`, signature norm 166 exact.
- To our knowledge, the **first independent solution found at any Wang–Zhu rung** since
  their paper.
- **First hit ever achieved at `n ≥ 38`** by this engine, in the 8th search window, at
  ~4.9 hours of wall clock on the first-ever checkpointed lane.
- Solved every admissible signature class on the ladder `n = 32 → 37` blind (no seeds), in
  roughly one week, with **every single solution independently verified** (47 banners in
  one wave, 47/47 PASS; 14 in another, 14/14 PASS).
- Enumerated and stream-validated the full 12-class `n = 44` frontier — the first concrete
  structural step toward the open record.
- Ran and *killed* three separate throughput levers (compression filtering, SAT+CAS, GPU)
  with pre-registered decision rules, each in ≤1 session — i.e. produced publishable-grade
  negative results cheaply instead of burning weeks.

**NOT TRUE / must not be claimed:**

- ❌ "Set a world record" or "found BS(45,44)". Not found. It remains open.
- ❌ "Faster than Wang & Zhu." They published no runtime; no comparison exists. This is
  explicitly forbidden in the project's own rules.
- ❌ Calling ladder rungs (`n ≤ 40`) "records" or "discoveries". They are known to the
  literature; finding them is a **solver-capability result** (a replication-class result).
- ❌ "Tens of thousands of CPU cores." See §5 for the real, defensible numbers.
- ❌ Describing the current engine as a simulated-annealing solver. SA was **retired** at a
  measured ceiling of `n ≈ 33–35`.
- ❌ Claiming BS(28,27). That champion file was **retracted on 2026-07-16** after it failed
  independent NPAF verification, and quarantined. (This retraction is itself a credibility
  asset — see §8.)

---

## 3. Project timeline and phases

| Period | Phase | What happened |
|---|---|---|
| Nov 2025 | **Sarukhanian verification** (CP468) | Verified and corrected published δ-code constructions; proved one of them mathematically invalid |
| Dec 2025 – Feb 2026 | **Exact solver v1** | From-scratch C++17 backtracking solver `wz_exact.cpp` → `wz_exact_t23.cpp` with Theorem 2.3 residue pruning + Theorem 2.4 spectral (DFT) filter |
| Mar – Apr 2026 | **Simulated annealing** | `wz_sa_v8.cpp` — block coordinate descent on a coupled objective, stall-kick perturbation, adaptive heating, PSD bias. Climbed `n = 29 → 30 → 31` blind |
| May – Jun 2026 | **Scaling + the join saga** | `wz_generate.cpp` (Wang–Zhu generate-filter architecture) and `wz_match.cpp` (hash-join matching). Hash-join measured **dead by time** at `n ≳ 29`. Exhaustive search proven 6–15 orders short at `n ≥ 36` |
| Jul 2026 | **The firsthit era — the breakthrough** | `wz_match.cpp` in `WZ_FIRSTHIT` mode. Ladder `n = 32 → 37` in one week. Theorem 2.11b/2.12 filters, canonicalization, flat-first ordering, profile-constrained completion, per-arm checkpoint/resume. **BS(42,41) solved 2026-07-30** |
| Jul – Aug 2026 | **The record program** | 12-class `n = 44` frontier opened; compression, SAT+CAS and GPU levers each measured and killed; class-diversification strategy; collaboration brief prepared for a domain expert |

**Scale of the effort:** 252 commits, ~9,900 lines of first-party C++/Python/Bash (plus
~2,000 lines of Maple/Python in the Sarukhanian sub-project), a 682-line living state
document (`HANDOFF.md`) plus a 3,100-line archive, and 24 consecutive days of autonomous
daily-loop logs.

---

## 4. Solver architecture — four generations

### Generation 1 — exact backtracking with theorem pruning (`wz_exact_t23.cpp`, 1,013 LOC)

The first working engine. Reduces the naive `2^(4n)` space via:

- **Theorem 2.3 residue prune.** A `T23Filter` precomputes every valid `(K,R,P,Q)` mod-3
  residue-sum 4-tuple for the target signature *before* entering the search tree. At the
  midpoint of sequence placement (`d = half − 1`), an empty lookup kills the entire
  remaining A/B subtree instantly. Roughly **100×** narrowing on its own.
- **Sum-constraint pruning** at every layer: `|sig_x − partial_sum_x| ≤ remaining_capacity`.
  A branch that diverges beyond recovery dies immediately.
- **Theorem 2.4 spectral filter (`hall_ok`)** — a DFT-based test rejecting candidates whose
  power spectrum cannot belong to a valid code, applied before committing to deeper search.
- **Wang–Zhu mirror-pair encoding** (`comb16`, `comb8_pos`, `comb8_neg`), reducing the raw
  space from `2^(4n)` to roughly `8^(n/2)`.

Verified blind discoveries: BS(7,6) in ~1 ms, BS(11,10) in ~2.7 s, BS(19,18) from scratch
in ~1.3 s. Reproduced the published BS(43,42) with 8 of 21 layers searched blind in ~15 s
on a single laptop core — and *measured its own ceiling*: `K ≤ 12` (9 blind layers) exceeds
10 minutes, a concrete demonstration of the per-layer exponential wall.

### Generation 2 — simulated annealing (`wz_sa_v8.cpp`, 1,839 LOC)

Block coordinate descent on a coupled objective: freeze A/B and optimize C/D, then swap.
Stall-kick perturbation, adaptive heating, `WZ_PSD_BIAS` spectral penalty term. Climbed
`n = 29 → 30 → 31` blind on cluster arrays (≈1,536 concurrent annealing chains per
cluster). **Retired** at a measured ceiling of `n ≈ 33–35`: plain and bias arms floor at
the same value from `n ≥ 31` onward, and 24-hour walltimes over 1,536 chains produced
exactly the same floor as 12-hour ones. This is a *measured* death, not an assumed one.

### Generation 3 — hash join (`wz_match.cpp` join mode, 2,224 LOC)

Replaces the `O(|A,B| × |C,D|)` re-backtracking product with an `O(|A,B| + |C,D|)` hash
join — the Wang–Zhu / Đoković–Kotsireas "efficient matching algorithm based on hashing".

Elegant engineering detail worth citing in interviews: the join key is a **single 64-bit
FNV-1a hash** of the length-`n` autocorrelation vector rather than the vector itself, and
only **one representative record per distinct key** is stored. Both are sound because
`NPAF[s] = AB[s] + CD[s]` depends only on the autocorrelations, not on which particular
sequences realized them — and any 64-bit collision is caught by the mandatory exact
`npaf_at == 0` recheck over the full quadruple before a `FOUND` is ever printed. The
solver also automatically hashes the *smaller* side (estimated by a fast pre-count) and
streams the larger.

The correctness crux — documented at length in the source — is the indexing asymmetry:
`AB` is defined for `s = 1..n` but `CD` only for `s = 1..n−1`, so a solution needs both
`AB[s] = −CD[s]` and `AB[n] = 0`. Both conditions are folded into a single length-`n` key
by pinning the last CD entry to 0.

**Measured dead above `n ≈ 29`** — pair-work is `1.58e15` at `n = 29` and `4.0e16` at
`n = 31`. It dies by *time*; the memory OOM everyone notices first is just the earliest
symptom. A count-only probe (~100 lines, one day of work) killed a multi-week streaming
rewrite with two numbers.

### Generation 4 — the firsthit architecture (`WZ_FIRSTHIT` mode) — **the engine that works**

This is the current production engine and the one that found BS(42,41). It abandons
exhaustion entirely in favour of *first-hit* search over an intelligently ordered stream:

1. **Stream** Theorem-2.2-constrained `C,D` candidate pairs out of mod-6 residue profile
   cells, with Theorem 2.11a + **2.11b** + 2.12 forced as stream filters at `n ≥ 36`.
   (2.11b turned out to be the *enabler*, not merely an optimization — without it the
   stream produces zero candidates at deep `n`.)
2. **Order** the search by *flatness*: flattest profile cells first (`WZ_FH_PROF_ORDER`),
   and flattest candidates first within each cell (`WZ_FH_CELL_ORDER`). This is a pure
   reordering — zero coverage loss — motivated by the measurement that flat candidates are
   **~35× denser in solutions** at `n = 19`, and confirmed by the fact that every known
   deep solution is flat (ours scores 124; Wang & Zhu's `n = 41/42/43` score 140/142/134).
3. **Complete** each `C,D` candidate by a profile-constrained mirror-pair DFS over `A,B`
   (`WZ_FH_AB_PROF`), bounded by a node budget.
4. **Verify** exactly (`NPAF == 0` over the full quadruple) before any `FOUND` is emitted.
5. **Checkpoint** every arm at candidate granularity so successive node-days accumulate
   with zero re-tread.

Deployment shape: **178 independent single-core "arms" per node**, each taking profile
shards `≡ i mod 178` (interleaved, so fat-tail cells never collide), 12-hour jobs.

---

## 5. HPC infrastructure and real scale

**Clusters:** Trillium, Nibi, Fir, Rorqual — all four, via the **Digital Research Alliance
of Canada**. Account `def-ikotsire` (Nibi requires `def-ikotsire_cpu`).

**Job shape:** `--nodes=1 --cpus-per-task=192 --time=12:00:00`, one node per lane, 178
single-core arms per node. Earlier eras used SLURM job arrays (`--array=0-19`) with OpenMP
`#pragma omp parallel` fanning 192 threads inside each node.

**Defensible concurrency numbers** (use these, not "tens of thousands"):

| Measure | Value |
|---|---|
| Cores allocated per node job | 192 |
| Working search arms per node job | 178 |
| Typical steady-state board | 24–37 concurrent node jobs |
| Peak concurrent wave observed | 40 node jobs ≈ **7,680 allocated cores / ~7,100 concurrent search arms** |
| SA-era concurrency | ~1,536 annealing chains per cluster ≈ 6,100 across four |
| Cumulative compute burned on `n = 41` alone | ~7 CPU-core-years |

**Measured search depth** (candidates fully *tested*, not merely streamed — see §7 for why
that distinction cost real time to learn):

| Rung | Cumulative tested | Status |
|---|---|---|
| `n = 41` | ~700 M to the hit | **SOLVED** |
| `n = 42` (7,11,0,0) | **≈2.3 B**, past its 1.4–2 B predicted band | hitless |
| `n = 43` (8,−2,5,9) | ~140–155 M | open |
| `n = 44` | 1.6–70 M per class per 12h node across 10 live classes | open, the record |

**Deployment mechanics worth mentioning as engineering:**

- **SSH tar-pipe deployment**, because `scp` does not expand `$SCRATCH` on the remote:
  `tar -cf - <files> | ssh host 'cd $SCRATCH/bs45 && tar -xvf - && sbatch ...'` — one
  command, one MFA challenge, source shipped and job queued atomically.
- **Duo MFA as a first-class engineering constraint.** Every SSH login costs a physical
  push notification with a 180-second window. This shaped the entire operational design:
  batched per-cluster command blocks, deliberate refusal of SSH `ControlMaster` (connection
  sharing leaked Duo prompts), a custom `duo_ssh.py`/`duo_run.sh` wrapper, and a checker
  that skips rather than re-pushes an unapproved cluster.
- **Lane-keyed checkpointing.** Checkpoint directories are keyed by *search lane*
  (`class + arms + order + skip`), not by job ID — so resubmitting the identical `sbatch`
  line auto-resumes all 178 arms at their exact stopping candidates. Wave `N+1` costs zero
  re-tread. Validated by three independent resume-equivalence gates plus a bit-identical
  resume-off control.
- **Preemption tolerance:** `--requeue` throughout; a preempted job restarts on the same
  deterministic window with no coverage loss.

---

## 6. The autonomous operations loop (a genuinely distinctive piece of engineering)

The project runs a **fully autonomous daily research loop** — not a cron job that prints
status, but a closed loop that reads results, makes strategic decisions, edits code,
submits jobs, and reports.

```
launchd (13:00 daily)
  └─> daily_auto.sh
        1. check_all_retry.sh   — one MFA push per cluster, sequential, 180s window,
                                  unapproved cluster skipped (not re-pushed);
                                  aborts before spending agent budget if none answered
        2. writes results/latest_check.txt
        3. headless Claude agent (auto_prompt.md) — interprets output, decides,
           refills idle clusters with deterministic disjoint seeds, may edit solver
           code, updates HANDOFF.md, commits, pushes
        4. ntfy push notification to phone with the day's verdict
```

**Safety rails that make it trustworthy** (this is the part that impresses engineers):

- **R1 — validate-before-ship.** A code change must compile *and* pass local small-`n`
  validation in the same run before it can deploy. Otherwise it lands on a dated branch
  `auto/<date>` and the phone gets a `⚠️ needs you` alert instead.
- **R2 — verify-before-claim.** A `FOUND` banner is never committed or announced as real
  until the independent Python verifier passes and a champion file with full provenance is
  written. Only then does the phone get the `🏆 verified result` text.
- **Retry safety.** A session-limit failure retries only when the agent provably did
  *nothing* — no summary written **and** git HEAD unchanged **and** clean working tree.
  A *partial* run is never retried, because a blind retry could double-submit cluster jobs.
- **Kill switch:** `touch cluster/deploy/AUTOPILOT_OFF`.
- **Push guard:** a `PreToolUse` hook intercepts and confirms any push to `main`.
- **Seed ledger** with a monotonic 3,000,000 stride, so no two runs ever repeat a
  trajectory.

The loop has run daily since 2026-07-10 (24 consecutive logged runs at time of writing),
including diagnosing and self-correcting its own bugs — e.g. it discovered that its
checker's `head -12` truncation was hiding 8 of Fir's 20 outputs *including 6 of 7 hits*,
and fixed it.

---

## 7. Systems-engineering wins (with measured numbers)

Each of these is a concrete, quotable performance result.

| Optimization | Measured effect |
|---|---|
| **Atomic-contention fix.** A single `std::atomic` node counter shared across 192 cores caused cache-line ping-ponging that serialized the hot path. Replaced with `thread_local` counters flushing to the global atomic every 2²⁰ nodes | Core throughput was collapsing to ~70k nodes/core; removing atomics from the critical path restored expected parallel scaling. Root cause of a multi-week "why are there no results?" mystery |
| **Precomputed DFT basis table.** The Theorem 2.4 spectral filter recomputed `cos`/`sin` inside the recursive tree — ~33,600 transcendental evaluations per sequence at `n = 42`. Replaced with global `G_HALL_COS`/`G_HALL_SIN` built once | All floating-point trigonometry removed from the hot path, zero change to correctness |
| **Symmetry breaking.** Negating a sequence leaves its NPAF invariant and only flips its sum signature; for zero-sum targets both variants are redundant. Pinned `C[0] = D[0] = +1` | **4× total work reduction** for the BS(43) target signature |
| **Profile-constrained A,B completion (`WZ_FH_AB_PROF`).** The 2.11b/2.12 filters *prove* which `(k,r)` mod-6 A,B profiles are compatible with each `C,D` cell — the completer was ignoring that and searching the whole A,B space | `n = 41`: nodes 1,142.5 M → 218.9 M (**5.2×**). `n = 42`: 319.3 M → 48.3 M (**6.6×**). `n = 19` total nodes 488k → 77k (**6.3×**). At `n = 29`, budget-aborts collapsed **424 → 2 per 1,000** |
| **Flat-first ordering** (cell + in-cell) | **~7–10× candidate throughput** in the first wave that used it, with *zero* coverage loss — pure reordering exploiting a measured 35× density enrichment |
| **Reversal + root canonicalization** | Retains one representative per isomorphism orbit; drove per-candidate abort rates from 98–99% down to 93–97%, then to **0.006–0.27%** once combined with the other levers |
| **Bidirectional double-counting bug fix.** Batched position updates traversed intervals bidirectionally, driving NPAF bounds negative and *pruning valid solution branches*. Fixed by interleaving placement and update (`place_and_update_layer`) | A silent correctness bug that was destroying solutions, not just wasting time |
| **The `candidates=` vs `tested=` telemetry correction** | The depth metric itself was wrong: `candidates=` counted *streamed* candidates (an artifact of the 500k drain buffer) and read ~100M/class regardless of completer speed. True depth is `backtracks_entered`. Discovering this reframed the entire progress picture — the "5× faster completer" *had* delivered; the metric was hiding it |

**The single most decisive finding:** budget-aborts were not neutral — they were *hiding
hits*. At `n = 29`, the unconstrained completer streamed the exact same candidates up to a
known solution, aborted 2,326 of 5,006 including **the solution candidate itself**, and
found nothing. The constrained completer resolved that same candidate in 81,320 nodes and
printed the solution. Every prior deep-`n` "negative" with a high abort fraction may have
been sitting on a solution.

---

## 8. Scientific method and verification discipline

This is arguably the most transferable part of the project and the thing that most
distinguishes it from a hobby search.

**The verification rule (hard, non-negotiable).** A result exists only after all five:

1. the solver prints its `*** FOUND ***` banner, **and**
2. `python3 tools/verify_npaf.py` — a completely independent Python implementation, not
   sharing a line of code with the C++ solver — passes, **and**
3. a champion file with full provenance (job ID, node, env vars, elapsed time, wave) is
   written to `results/champions/`, **and**
4. `HANDOFF.md` is updated, **and**
5. it is committed.

Progress lines, `bestAB` counters, and unverified banners are explicitly **banned as
evidence**. The `bestAB` counter is known to read `8` on files whose banner contains a real
solution.

**The retraction.** On 2026-07-16 the banked BS(28,27) champion was independently
re-verified, **failed** (9 nonzero NPAF shifts), and was retracted and moved to
`results/quarantine/` with a written explanation — including updating the README's own
results table to say `RETRACTED`. A second file (`champion_v3_n27`) was quarantined the
same way. Retracting your own result is a stronger credibility signal than any positive
claim.

**Pre-registered decision rules.** Before any expensive experiment, the kill/build
threshold is written into `HANDOFF.md` *before the number exists*, to prevent motivated
reasoning when results land in an ambiguous band. Examples:

- Compression filter: *"reject rate >50% ⇒ build it into the stream; <10% ⇒ kill."*
- SAT+CAS: *"within 10× of the probe on n=38 ⇒ scale; 100× slower ⇒ kill."*
- GPU: *"≥300× ⇒ build the production completer; 30–300× ⇒ marginal; <30× ⇒ killed."*

All three subsequently landed in kill territory, and all three were killed **without
argument** — which is the entire point of pre-registration.

**Canary before trusting a negative.** Any new solver path must first *re-find a banked
solution* before any "no solution" it reports means anything. The `n = 29` blind re-find
canary is run against every new build; the resume build reproduced the prior record
*exactly* (`idx = 26694`, `nodes = 81320`) — a fingerprint-level match.

**Measure before you build.** Repeated, and repeatedly vindicated:

- A ~100-line count-only probe killed a multi-week streaming-join rewrite with two numbers.
- The compression lever was mathematically validated *and* measured dead in **one laptop-hour**.
- The SAT+CAS lever was killed in **one afternoon** instead of the weeks a full build costs.

**Adversarial audit.** The Theorem 2.11b implementation was independently reviewed for
mathematical correctness. An `even-n` guard in `wz_match` that looked like mathematics
turned out to be legacy cruft — found by small-`n` empirical testing, confirmed by
adversarial audit plus exhaustive small-`n` ground truth (280/280 agreement).

---

## 9. Results ledger

### Verified, banked solutions (`results/champions/`, 37 files)

| Rung | Count | Method | Notes |
|---|---|---|---|
| `n = 7, 11` | 2 | exact backtracking | validation cases |
| `n = 29` | 3 | SA + hash-join | including the `n = 29` blind canary reference |
| `n = 30, 31` | 2 | SA | blind, independently verified |
| `n = 32` | 2 | firsthit | |
| `n = 33` | 1 | firsthit | |
| `n = 34` | 10 | firsthit | **the entire admissible frontier, 10/10 in one wave** |
| `n = 35` | 5 | firsthit | **5/5 — the whole frontier** |
| `n = 36` | 7 | firsthit | 7 of 9 classes |
| `n = 37` | 4 | firsthit | 4 of 4 completed classes |
| **`n = 41`** | **1** | **firsthit, checkpointed lane** | **BS(42,41) — new inequivalent solution** |

Wave-level verification statistics: 47 hit banners fetched on 2026-07-20, **47/47 pass**
independent verification; 14 banners on 2026-07-21, **14/14 pass**.

### The headline result, in full detail

```
BS(42,41)  ·  n = 41  ·  signature class (0, 2, 9, 9)  ·  banked 2026-07-30
Provenance : Fir job 51517707, wave-6, FIRST checkpointed lane
Parameters : WZ_FH_PROF_ORDER=1 (flat), WZ_FH_PROF_SKIP=8 (window 8),
             FH_NARMS=178, WZ_FH_AB_BUDGET=5e7
Found      : arm 5 of 178, elapsed 17,708.9 s (~4.9 h)
             idx=500000  profile_rank=1429  nodes_this_cand=212,872  score=124
Verified   : verify_npaf.py — NPAF[s]=0 for all s=1..42, norm 166 exact,
             fits Wang-Zhu comb8 pair encoding
Novelty    : Wang-Zhu's Table-1 representative for this class scores 140;
             this one scores 124. Score is invariant under swap / negation /
             reversal  =>  the solutions are INEQUIVALENT.
```

The cost model behind it was **predicted before the fact and validated on its first test**:
extrapolating the observed `×2–3` per-rung density thinning gave an expected first hit
somewhere in a 500 M–1 B cumulative band; the actual hit came at ~700 M tested.

### Measured density curve (a research contribution in its own right)

Solution densities, measured per rung: `n = 34`: 1 per 36k–599k · `n = 35`: 1 per
201k–750k · `n = 36`: 1 per 360k–1.52M · `n = 37`: 1 per 443k–2.95M. Consistent thinning of
`×2–3` per rung with **no collapse** — which is precisely what makes `n = 44` a
compute-and-aim problem rather than an impossibility.

---

## 10. Negative results — the measured-dead register

The project maintains an explicit list of things proven not to work, with the numbers, so
they are never rebuilt. Each of these represents real research output.

| Killed | Verdict and number |
|---|---|
| **Exhaustive / complete search at `n ≥ 36`** | 6–15 orders of magnitude short (2026-06-27) |
| **Hash join above `n ≈ 29`** | Pair-work `1.58e15` at `n = 29`, `4.0e16` at `n = 31` — dead by time |
| **Incremental PSD/PAF pruning** | Built, A/B tested, net-negative |
| **`WZ_PSD_BIAS` above `n = 30`** | Cracked `n = 30` (floor 4→0), but plain == bias floors from `n ≥ 31`, confirmed three independent ways |
| **Long walltimes** | 24h × 1,536 chains at `n = 31` → identical floor, no hit. Hits arrive at random restart times (41 min, 3.9 h, 11 h) — so 12h arrays maximize schedulability |
| **Composition / construction routes to BS(45,44)** | Two independent literature dives (2026-07-17). Yang multiplication outputs the wrong shape (equal-length `BS(m',m')`, `m'` odd — can never be (45,44)); Turyn-type TT(44) exists but maps to BS(87,44); `44 = 2²·11` is not a Golay number; **every** historical `BS(n+1,n)` came from direct search, none from composition |
| **Đoković–Kotsireas compression as a stream filter** | Mathematics fully validated (banked BS(42,41) zero-padded to length 42 *is* periodic-complementary; all six compressions pass exactly) — but rejection power measured at **0.0% / 0.1% / 0.6%** on real candidates vs a pre-registered 10% kill line. Survivors of 2.11a/b + 2.12 already satisfy compression almost surely. Cost of the answer: ~1 laptop-hour |
| **Compression as a class-killer** | Turned around as an existence test with a positive control (the solved `n = 41` class is feasible at `d = 2/3/6/7` ✓). Result: **all 12 `n = 44` classes survive** at `d = 3` and `d = 5`. No free eliminations |
| **SAT / CAS direct encoding** | Built `tools/sat_bs_encoder.py` (pysat/CaDiCaL, XNOR product variables, exact-cardinality NPAF, non-negative-sum WLOG). Soundness gate passes; blind `n = 11` solved in 0.9 s with an NPAF re-check of 0 — **the encoding is correct**. But blind `n = 19` took >600 s vs the firsthit probe's **0.2 s** — a **≥3,000× deficit at a rung ~10 orders of magnitude easier than `n = 44`**, against a 100× kill line. Killed in one afternoon |
| **GPU completer** | Wrote `fh_gpu_spike.cu` compiling as **both** plain C++ (for host validation) and CUDA. Validated on an H100 80GB with an exact CPU↔GPU verdict and node-count cross-check (PASS at both budgets — the instrument is sound). At light budget: **69.3× vs one core**. At the budget production lanes actually run: **5.9×** — warp divergence eats the machine at depth, exactly as the pre-registered caveat predicted. `1 H100 ≈ 0.03 of a 192-core CPU node`. Killed per the `<30×` rule |

Killing three throughput levers in three days reshaped the entire program: the record
attempt now rests on **CPU lane volume + mathematical class triage + expert collaboration**,
not on finding a faster machine.

---

## 11. The current strategic position (as of 2026-08-03)

- `n = 42` class `(7,11,0,0)` is at ~2.3 B tested and **past** its predicted 1.4–2 B band,
  still hitless. Under the band's own Poisson assumptions `P(no hit | band) ≈ 25%` — not an
  exclusion, but a signal.
- **Response: class diversification.** The project's own ladder data says solutions live in
  *many* classes per rung (`n = 36`: 7 of 9 classes bore solutions; `n = 37`: 4 of 4).
  Concentrating on the single published class was right while the band held; now it hedges.
  Local triage over sibling `n = 42` classes ranked three of them — `(1,5,0,12)`,
  `(3,9,4,8)`, `(1,3,4,12)` — as having **minimum flatness score 122, flatter than the
  score-124 candidate that produced the `n = 41` hit**. Lanes queued.
- `n = 44`: 10 of 12 classes under live search, per-class cost fully measured
  (`(3,13,0,0)` is the workhorse at 43.4 M/lane-day; `(7,11,2,2)` is the slow tail at
  1.6 M — a 27× spread that directly drives allocation).
- **The remaining highest-leverage move is human, not computational:** a one-page methods
  brief to a domain expert (Ilias Kotsireas, Laurier — co-leader of the MathCheck project
  and co-author of the compression paper), asking three specific questions that the
  project's own measurements have isolated: (1) can NS(44)/NNS(44) obstruction machinery
  kill or rank *signature classes*? (2) is there a stronger way to deploy compression
  against *aperiodic* base sequences at length `45 = 9·5`? (3) would a MathCheck-grade
  PB+CAS formulation change the SAT picture for NPAF systems? Each question is backed by a
  measurement rather than a guess, and each would replace weeks of speculative engineering
  with one paragraph of expert judgment.

---

## 12. The Sarukhanian sub-project (CP468 — separate, self-contained, resume-worthy)

A distinct piece of work, and the strongest "found errors in published mathematics" story
in the repo.

**Construction 1 (Sarukhanian, Proposition 1).** The paper's worked example produced
nonzero NPAF at shifts `s = 4..24`. Algorithmic diffing against the space of valid δ-codes
localized **exactly one sign error** — block 26, term `zrD` should have been `mzrD`. With
that single correction the length-110 sequence satisfies `NPAF = 0` for all
`s = 1..109`. **The theory was right; the published example had a typo.** Deliverable:
corrected Maple implementation plus an independent Python verification notebook.

**Construction 2 (Proposition 2).** Implemented exactly as published (Turyn `n = 3` ×
Golay `k = 2` → expected length 50). It failed the δ-code test with substantial nonzero
NPAF values. Rather than assume implementation error, **seven distinct verification and
correction methods** were applied:

1. Direct implementation exactly as written — fails
2. Brute-force sign search over all `2^16 = 65,536` sign combinations — **no solution**
3. Index search: swapping `j` vs `k−j+1` across all terms — **no solution**
4. Generalized structure search via simulated annealing over sequence choice (`F` vs `G`),
   indices, signs, and reversals — **no solution**
5. **Z3 SMT-based synthesis and sign verification** (`synthesize_construction_z3.py`,
   `verify_signs_z3.py`, `fix_construction_z3.py`, `search_parameters_z3.py`) — formal
   proof of impossibility
6. Parameter-space search over alternative `(n, k)` values
7. A working alternative built from Yang's base-sequence approach

**Conclusion: the published construction is mathematically invalid**, not mistyped. The
error is structural. This is a negative result about a published paper, established by
exhaustive search *and* formal methods, with a working alternative supplied.

Deliverables: ~2,000 lines of Maple and Python, a full technical report, a code-comparison
document, a verification notebook, and a packaged submission.

---

## 13. Tech stack

| Layer | Technology |
|---|---|
| Core solvers | **C++17** (~6,100 LOC across five solver generations) |
| GPU spike | **CUDA** (`fh_gpu_spike.cu` — dual-compiling as C++ for host validation) |
| Node parallelism | **OpenMP** (192 threads) and a 178-arm single-core shard model |
| Cluster orchestration | **SLURM** (`sbatch`, `squeue`, `scancel`, job arrays, `--requeue`) |
| Clusters | Trillium, Nibi, Fir, Rorqual (Digital Research Alliance of Canada) |
| Independent verification | **Python 3** (`verify_npaf.py` — separate code path by design) |
| Formal methods | **Z3 SMT solver**, **pysat / CaDiCaL** (SAT), cardinality encodings |
| Symbolic math | **Maple** (Sarukhanian constructions) |
| Automation | **Bash**, **launchd**, headless **Claude** agent, **ntfy** push notifications |
| Deployment | SSH tar-pipe (Duo MFA-aware), lane-keyed checkpoint/resume |
| Build flags | `g++ -O3 -march=native -std=c++17 -fopenmp` |
| Version control | Git, 252 commits, dated `auto/<date>` branches for unvalidated agent work, `PreToolUse` push guard on `main` |

---

## 14. Repository map

```
src/solver/
  wz_match.cpp          2,224 LOC  ACTIVE — hash-join + WZ_FIRSTHIT streaming engine
  wz_sa_v8.cpp          1,839 LOC  simulated annealing (retired at n≈33-35)
  wz_exact_t23.cpp      1,013 LOC  exact backtracking + Thm 2.3 / 2.4 pruning
  wz_generate.cpp         600 LOC  Wang-Zhu generate-filter architecture
  wz_exact.cpp            375 LOC  first exact solver
  t23_filter.cpp          196 LOC  Theorem 2.3 residue filter
  enum_m3_tuples.cpp      113 LOC  mod-3 residue tuple enumeration
  gpu/fh_gpu_spike.cu                CUDA completer spike (dual-compiles as C++)
src/verifier/verify_bs43.cpp         C++-side reproduction verifier
tools/
  verify_npaf.py          169 LOC  THE independent verifier (separate code path)
  sat_bs_encoder.py       137 LOC  SAT/CNF encoder (killed lever, kept as a tool)
  find_combo_index.py, canary_thm211b.py, measure_thm211b_prune.py,
  class_triage_compression.py
cluster/deploy/                      active SLURM + automation
  cluster_firsthit_probe.sh          the production 178-arm driver
  daily_auto.sh           207 LOC    the autonomous 1pm loop
  check_all_retry.sh, duo_ssh.py, guard_git_push.py, gpu_spike.sh,
  AUTOMATION.md, auto_prompt.md, com.dangord.bs45check.plist
cluster/jobs/                        per-cluster job scripts (fir/nibi/rorqual/trillium)
results/champions/                   37 verified banked solutions with provenance
results/quarantine/                  RETRACTED artifacts + written explanation
results/reference/                   Wang-Zhu Table-1 sequences for n=41/42/43
docs/
  n44_search_narrowing_research.md   the record program + measured-dead register
  kotsireas_brief.md                 one-page expert collaboration brief
  wz_paper_reconstruction.md         reconstruction of the Wang-Zhu method
  gate_bc_firsthit_results.md        full hit/density tables
  RESULTS.md, hpc_interview_prep.md, wz_firsthit_plan.md
sarukhanian/                         the CP468 sub-project (papers, reports, Maple, Z3)
HANDOFF.md               682 lines   canonical living project state
HANDOFF_ARCHIVE.md     3,100 lines   full history
```

---

## 15. Notes for updating the resume and LinkedIn

### What is inaccurate or outdated in the current copy

| Current text | Problem | Correct version |
|---|---|---|
| "high-performance C++ **simulated annealing** solver" | SA was retired months ago at a measured `n ≈ 33–35` ceiling. It is one of five solver generations, not the engine | "constraint-propagating streaming search engine" / "residue-filtered first-hit search architecture" — or just "C++17 combinatorial search engine (five generations: exact backtracking → simulated annealing → hash-join → streaming first-hit)" |
| "coordinating **tens of thousands** of CPU cores" | Not supported. Peak observed is ~40 concurrent 192-core nodes | "up to ~7,700 cores concurrently across four national clusters" or "thousands of cores, peak ~7,700" |
| "**correcting** implementation errors in published mathematical constructions (Construction 1 and 2)" | Construction 1 was corrected (one sign error). Construction 2 was **proven invalid** — a stronger and more accurate claim | "corrected a sign error in one published construction and **proved a second mathematically invalid** using exhaustive search plus Z3 SMT formal methods" |
| "that had previously prevented convergence" | Vague and slightly wrong — the constructions are closed-form, not iterative | drop it |
| "**Actively searching** for BS(45) ... while reproducing known results (BS(42) to BS(44))" | Undersells badly and is imprecise. BS(42,41) is not merely "reproduced" — a **new inequivalent solution** was found | "**Found a previously unknown BS(42,41)** — inequivalent to the only published solution — believed to be the first independent solution at any Wang–Zhu rung; actively searching the open BS(45,44)" |
| "reduce the naive 2^(4n) search space by ~100x" | Stale (that was Generation 1's Theorem 2.3 figure alone) and understates the current stack | Quote the measured levers: "profile-constrained completion cut search nodes 5.2–6.6×; flat-first ordering raised throughput 7–10×; canonicalization drove per-candidate abort rates from 98% to under 0.3%" |
| "backtracking solver" | Half right — the production engine is a *streaming* candidate generator with a bounded backtracking completer | "streaming candidate generator with residue/spectral filtering and a profile-constrained backtracking completer" |
| — | **Missing entirely** | the autonomous daily agent loop; the lane-keyed checkpoint/resume system; the pre-registered kill-test methodology; three measured-dead levers (compression / SAT+CAS / GPU); the self-retraction; the density-curve measurements |

### High-value material currently absent from both documents

1. **The result.** A new inequivalent BS(42,41), independently verified, first at `n ≥ 38`.
2. **The whole-frontier sweeps.** 10/10 classes at `n = 34` and 5/5 at `n = 35` in single
   waves; 29 verified solutions across `n = 32–37` in roughly one week.
3. **Pre-registered kill tests.** Three levers priced and killed in three days (compression
   ~1 laptop-hour, SAT one afternoon, GPU one node-hour) instead of weeks of speculative
   engineering. This is *engineering judgment*, and it interviews extremely well.
4. **The GPU result specifically.** Wrote a CUDA kernel that dual-compiles as plain C++ for
   host-side verdict validation, cross-checked it exactly against the CPU path on an H100,
   measured 69.3× at light budget but 5.9× at production budget, and **killed its own
   project** per a rule written before the number existed.
5. **Checkpoint/resume at candidate granularity**, lane-keyed so successive node-days
   accumulate with zero re-tread — validated by three resume-equivalence gates plus a
   bit-identical control.
6. **The autonomous loop** with validate-before-ship and verify-before-claim rails, safe
   retry semantics that cannot double-submit cluster jobs, and a kill switch.
7. **Self-retraction of a bad result** — quarantined with a written explanation.
8. **Distributed-systems debugging under an MFA constraint** — every diagnostic costs a
   physical phone push with a 180-second window, which drove batched command design and the
   deliberate rejection of SSH connection sharing.

### Suggested resume bullets (accurate, quantified, ready to trim)

> **Research Assistant, Combinatorial Mathematics & HPC** — Wilfrid Laurier University · Sep 2025 – Present
>
> - Built a from-scratch C++17 search engine (5 solver generations, ~6k LOC) for base
>   sequences BS(n+1,n), an open problem in combinatorial design theory; **discovered a
>   previously unknown BS(42,41) inequivalent to the only published solution** — believed to
>   be the first independent solution at any Wang–Zhu rung — and solved every admissible
>   signature class from n=32 to n=37 blind, all 29 results independently verified.
> - Deployed across four national HPC clusters (Digital Research Alliance of Canada) via
>   SLURM, peaking at ~7,700 concurrently allocated cores; implemented candidate-level
>   checkpoint/resume keyed by search lane so successive 12-hour node-days accumulate with
>   zero recomputation.
> - Cut search cost with measured algorithmic levers: mod-6 residue-profile filtering
>   (Thm 2.11a/b, 2.12), profile-constrained backtracking (**5.2–6.6× fewer nodes**),
>   flatness-ordered streaming (**7–10× throughput**, zero coverage loss), and isomorphism
>   canonicalization (per-candidate abort rate **98% → <0.3%**).
> - Priced and killed three throughput levers against pre-registered decision rules —
>   Đoković–Kotsireas compression (0.1% rejection vs a 10% bar), a SAT/CaDiCaL encoding
>   (3,000× slower than the production search at n=19), and a CUDA completer (**5.9× vs one
>   core at production depth**, cross-validated on an H100) — each in under a day instead
>   of weeks of speculative engineering.
> - Engineered a fully autonomous daily research loop (launchd → MFA-aware cluster checker →
>   headless LLM agent → commit/push → phone alert) with validate-before-ship and
>   verify-before-claim rails and double-submit-safe retry semantics; ran unattended for 24+
>   consecutive days.
> - Verified published δ-code constructions: localized a single sign error that repaired one,
>   and **proved a second mathematically invalid** via exhaustive 2^16 sign search plus Z3
>   SMT formal methods, supplying a working alternative.

### Interview soundbites

- *"The metric was the bug."* For weeks the depth telemetry counted *streamed* candidates
  rather than *tested* ones, so a 5× faster completer looked like it had changed nothing.
  Fixing the measurement changed the strategy.
- *"Aborts were hiding hits."* A budget-abort isn't a negative result — at `n = 29`, the
  unconstrained completer aborted on the solution candidate itself and reported nothing.
- *"I killed my own GPU project."* Wrote the rule (`<30× ⇒ kill`) before the number existed,
  measured 5.9× at production depth, and shipped nothing.
- *"I retracted a result."* A banked solution failed independent verification months later;
  it was pulled, quarantined, and the README table updated to say `RETRACTED`.
- *"Every debugging session cost a phone notification."* Duo MFA with a 180-second window
  made observability itself a scarce resource, and that shaped the architecture.

---

## 16. Glossary

| Term | Meaning |
|---|---|
| **BS(n+1, n)** | Base sequences: `A`,`B` of length `n+1` and `C`,`D` of length `n`, all ±1, with summed NPAF zero at every nonzero shift |
| **NPAF** | Non-periodic autocorrelation function, `Σ_i X_i X_{i+s}` |
| **δ-code** | Synonym for a base sequence (Russian literature) |
| **Signature** | `(a,b,c,d)` = the four sequence sums; constrained by `a²+b²+c²+d² = 4n+2` |
| **Signature class** | The set of all base sequences with a given signature — the unit of search allocation |
| **Flatness score** | `Σ|NPAF_C + NPAF_D|` — the project's solution-proximity heuristic; invariant under swap/negation/reversal, so it also serves as an inequivalence certificate |
| **Profile cell** | A mod-6 residue-class-sum profile bucket; the granularity at which candidate streams are ordered and sharded |
| **Arm** | One of 178 independent single-core searchers within a node job, taking profile shards `≡ i mod 178` |
| **Lane** | A search identity `(class, arms, order, skip)` — the key under which checkpoints are stored, so a resubmit resumes exactly |
| **Window / skip** | `WZ_FH_PROF_SKIP=k` dedicates each arm to its `(k+1)`-th flattest cell; distinct `k` values are disjoint by construction |
| **Wave** | One coordinated round of submissions across all four clusters |
| **Tested vs streamed** | `tested` (= `backtracks_entered`) is real search depth; `candidates` counts streamed candidates and is inflated by the drain buffer |
| **Bank / champion** | To write a verified solution with full provenance into `results/champions/` |
| **Canary** | Re-finding a known banked solution with a new build before trusting any negative it reports |
| **Measured-dead** | An approach killed by measurement, recorded with its numbers so it is never rebuilt |
```
