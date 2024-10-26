FROM ubuntu:latest
WORKDIR /app
RUN apt-get update && apt-get install -y \
    g++ \
    cmake
RUN apt-get install -y vim
RUN apt-get install -y nano


