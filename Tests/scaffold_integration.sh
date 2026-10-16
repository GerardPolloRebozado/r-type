#!/usr/bin/env bash
# End-to-end scaffold CLI: file placement + CMake + compile.
# Run from repo root: bash Tests/scaffold_integration.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EDITOR="${ROOT}/build/RTypeEditor"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/rtype-scaffold-integration-XXXXXX")"
FAKE_REPO="${TMP}/fake-repo"
PROJECT="${FAKE_REPO}/test"
ENGINE="${FAKE_REPO}/Engine"

fail() {
  echo "FAIL: $*"
  exit 1
}

pass() {
  echo "OK: $*"
}

EDITOR_BUILD_FILE=""
cleanup() {
  if [[ -n "${EDITOR_BUILD_FILE}" && -f "${EDITOR_BUILD_FILE}" ]]; then
    rm -f "${EDITOR_BUILD_FILE}"
  fi
  rm -rf "$TMP"
}
trap cleanup EXIT

if [[ ! -x "$EDITOR" ]]; then
  fail "Build RTypeEditor first: cmake --build build --target RTypeEditor"
fi

echo "Integration temp dir: $TMP"
echo

mkdir -p "${ENGINE}/include/Systems" "${ENGINE}/include/Components" "${ENGINE}/include/Plugins"
mkdir -p "${ENGINE}/src/Systems" "${ENGINE}/src/Plugins"
cp "${ROOT}/Engine/CMakeLists.txt" "${ENGINE}/CMakeLists.txt"
cp "${ROOT}/CMakeLists.txt" "${FAKE_REPO}/CMakeLists.txt"

mkdir -p "${PROJECT}/assets"
cat > "${PROJECT}/test.scene" <<'EOF'
{"sceneName":"test","config":{"windowTitle":"test","windowWidth":1280,"windowHeight":720,"targetFPS":60,"m_ProjectFolder":"","m_BuildExecutableName":"test","m_BuildWindowTitle":"test"},"entities":{"sceneName":"test","entities":[]}}
EOF
echo '{}' > "${PROJECT}/input.json"

echo "== 1) create system from Gallery project cwd (Gerard case) =="
(
  cd "$PROJECT"
  "$EDITOR" create system IntegrationProbe
)
test -f "${PROJECT}/system/IntegrationProbeSystem.hpp"
test -f "${PROJECT}/system/IntegrationProbeSystem.cpp"
test -f "${PROJECT}/system/build.cmake"
grep -q 'IntegrationProbeSystem.cpp' "${PROJECT}/system/build.cmake"
pass "project/system/* and build.cmake"

echo "== 2) create system from Engine cwd =="
(
  cd "$ENGINE"
  "$EDITOR" create system EngineCwdProbe
)
test -f "${ENGINE}/include/Systems/EngineCwdProbeSystem.hpp"
test -f "${ENGINE}/src/Systems/EngineCwdProbeSystem.cpp"
grep -q 'EngineCwdProbeSystem.cpp' "${ENGINE}/CMakeLists.txt"
pass "Engine include/src + CMakeLists.txt"

echo "== 3) create system with explicit --path Engine =="
"$EDITOR" create system ExplicitPathProbe --path "$ENGINE"
test -f "${ENGINE}/include/Systems/ExplicitPathProbeSystem.hpp"
pass "--path Engine"

echo "== 4) create plugin from project cwd =="
(
  cd "$PROJECT"
  "$EDITOR" create plugin ProjectFeature
)
test -f "${PROJECT}/system/ProjectFeature.hpp"
test -f "${PROJECT}/system/ProjectFeaturePlugin.hpp"
grep -q 'ProjectFeaturePlugin.cpp' "${PROJECT}/system/build.cmake"
pass "project plugin under system/"

echo "== 5) compile scaffolded Engine systems (fake tree, EDITOR_BUILD off) =="
# Build only the fake Engine static lib by compiling the two new .cpp into a tiny check.
ENGINE_CXX=(c++ -std=c++23 -I "${ENGINE}/include" -I "${ROOT}/Engine/include")
for src in EngineCwdProbeSystem ExplicitPathProbeSystem; do
  "${ENGINE_CXX[@]}" -c "${ENGINE}/src/Systems/${src}.cpp" -o "${TMP}/${src}.o"
done
pass "engine scaffold sources compile"

echo "== 6) compile Gallery project executable (EDITOR_BUILD) =="
cat > "${PROJECT}/main.cpp" <<EOF
#include "Core/Application.hpp"
#include "rtype_scaffold_register.hpp"
#include "Plugins/PositionPlugin.hpp"

int main() {
    Engine::ApplicationConfig config;
    Engine::Application app(config);
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());
    RegisterScaffoldedSystems(app.GetRegistry());
    return 0;
}
EOF

EDITOR_BUILD_FILE="${ROOT}/.editor_build.cmake"

cat > "${EDITOR_BUILD_FILE}" <<EOF
include("${PROJECT}/system/build.cmake")
add_executable(test_game "${PROJECT}/main.cpp" \${RTYPE_PROJECT_SYSTEM_SOURCES})
target_include_directories(test_game PRIVATE "${PROJECT}/system")
target_link_libraries(test_game PRIVATE Engine Serialization)
EOF

cmake -B "${ROOT}/build" -S "${ROOT}" -DEDITOR_BUILD=ON >/dev/null
cmake --build "${ROOT}/build" --target test_game -j"$(sysctl -n hw.ncpu 2>/dev/null || nproc)"
test -f "${ROOT}/build/test_game"
pass "EDITOR_BUILD test_game links"

echo
echo "All scaffold integration checks passed."
