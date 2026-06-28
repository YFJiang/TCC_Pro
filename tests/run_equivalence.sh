#!/usr/bin/env bash
set -euo pipefail

# Functional equivalence harness for TCC_Pro compiler
# Usage: tests/run_equivalence.sh [tcc_path]
#   tcc_path - path to TCC_Pro executable (default: ./build/TCC_Pro)

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
TCC="${1:-${PROJECT_DIR}/build/TCC_Pro}"
CASES_DIR="${SCRIPT_DIR}/cases"
EXPECTED_DIR="${SCRIPT_DIR}/expected"

TMPDIR=$(mktemp -d /tmp/tcc_harness.XXXXXX)
trap "rm -rf ${TMPDIR}" EXIT

PASS=0
FAIL=0
TOTAL=0

echo "=== TCC_Pro Equivalence Harness ==="
echo "TCC_Pro:  ${TCC}"
echo "Cases: ${CASES_DIR}"
echo ""

run_test() {
    local src="$1"
    local name=$(basename "$src" .c)
    local bin="${TMPDIR}/${name}"
    local out="${TMPDIR}/${name}.stdout"
    local exp_out="${EXPECTED_DIR}/${name}.stdout"
    local exp_exit_file="${EXPECTED_DIR}/${name}.exit"
    local exp_exit=0

    TOTAL=$((TOTAL + 1))

    if [ -f "${exp_exit_file}" ]; then
        exp_exit=$(cat "${exp_exit_file}")
    fi

    local check_stdout=1
    if [ ! -f "${exp_out}" ]; then
        check_stdout=0
    fi

    local compile_err="${TMPDIR}/${name}.compile_err"
    if ! "${TCC}" -o "${bin}" "${src}" 2> "${compile_err}"; then
        echo "[FAIL] ${name} - compilation failed"
        cat "${compile_err}" >&2
        FAIL=$((FAIL + 1))
        return 1
    fi

    local exit_code=0
    "${bin}" > "${out}" 2>&1 || exit_code=$?

    if [ "${exit_code}" -ne "${exp_exit}" ]; then
        echo "[FAIL] ${name} - exit code: expected ${exp_exit}, got ${exit_code}"
        FAIL=$((FAIL + 1))
        return 1
    fi

    if [ "${check_stdout}" -eq 1 ]; then
        if ! diff -q "${out}" "${exp_out}" > /dev/null 2>&1; then
            echo "[FAIL] ${name} - stdout mismatch"
            echo "  Expected (${exp_out}):"
            cat "${exp_out}"
            echo "  Got (${out}):"
            cat "${out}"
            FAIL=$((FAIL + 1))
            return 1
        fi
    fi

    echo "[PASS] ${name}"
    PASS=$((PASS + 1))
    return 0
}

for src in "${CASES_DIR}"/*.c; do
    if [ -f "${src}" ]; then
        run_test "${src}"
    fi
done

echo ""
echo "=== Results: ${PASS}/${TOTAL} passed, ${FAIL}/${TOTAL} failed ==="

if [ "${FAIL}" -gt 0 ]; then
    exit 1
fi
exit 0
