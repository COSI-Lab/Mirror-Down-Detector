FROM ubuntu:24.04 AS builder
RUN apt-get update && apt-get install -y yes g++ cmake libcurl4-openssl-dev libssl-dev ninja-build curl libopus0
COPY ./src /down-detector/src
COPY ./CMakeLists.txt /down-detector/CMakeLists.txt
RUN curl -o dpp.deb https://dl.dpp.dev/ && yes | dpkg -i dpp.deb && rm dpp.deb
WORKDIR /down-detector
RUN cmake -B build -G Ninja
RUN cmake --build build/

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y libcurl4 iputils-ping curl libopus0
RUN curl -o dpp.deb https://dl.dpp.dev/ && yes | dpkg -i dpp.deb && rm dpp.deb
COPY --from=builder /down-detector/build/discord-bot /down-detector/bin/discord-bot
ENTRYPOINT /down-detector/bin/discord-bot