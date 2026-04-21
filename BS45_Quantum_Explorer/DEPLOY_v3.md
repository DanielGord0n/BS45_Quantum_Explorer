# Deploy — BS(43), BS(28), BS(45) across 4 clusters

## Job allocation

| Target     | Fir            | Rorqual        | Nibi           | Trillium       | Total |
|------------|----------------|----------------|----------------|----------------|-------|
| BS(28) v3  | 10 tasks       | —              | —              | —              | 10    |
| BS(43) v2  | 20 tasks       | 20 tasks       | 20 tasks       | —              | 60    |
| BS(45) v3  | 50 tasks       | 50 tasks       | 50 tasks       | 50 tasks       | 200   |

**Seed-offset map (disjoint)**

| Target    | Fir       | Rorqual   | Nibi      | Trillium  |
|-----------|-----------|-----------|-----------|-----------|
| BS(28) v3 | 6000-6009 | —         | —         | —         |
| BS(43) v2 | 5000-5019 | 4000-4019 | 5200-5219 | —         |
| BS(45) v3 | 7000-7049 | 7100-7149 | 7200-7249 | 7300-7349 |

Each command below uses **one Duo push** (tar piped over a single SSH connection).

---

## Fir  —  BS(28) + BS(43) + BS(45)

```bash
cd ~/School/CP468/CP468-Assignments/CP468-Sarukhanian/BS45_Quantum_Explorer

tar -czf - src/solver/wz_sa_v2.cpp src/solver/wz_sa_v3.cpp \
    fir_bs28_v3_job.sh fir_bs43_v2_job.sh fir_bs45_v3_job.sh | \
ssh dangord@fir.alliancecan.ca \
  "mkdir -p \$SCRATCH/bs45/src/solver && \
   tar -xzf - -C \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch fir_bs28_v3_job.sh && \
   sbatch fir_bs43_v2_job.sh && \
   sbatch fir_bs45_v3_job.sh && \
   squeue -u dangord"
```

---

## Rorqual  —  BS(43) + BS(45)

```bash
tar -czf - src/solver/wz_sa_v2.cpp src/solver/wz_sa_v3.cpp \
    rorqual_bs43_v2_job.sh rorqual_bs45_v3_job.sh | \
ssh dangord@rorqual.alliancecan.ca \
  "mkdir -p \$SCRATCH/bs45/src/solver && \
   tar -xzf - -C \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch rorqual_bs43_v2_job.sh && \
   sbatch rorqual_bs45_v3_job.sh && \
   squeue -u dangord"
```

---

## Nibi  —  BS(43) + BS(45)

```bash
tar -czf - src/solver/wz_sa_v2.cpp src/solver/wz_sa_v3.cpp \
    nibi_bs43_v2_job.sh nibi_bs45_v3_job.sh | \
ssh dangord@nibi.alliancecan.ca \
  "mkdir -p \$SCRATCH/bs45/src/solver \$SCRATCH/tmp && \
   tar -xzf - -C \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch nibi_bs43_v2_job.sh && \
   sbatch nibi_bs45_v3_job.sh && \
   squeue -u dangord"
```

---

## Trillium  —  BS(45) only

```bash
tar -czf - src/solver/wz_sa_v3.cpp trillium_bs45_v3_job.sh | \
ssh dangord@trillium.scinet.utoronto.ca \
  "mkdir -p \$SCRATCH/bs45/src/solver && \
   tar -xzf - -C \$SCRATCH/bs45/ && \
   cd \$SCRATCH/bs45 && \
   sbatch trillium_bs45_v3_job.sh && \
   squeue -u dangord"
```

---

## Check progress

```bash
# Fir
ssh dangord@fir.alliancecan.ca \
  "cd \$SCRATCH/bs45 && ls -t bs28_v3_fir_output_*.txt bs43_v2_fir_output_*.txt bs45_v3_fir_output_*.txt 2>/dev/null | head -6 | xargs -I{} sh -c 'echo ===={}====; tail -10 {}'"

# Rorqual
ssh dangord@rorqual.alliancecan.ca \
  "cd \$SCRATCH/bs45 && ls -t bs43_v2_rorqual_output_*.txt bs45_v3_rorqual_output_*.txt 2>/dev/null | head -6 | xargs -I{} sh -c 'echo ===={}====; tail -10 {}'"

# Nibi
ssh dangord@nibi.alliancecan.ca \
  "cd \$SCRATCH/bs45 && ls -t bs43_v2_nibi_output_*.txt bs45_v3_nibi_output_*.txt 2>/dev/null | head -6 | xargs -I{} sh -c 'echo ===={}====; tail -10 {}'"

# Trillium
ssh dangord@trillium.scinet.utoronto.ca \
  "cd \$SCRATCH/bs45 && ls -t bs45_v3_trillium_output_*.txt 2>/dev/null | head -6 | xargs -I{} sh -c 'echo ===={}====; tail -10 {}'"
```

## Find a solution

```bash
# On any cluster — run after SSHing in
grep -rl "REPRODUCTION CONFIRMED\|WORLD RECORD" $SCRATCH/bs45/
```

## Expected timelines

- **BS(28,27)**: minutes per task on 192 cores. Should succeed within first hour.
- **BS(43,42)**: ~6 h per task historically. 60 attempts across 3 clusters — high confidence within 24 h.
- **BS(45,44)**: open problem. Watch champion cost descend in output files over days.
