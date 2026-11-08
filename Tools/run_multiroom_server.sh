#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GAME="$ROOT/build/bin/openfront"
PORT="${1:-4242}"
ROOMS="${2:-3}"
CAPS="${3:-1,4,12}"
MAPS="${4:-procedural,pangaea,world}"
NAMES="${5:-Solo,Duo,Squad}"
BOTS="${6:-2,6,10}"
exec "$GAME" --server "$PORT" --rooms "$ROOMS" --room-cap "$CAPS" --room-maps "$MAPS" --room-name "$NAMES" --room-bots "$BOTS"