# docs/ index

Start with `../HANDOFF.md` (live state). This folder holds everything longer-lived, grouped by purpose.
New files go in the matching folder; anything superseded moves to `archive/` (never deleted).

## plans/ — what we are about to do
| File | What it is |
|---|---|
| `plans/pass_h_plan.md` | Next fleet-wide change: 64-element group + closure prune + orbit-min order, with launch gates |
| `plans/lever19_sweep_plan.md` | Pass G, the running lane tiling of all 12 n=44 classes (ledger of windows and lanes) |
| `plans/wz_firsthit_plan.md` | The original first-hit architecture plan and its measurement gates |

## research/ — what we know and why
| File | What it is |
|---|---|
| `research/n44_search_narrowing_research.md` | Levers ledger: every speed or coverage change with its measured numbers |
| `research/wz_paper_reconstruction.md` | Wang-Zhu 2025 paper reconstructed: theorems 2.2/2.3 and equations 2.11b/2.12 |
| `research/2026-07-28-per-arm-candidate-resume-design.md` | Checkpoint/resume design used by every lane |

## briefs/ — written for people outside the project
| File | What it is |
|---|---|
| `briefs/external_review_brief.md` | Problem, solver and results for outside reviewers (Astra) |
| `briefs/kotsireas_brief.md` | Methods brief for Prof. Kotsireas |
| `briefs/paper_methods_record.md` | Methods and provenance record for the paper |

## reviews/ — dated reviews and their evidence
- `reviews/YYYY-MM-DD-*.md`: reviews by Astra (math), Claude and Codex, newest dates last.
- `reviews/prompts/`: the exact prompts sent to Astra.
- `reviews/evidence/`: raw outputs behind decisions. Job reads by the daily loop sit at the top
  level (`qcanary_*.txt`, `cellsize_*.txt`, `cdpilot_*.txt`); review evidence sits in dated
  subfolders (`2026-09-22/` ...), including the scripts that reproduce it.

## archive/ — superseded, kept for history
Old status docs, work orders, the June solver README, interview prep, and `archive/handoff/`
with HANDOFF entries before 2026-09-15.
