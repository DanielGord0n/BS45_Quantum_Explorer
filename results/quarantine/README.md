# Quarantine — banked artifacts that FAILED independent verification

Files here are NOT results. They are kept (rather than deleted) as negative test
fixtures and as the audit trail for retracted claims.

## champion_v3_n27.txt — NOT a valid BS(28,27) (quarantined 2026-07-16)

- **Claim it carried:** BS(28,27), sig (6,8,-3,1), banked ~2026-04-21 (v3 SA era),
  cited in README.md and docs/kotsireas_brief.md as "verified".
- **Fact (re-verified 2026-07-16, `tools/verify_npaf.py`):** NPAF non-zero at 9
  shifts — s=6:-2, s=7:-4, s=12:+2, s=14:+2, s=16:+2, s=18:-2, s=20:-2, s=23:+2,
  s=25:+2. It also fails Thm 2.3 eq 2.11a (norm 106 ≠ 110) and 2.11b
  (`tools/canary_thm211b.py`). Signature and lengths are plausible; the content
  is not a solution.
- **How it was caught:** the 2.11b soundness canary (2026-07-15) flagged it; the
  independent NPAF check confirmed. It sat banked for ~3 months in violation of
  verify-before-claim — the champion predates `tools/verify_npaf.py` discipline.
- **Consequences:** BS(28,27) rows retracted from README.md and
  docs/kotsireas_brief.md; the SA ladder record now starts at n=29. Any canary
  or probe keyed on this n=27 "class" was chasing a non-solution. If the
  original "Kotsireas-verified" BS(28,27) sequences exist outside this repo
  (e-mail?), they can be re-banked after passing `tools/verify_npaf.py`.
- **Kept because:** `tools/canary_thm211b.py` needs a known-bad fixture, and the
  retraction needs its evidence preserved.

Nothing in this directory may be counted, cited, or used as a search target.
