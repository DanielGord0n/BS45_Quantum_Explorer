# Deploy v8 — Phased CD-then-AB SA for BS(43,42)

v8 = restored phased SA solver from commit `2b7face` (the algorithm that
historically solved BS(43)) + per-thread `random_device` seeding +
`seed_offset` CLI argument.

Solves CD first with a relaxed cost (only penalize NPAF that AB cannot
compensate), then with CD fixed solves AB so `corr_AB[s] = -corr_CD[s]`.

## Seed-offset map

| Cluster   | Seed range  |
|-----------|-------------|
| Fir       | 12000–12009 |
| Rorqual   | 12100–12109 |
| Nibi      | 12200–12209 |
| Trillium  | 12300–12309 |

## Cancel current jobs and queue v8

Run each command in your terminal. Each is a single SSH session (one Duo
push per cluster).

### Fir
```bash
tar -czf - src/solver/wz_sa_v8.cpp fir_bs43_v8_job.sh | \
  ssh dangord@fir.alliancecan.ca "scancel -u dangord; tar -xzf - -C \$SCRATCH/bs45/ && cd \$SCRATCH/bs45 && sbatch fir_bs43_v8_job.sh && squeue -u dangord"
```

### Rorqual
```bash
tar -czf - src/solver/wz_sa_v8.cpp rorqual_bs43_v8_job.sh | \
  ssh dangord@rorqual.alliancecan.ca "scancel -u dangord; tar -xzf - -C \$SCRATCH/bs45/ && cd \$SCRATCH/bs45 && sbatch rorqual_bs43_v8_job.sh && squeue -u dangord"
```

### Nibi
```bash
tar -czf - src/solver/wz_sa_v8.cpp nibi_bs43_v8_job.sh | \
  ssh dangord@nibi.alliancecan.ca "scancel -u dangord; tar -xzf - -C \$SCRATCH/bs45/ && cd \$SCRATCH/bs45 && sbatch nibi_bs43_v8_job.sh && squeue -u dangord"
```

### Trillium
```bash
tar -czf - src/solver/wz_sa_v8.cpp trillium_bs43_v8_job.sh | \
  ssh dangord@trillium.alliancecan.ca "scancel -u dangord; tar -xzf - -C \$SCRATCH/bs45/ && cd \$SCRATCH/bs45 && sbatch trillium_bs43_v8_job.sh && squeue -u dangord"
```

## Monitor progress

```bash
./check_all.sh
```
