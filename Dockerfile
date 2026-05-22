ARG PLATFORM=${BUILDPLATFORM:-linux/amd64}

FROM --platform=${PLATFORM} debian:trixie

ARG PLATFORM

ENV PLATFORM="${PLATFORM}"

RUN apt-get update \
 && apt-get install -y "git" "ssh" "gcc" "clang" "cmake" "make" "lcov"
