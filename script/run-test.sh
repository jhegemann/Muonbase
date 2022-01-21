#!/bin/bash

rm -f data/*
make clean
make -j16
./bin/muonbase-server -d -c config/muonbase-config.json
./bin/muonbase-client -t -o 4096 -c 2 -n 4
./script/kill-daemon.sh
rm -f data/*
make clean
