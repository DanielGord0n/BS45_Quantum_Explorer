# v3 Deployment Commands — BS(28), BS(43), BS(45) across 4 clusters

## What's new
- `src/solver/wz_sa_v3.cpp` — general-purpose alternating SA over full ±1 manifold. Proven to solve BS(8,7) in 2 ms and BS(12,11) in ~50 s locally on 4 threads (v2's Wang-Zhu encoding could not).
- v2 (Wang-Zhu) kept for BS(43,42) reproduction — its encoding contains that specific solution.

## Seed-offset map (disjoint)
| Target     | Fir       | Rorqual   | Nibi      | Trillium  |
|------------|-----------|-----------|-----------|-----------|
| BS(43) v2  | 5000-5019 | 4000-4019 | 5200-5219 | 5300-5319 |
| BS(28) v3  | 6000-6009 | 6100-6109 | 6200-6209 | 6300-6309 |
| BS(45) v3  | 7000-7049 | 7100-7149 | 7200-7249 | 7300-7349 |

## Files to upload
Each cluster needs:
- `src/solver/wz_sa_v2.cpp`  (for BS(43))
- `src/solver/wz_sa_v3.cpp`  (for BS(28) and BS(45))
- The three cluster-specific `.sh` scripts

## Fir

```bash
cd ~/School/CP468/CP468-Assignments/CP468-Sarukhanian/BS45_Quantum_Explorer

scp src/solver/wz_sa_v2.cpp src/solver/wz_sa_v3.cpp \
    fir_bs43_v2_job.sh fir_bs28_v3_job.sh fir_bs45_v3_job.sh \
    dangord@fir.alliancecan.ca:~/

ssh dangord@fir.alliancecan.ca \
  "scancel -u dangord; \
   mkdir -p \$SCRATCH/bs45/src/solver && \
   cp ~/wz_sa_v2.cpp ~/wz_sa_v3.cpp \$SCRATCH/bs45/src/solver/ && \
   cp ~/fir_bs43_v2_job.sh ~/fir_bs28_v3_job.sh ~/fir_bs45_v3_job.sh \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch fir_bs28_v3_job.sh && \
   sbatch fir_bs43_v2_job.sh && \
   sbatch fir_bs45_v3_job.sh && \
   squeue -u dangord"
```

## Rorqual

```bash
scp src/solver/wz_sa_v2.cpp src/solver/wz_sa_v3.cpp \
    rorqual_bs43_v2_job.sh rorqual_bs28_v3_job.sh rorqual_bs45_v3_job.sh \
    dangord@rorqual.alliancecan.ca:~/

ssh dangord@rorqual.alliancecan.ca \
  "scancel -u dangord; \
   mkdir -p \$SCRATCH/bs45/src/solver && \
   cp ~/wz_sa_v2.cpp ~/wz_sa_v3.cpp \$SCRATCH/bs45/src/solver/ && \
   cp ~/rorqual_bs43_v2_job.sh ~/rorqual_bs28_v3_job.sh ~/rorqual_bs45_v3_job.sh \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch rorqual_bs28_v3_job.sh && \
   sbatch rorqual_bs43_v2_job.sh && \
   sbatch rorqual_bs45_v3_job.sh && \
   squeue -u dangord"
```

## Nibi

```bash
scp src/solver/wz_sa_v2.cpp src/solver/wz_sa_v3.cpp \
    nibi_bs43_v2_job.sh nibi_bs28_v3_job.sh nibi_bs45_v3_job.sh \
    dangord@nibi.alliancecan.ca:~/

ssh dangord@nibi.alliancecan.ca \
  "scancel -u dangord; \
   mkdir -p \$SCRATCH/bs45/src/solver \$SCRATCH/tmp && \
   cp ~/wz_sa_v2.cpp ~/wz_sa_v3.cpp \$SCRATCH/bs45/src/solver/ && \
   cp ~/nibi_bs43_v2_job.sh ~/nibi_bs28_v3_job.sh ~/nibi_bs45_v3_job.sh \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch nibi_bs28_v3_job.sh && \
   sbatch nibi_bs43_v2_job.sh && \
   sbatch nibi_bs45_v3_job.sh && \
   squeue -u dangord"
```

## Trillium

```bash
scp src/solver/wz_sa_v2.cpp src/solver/wz_sa_v3.cpp \
    trillium_bs43_v2_job.sh trillium_bs28_v3_job.sh trillium_bs45_v3_job.sh \
    dangord@trillium.scinet.utoronto.ca:~/

ssh dangord@trillium.scinet.utoronto.ca \
  "scancel -u dangord; \
   mkdir -p \$SCRATCH/bs45/src/solver && \
   cp ~/wz_sa_v2.cpp ~/wz_sa_v3.cpp \$SCRATCH/bs45/src/solver/ && \
   cp ~/trillium_bs43_v2_job.sh ~/trillium_bs28_v3_job.sh ~/trillium_bs45_v3_job.sh \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch trillium_bs28_v3_job.sh && \
   sbatch trillium_bs43_v2_job.sh && \
   sbatch trillium_bs45_v3_job.sh && \
   squeue -u dangord"
```

## Checking progress

```bash
# Fir
ssh dangord@fir.alliancecan.ca \
  "cd \$SCRATCH/bs45 && ls -t bs28_v3_fir_output_*.txt bs43_v2_fir_output_*.txt bs45_v3_fir_output_*.txt 2>/dev/null | head -5 | xargs -I{} sh -c 'echo ===={}====; tail -15 {}'"

# Rorqual
ssh dangord@rorqual.alliancecan.ca \
  "cd \$SCRATCH/bs45 && ls -t bs28_v3_rorqual_output_*.txt bs43_v2_rorqual_output_*.txt bs45_v3_rorqual_output_*.txt 2>/dev/null | head -5 | xargs -I{} sh -c 'echo ===={}====; tail -15 {}'"

# Nibi
ssh dangord@nibi.alliancecan.ca \
  "cd \$SCRATCH/bs45 && ls -t bs28_v3_nibi_output_*.txt bs43_v2_nibi_output_*.txt bs45_v3_nibi_output_*.txt 2>/dev/null | head -5 | xargs -I{} sh -c 'echo ===={}====; tail -15 {}'"

# Trillium
ssh dangord@trillium.scinet.utoronto.ca \
  "cd \$SCRATCH/bs45 && ls -t bs28_v3_trillium_output_*.txt bs43_v2_trillium_output_*.txt bs45_v3_trillium_output_*.txt 2>/dev/null | head -5 | xargs -I{} sh -c 'echo ===={}====; tail -15 {}'"
```

## Finding a successful result

On any cluster, search outputs for the success marker:

```bash
grep -l "REPRODUCTION CONFIRMED\|WORLD RECORD" $SCRATCH/bs45/*_output_*.txt
```

## Expected timelines (rough)
- **BS(28,27) v3**: should finish within **minutes** per task on 192 cores. 10 tasks × 4 clusters = 40 concurrent attempts. If none finds it within 30 min, there's a problem.
- **BS(43,42) v2**: historically ~6 h per task in `bs43_repro.log`. 20 tasks × 3 Alliance clusters = 60 attempts, high confidence of reproduction within 24 h.
- **BS(45,44) v3**: open problem. Monitor champion-cost trajectory — we want to see it descend below 8 across wall-clock time.
