# Astra follow-up (2026-09-22): shadow instrument for joint reachability; exact alpha/eta accounting

(Verbatim spec received from the external reviewer; implementation status tracked in
docs/n44_search_narrowing_research.md.)

## 1. Minimal shadow instrument (joint-reachability go/no-go from one lane)
- Sample whole candidates with p=1/4096, deterministic hash(seed, class, canonical_cell_key,
  direction, batch, sorted_index). Log build, node budget, WZ_FH_EARLY_CHECK. Production
  survivor lists, return values, node charges, checkpoints unchanged.
- In fh_abp_filter keep the capacity loop; for sampled candidates maintain a SHADOW survivor
  stack: shadow_root = original root witnesses; per call: run original unchanged, record
  old_in/old_out; if an ancestor was shadow-pruned -> suppressed; else if original output
  empty -> ordinary rejection; else capacity-filter shadow_parent then joint reachability ->
  shadow_child; first_cut = shadow_child.empty().
- Record per call: candidate_id, call_id, depth (d+1), old_in, old_out, shadow_in,
  shadow_box, shadow_out, ancestor_cut, first_cut, joint_cpu_ns, child_nodes, child_cpu_ns.
  Aggregate by depth in memory; flush at candidate boundaries/shutdown; no hot-loop prints.
- Third hook: at the recursive call in fh_ab_search, when first_cut, time the ORIGINAL
  child recursion (still executed) = removed-subtree work; suppress shadow testing below.
- Per sampled candidate: C_i baseline completion CPU, W_i CPU in union of first-cut child
  subtrees, J_i extra joint-filter CPU. s_hat = sum W / sum C; h_hat = sum J / sum C;
  f_hat = baseline completion CPU / total worker CPU. Screening estimate G_CPU = f(s-h).
  Ratios of sums, uncertainty by cell (candidates within a cell correlated). Capped
  candidates are censored (observed prefix only). Require shadow on/off identity on small
  fixtures and a no-op-predicate overhead measurement before production data.

## 2. Exact alpha accounting
Over a COMPLETED ownership unit: R = raw cells owned = max(0, min(N_raw, A*e) - A*k);
O = raw cells skipped as orbit dups; D = R - O retained cells completed; P = prof-dead;
E = retained non-dead cells that yielded zero eligible candidates (NEW counter cells_empty).
alpha_profile = (D-P)/D; alpha_front = (D-P-E)/D. Do not use D/(D+O). Counters must be
CUMULATIVE across reps (persist with the checkpoint cursor); range_done=178/178 proves
completion, not counter completeness. Pool as sum(L)/sum(D); count one direction.

## 3. Exact eta accounting
L = D-P-E; Q = sum over jobs of allocated nodes x actual allocated seconds / 86400 (lane-
days, from sacct elapsed, not requested limits); r0 = reference live cells per arm per 12 h.
eta_eff = L/(356 r0 Q); with the 2.2-3.4 reference: eta_eff in [L/(1210.4 Q), L/(783.2 Q)].
Simplest forecasting input: v_observed = L/Q; S_class ~= 35,925 alpha_front / v_observed
(class-representative rate, matching K/buffer/direction).
