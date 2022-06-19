#!/bin/sh
pip3 install tqdm
pip3 install regex
sudo apt install curl
sudo apt install -y libssl-dev
sudo apt install z3

sudo add-apt-repository ppa:sri-csl/formal-methods
sudo apt-get update
sudo apt-get install yices2