FROM ubuntu:18.04

RUN apt-get update 
RUN apt-get install -y \
	curl libssl-dev z3 python3 python3-pip software-properties-common

RUN add-apt-repository ppa:sri-csl/formal-methods
RUN apt-get update
RUN apt-get install -y yices2

# add pip3 dependencies

RUN pip3 install tqdm regex pydot

# install llvm and clang

RUN apt-get install -y llvm-3.9 clang-3.9

VOLUME /project
