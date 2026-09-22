#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build-tests"
CXX="${CXX:-g++}"

TEST_SOURCES=("${PROJECT_DIR}"/tests/*.cpp)
CORE_SOURCES=("${PROJECT_DIR}"/core/src/*.cpp)
SUPPORT_SOURCES=(
  "${PROJECT_DIR}/src/config.cpp"
  "${PROJECT_DIR}/src/sensor_simulation.cpp"
  "${PROJECT_DIR}/src/signal_store.cpp"
)

CXXFLAGS=(
  -std=c++11
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wconversion
  -Wsign-conversion
  -I"${PROJECT_DIR}/core/include"
  -I"${PROJECT_DIR}/include"
)

command -v "${CXX}" >/dev/null 2>&1 || {
  echo "ERROR: compiler not found: ${CXX}" >&2
  exit 1
}

mkdir -p "${BUILD_DIR}"

echo "=== [1/2] Compiling ${#TEST_SOURCES[@]} test executables ==="

TEST_EXECUTABLES=()
for test_source in "${TEST_SOURCES[@]}"; do
  test_name="$(basename "${test_source%.cpp}")"
  test_executable="${BUILD_DIR}/${test_name}"

  echo "Building ${test_name}"
  "${CXX}" \
    "${CXXFLAGS[@]}" \
    "${test_source}" \
    "${CORE_SOURCES[@]}" \
    "${SUPPORT_SOURCES[@]}" \
    -o "${test_executable}"

  TEST_EXECUTABLES+=("${test_executable}")
done

echo
echo "=== [2/2] Running tests ==="

for test_executable in "${TEST_EXECUTABLES[@]}"; do
  echo
  echo "--- $(basename "${test_executable}") ---"
  "${test_executable}"
done

echo
echo "=== All ${#TEST_EXECUTABLES[@]} test executables passed ==="
