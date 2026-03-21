FROM ubuntu:22.04
RUN apt-get update && apt-get install -y doxygen  make gcc g++ valgrind cmake lcov git
