#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GAME="$ROOT/build/bin/openfront"
IP="${1:-127.0.0.1}"
PORT="${2:-4242}"
NAME="${3:-Commander}"
exec "$GAME" --join "$IP" "$PORT" --name "$NAME"