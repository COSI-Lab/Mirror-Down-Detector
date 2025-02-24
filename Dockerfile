FROM ubuntu:24.04 AS builder
RUN apt-get update && apt-get install -y \
    yes \
    git \
    g++ \
    cmake \
    libcurl4-openssl-dev \
    libssl-dev \
    ninja-build \
    curl \
    libopus0
WORKDIR /down-detector
COPY ./src src
COPY ./include include
COPY ./cmake cmake
COPY ./CMakeLists.txt CMakeLists.txt
RUN cmake -B build -G Ninja
RUN cmake --build build


FROM ubuntu:24.04 as runner
RUN apt-get update && apt-get install -y \
    libcurl4 \
    iputils-ping \
    curl \
    libopus0 
COPY --from=builder \
    /down-detector/build/src/down-detector \
    /down-detector/bin/down-detector
COPY --from=builder \
    /down-detector/build/_deps/dpp-build/library/CMakeFiles/libdpp.so \
    /lib/libdpp.so
COPY --from=builder \
    /down-detector/build/_deps/spdlog-build/libspdlog.so \
    /lib/libspdlog.so
RUN ldconfig
ENTRYPOINT /down-detector/bin/down-detector
