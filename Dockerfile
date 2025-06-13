FROM ubuntu:latest
RUN apt-get update && apt-get install -y doxygen  make gcc g++ valgrind cmake lcov
