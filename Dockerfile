ARG PLATFORM=${BUILDPLATFORM:-linux/amd64}

FROM --platform=${PLATFORM} debian:bookworm

RUN apt-get update \
 && apt-get install -y "git" "ssh" "gcc" "clang" "cmake" "make" "valgrind" "lcov"
