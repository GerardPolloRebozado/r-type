#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GAME="$ROOT/build/bin/openfront"
LOGDIR="${TMPDIR:-/tmp/opencode}/openfront-multiroom-smoke"
mkdir -p "$LOGDIR"
rm -f "$LOGDIR"/*.log
export XAUTHORITY="${TMPDIR:-/tmp/opencode}/xvfb.auth"
export DISPLAY=":99"
Xvfb :99 -screen 0 1920x1080x24 -auth "$XAUTHORITY" >/dev/null 2>&1 &
XVFB_PID=$!
sleep 1
COOKIE=$(xxd -l 16 -p /dev/urandom)
xauth -f "$XAUTHORITY" add :99 . "$COOKIE"
"$GAME" --server 4242 --rooms 2 --room-cap 1,3 --room-maps procedural,pangaea --room-name Solo,Duo --room-bots 2,4 >"$LOGDIR/server.log" 2>&1 &
SERVER_PID=$!
sleep 4
"$GAME" --join 127.0.0.1 4242 --name ClientSolo >"$LOGDIR/clientA.log" 2>&1 &
CLIENTA_PID=$!
sleep 8
grep -q "Match started in 'Solo'" "$LOGDIR/server.log"
grep -q "Match started!" "$LOGDIR/clientA.log"
kill "$CLIENTA_PID" "$SERVER_PID" "$XVFB_PID" 2>/dev/null || true
wait "$CLIENTA_PID" "$SERVER_PID" "$XVFB_PID" 2>/dev/null || true
echo "multiroom smoke passed"