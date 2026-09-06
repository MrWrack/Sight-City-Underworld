#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${VITASDK_IMAGE:-vitasdk/vitasdk:latest}"

docker run --rm \
  -v "$ROOT:/work" \
  -w /work \
  "$IMAGE" \
  bash -lc 'export VITASDK=/usr/local/vitasdk; ./scripts/build_vita.sh'
