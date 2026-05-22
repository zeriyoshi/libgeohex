#!/bin/sh -e

cd "/workspaces/libgeohex"

build_and_test() {
    COMPILER="${1}"
    ASAN="${2}"
    UBSAN="${3}"
    MSAN="${4}"

    echo "Building with ${COMPILER} (ASAN=${ASAN}, UBSAN=${UBSAN}, MSAN=${MSAN})"
    rm -rf "build"
    mkdir -p "build" && cd "build" || exit 1
    cmake .. -DCMAKE_C_COMPILER="$(which "${COMPILER}")" -DUSE_ASAN="${ASAN}" -DUSE_UBSAN="${UBSAN}" -DUSE_MSAN="${MSAN}" -DBUILD_TESTS=true
    make -j"$(nproc)"
    make test
    cd .. || exit 1
    rm -rf "build"
}

should_run_sanitizers() {
    if test "x${RUN_SANITIZERS}" = "xtrue"; then
        return 0
    fi

    if test "x${RUN_SANITIZERS}" = "xfalse"; then
        return 1
    fi

    if test "x${NATIVE_PLATFORM}" = "x"; then
        return 0
    fi

    test "x${PLATFORM}" != "xlinux/s390x" && test "x${PLATFORM}" = "x${NATIVE_PLATFORM}"
}

build_and_test "clang" false false false
if should_run_sanitizers; then
  build_and_test "clang" true false false
  build_and_test "clang" false true false
  build_and_test "clang" false false true
fi

build_and_test "gcc" false false false
if should_run_sanitizers; then
  build_and_test "gcc" true false false
  build_and_test "gcc" false true false
fi

echo "All builds and tests completed successfully."
