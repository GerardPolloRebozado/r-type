#!/usr/bin/env bash
# Safe manual test: engine scaffolds use a fake Engine tree; project stays on disk for Gallery.
# Run from repo root: bash Tests/scaffold_cli_tmp.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EDITOR="${ROOT}/build/RTypeEditor"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/rtype-scaffold-safe-XXXXXX")"
FAKE_ROOT="${TMP}/source_root"
PROJ="${TMP}/my-game"

cleanup() {
  rm -rf "${FAKE_ROOT}"
}
trap cleanup EXIT

if [[ ! -x "$EDITOR" ]]; then
  echo "Build first: cmake --build build --target RTypeEditor"
  exit 1
fi

mkdir -p "${FAKE_ROOT}/Engine/include/Systems" "${FAKE_ROOT}/Engine/src/Systems"
mkdir -p "${FAKE_ROOT}/Engine/include/Components" "${FAKE_ROOT}/Engine/include/Plugins"
mkdir -p "${FAKE_ROOT}/Engine/src/Plugins"
cp "${ROOT}/Engine/CMakeLists.txt" "${FAKE_ROOT}/Engine/CMakeLists.txt"

export RTYPE_SOURCE_DIR="${FAKE_ROOT}"

echo "Temp dir: ${TMP}"
echo "Project will remain at: ${PROJ}"
echo "(Re-open RTypeEditor with no args to see it in the Gallery.)"
echo

echo "== create system (fake Engine tree) =="
"$EDITOR" create system TmpDamage
test -f "${FAKE_ROOT}/Engine/include/Systems/TmpDamageSystem.hpp"

echo "== create plugin (fake Engine tree) =="
"$EDITOR" create plugin TmpHealth
test -f "${FAKE_ROOT}/Engine/include/Plugins/TmpHealthPlugin.hpp"

echo "== new project (opens editor — close window to finish) =="
"$EDITOR" new "Tmp Game" --path "$PROJ" --description "tmp only"
test -f "$PROJ/test.scene"

echo
echo "Done. Project folder kept at: ${PROJ}"
echo "Gallery registry: ${HOME}/Library/Application Support/rtype/projects.json"
