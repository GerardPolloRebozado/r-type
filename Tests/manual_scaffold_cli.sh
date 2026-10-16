#!/usr/bin/env bash
# Manual smoke test for RTypeEditor scaffold commands.
# Run from the repo root: bash Tests/manual_scaffold_cli.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EDITOR="${ROOT}/build/RTypeEditor"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/rtype-scaffold-XXXXXX")"

cleanup() {
  rm -rf "$TMP"
}
trap cleanup EXIT

if [[ ! -x "$EDITOR" ]]; then
  echo "Build the editor first: cmake --build build --target RTypeEditor"
  exit 1
fi

echo "Using temp dir: $TMP"
echo

echo "== create system (should succeed) =="
"$EDITOR" create system ManualCliProbe --path "${ROOT}/Engine"
grep -q "ManualCliProbeSystem.cpp" "${ROOT}/Engine/CMakeLists.txt"

echo "== duplicate system (should fail) =="
if "$EDITOR" create system ManualCliProbe --path "${ROOT}/Engine"; then
  echo "Expected failure for duplicate system"
  exit 1
fi

echo "== create plugin (should succeed) =="
"$EDITOR" create plugin ManualCliFeature --path "${ROOT}/Engine"
test -f "${ROOT}/Engine/include/Plugins/ManualCliFeaturePlugin.hpp"

echo "== new project (creates files; editor opens — close the window to continue) =="
PROJ="${TMP}/cli-new-project"
"$EDITOR" new "CLI Smoke" --path "$PROJ" --description "manual script"
test -f "$PROJ/test.scene"
test -d "$PROJ/assets"
test -f "$PROJ/input.json"

echo
echo "Manual CLI smoke finished."
echo "Remove generated engine files if you do not want to keep them:"
echo "  Engine/include/Systems/ManualCliProbeSystem.hpp"
echo "  Engine/src/Systems/ManualCliProbeSystem.cpp"
echo "  Engine/include/Components/ManualCliFeature.hpp"
echo "  Engine/include/Systems/ManualCliFeatureSystem.hpp"
echo "  Engine/src/Systems/ManualCliFeatureSystem.cpp"
echo "  Engine/include/Plugins/ManualCliFeaturePlugin.hpp"
echo "  Engine/src/Plugins/ManualCliFeaturePlugin.cpp"
echo "  and the matching lines in Engine/CMakeLists.txt"
