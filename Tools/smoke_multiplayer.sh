#!/usr/bin/env bash
# Headless 2-client multiplayer smoke test.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GAME="$ROOT/R-Type/Game"
LOGDIR="${TMPDIR:-/tmp/opencode}/rtype-smoke"
mkdir -p "$LOGDIR"
rm -f "$LOGDIR"/*.log

if [ ! -x "$GAME" ]; then
    echo "Game binary not found: $GAME" >&2
    exit 1
fi

run_xvfb() {
    xvfb-run -a -s "-screen 0 1920x1080x24" "$@"
}

run_xvfb "$GAME" --server --headless >"$LOGDIR/server.log" 2>&1 &
SERVER_PID=$!
sleep 4

run_xvfb "$GAME" --client >"$LOGDIR/client1.log" 2>&1 &
CLIENT1_PID=$!
sleep 2

run_xvfb "$GAME" --client >"$LOGDIR/client2.log" 2>&1 &
CLIENT2_PID=$!

sleep 20

kill $CLIENT2_PID $CLIENT1_PID $SERVER_PID 2>/dev/null || true
wait $CLIENT2_PID $CLIENT1_PID $SERVER_PID 2>/dev/null || true

echo "=== server tail ==="; tail -5 "$LOGDIR/server.log"
echo "=== client1 tail ==="; tail -5 "$LOGDIR/client1.log"

grep -q "Spawned player" "$LOGDIR/server.log" || { echo "FAIL: no player spawn"; exit 1; }
echo "Smoke test passed (basic markers present)."
