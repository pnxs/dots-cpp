#
# This Dockerfile can be used to build dots-cpp
#
from ubuntu:20.04

ARG DEBIAN_FRONTEND="noninteractive"
ENV TZ=Europe/Berlin

RUN apt-get update -y && apt-get install -y \
    wget \
    curl \
    cmake \
    gcc \
    g++ \
    git \
    libsqlite3-dev \
    libncurses-dev \
    libboost-all-dev \
    rapidjson-dev \
    ninja-build \
    python3 \
    pkg-config \
    python3-pip

