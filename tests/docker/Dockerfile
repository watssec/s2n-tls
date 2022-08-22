FROM ubuntu:18.04

RUN apt-get update 
RUN apt-get update && DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata
RUN apt-get update && apt-get install -y curl libssl-dev z3 python3 python3-pip software-properties-common git-all

RUN add-apt-repository ppa:sri-csl/formal-methods
RUN apt-get update
RUN apt-get install -y yices2

# add pip3 dependencies

RUN pip3 install tqdm regex pydot

# install llvm and clang

RUN apt-get update && apt-get install -y llvm-3.9 llvm-3.9-dev llvm llvm-dev  clang-3.9 cmake

# install json support for c++

RUN git clone https://github.com/nlohmann/json.git && cd json && mkdir build && cd build && cmake .. && make install

VOLUME /project
