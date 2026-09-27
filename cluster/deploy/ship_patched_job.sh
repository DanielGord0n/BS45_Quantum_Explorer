#!/bin/bash
# ship_patched_job.sh <cluster> <scratch-subdir> <sbatch-script-in-repo> '<sbatch line>' [base-commit]
#
# Ships the CURRENT repo solver to an ISOLATED dir on a cluster as a sha-checked patch on top of
# the cluster's own deployed solver, plus one sbatch script, then submits. Never touches
# $SCRATCH/bs45. Refuses if the cluster's solver is not the expected base commit's file, if the
# patch does not reproduce HEAD's sha, or if the script's sha differs.
#   ./cluster/deploy/ship_patched_job.sh fir bs45_cdpilot cluster/deploy/cluster_cd_prune_pilot.sh \
#     'sbatch --account=rrg-ikotsire_cpu --export=ALL,PILOT_VAR=WZ_FH_HALL_FAST,PILOT_ON=1 cluster_cd_prune_pilot.sh'
# Rehearse locally: SCRATCH=<dir with bs45/src/solver/wz_match.cpp at the base commit> SBATCH=echo REHEARSE=1 ...
set -euo pipefail
cl=$1; sub=$2; scr=$3; sbline=$4; base=${5:-cbe3859}
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; REPO="$(cd "$DIR/../.." && pwd)"; cd "$REPO"
[ -z "$(git status --porcelain src/solver/wz_match.cpp "$scr")" ] || { echo "commit src/solver/wz_match.cpp and $scr first" >&2; exit 2; }
S1=$(git show "$base:src/solver/wz_match.cpp" | shasum -a 256 | cut -c1-64)
S3=$(shasum -a 256 src/solver/wz_match.cpp | cut -c1-64); S4=$(shasum -a 256 "$scr" | cut -c1-64)
PB=$(git diff "$base" HEAD -- src/solver/wz_match.cpp | base64 | tr -d '\n'); SB=$(base64 < "$scr" | tr -d '\n'); B=$(basename "$scr")
REMOTE="cd \$SCRATCH && mkdir -p $sub/src/solver && cd $sub && X=\$(sha256sum \$SCRATCH/bs45/src/solver/wz_match.cpp | cut -c1-64) && echo \"cluster solver=\$X\" && [ \"\$X\" = $S1 ] && cp \$SCRATCH/bs45/src/solver/wz_match.cpp src/solver/wz_match.cpp && echo $PB | base64 -d | patch -p1 --no-backup-if-mismatch -s && Y=\$(sha256sum src/solver/wz_match.cpp | cut -c1-64) && echo \"patched=\$Y\" && [ \"\$Y\" = $S3 ] && echo $SB | base64 -d > $B && Z=\$(sha256sum $B | cut -c1-64) && [ \"\$Z\" = $S4 ] && echo SHA_OK && \${SBATCH:-$sbline}"
if [ "${REHEARSE:-0}" = 1 ]; then mkdir -p /tmp/ship_fakebin; printf '#!/bin/sh\nexec shasum -a 256 "$@"\n' > /tmp/ship_fakebin/sha256sum; chmod +x /tmp/ship_fakebin/sha256sum; PATH=/tmp/ship_fakebin:$PATH bash -c "$REMOTE"; exit; fi
exec caffeinate -i "$DIR/duo_run.sh" "$cl" "$REMOTE"
