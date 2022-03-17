#
# This Dockerfile can be used to build dots-cpp
#
# To build, call from top-level directory:
# docker build -f contrib/Dockerfile .
#
FROM ubuntu:20.04 AS dots-cpp-build-env

ARG DEBIAN_FRONTEND="noninteractive"
ENV TZ=Europe/Berlin

RUN apt-get update -y && apt-get install -y \
    cmake \
    g++ \
    libboost-dev \
    libboost-program-options-dev \
    ninja-build \
    python3-pip

RUN pip3 install \
    dots-code-generator


FROM dots-cpp-build-env AS dots-cpp-build

WORKDIR /dots-build

COPY ./ /dots-build

RUN ls -l

RUN cmake -G Ninja -B ./build/ && \
    cmake --build ./build/ && \
    cmake --install ./build/ --strip

