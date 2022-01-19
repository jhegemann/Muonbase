#!/bin/bash

rm -f data/*
make clean
make -j16
./bin/muonbase-server.app -d -c config/muonbase-config.json
./bin/muonbase-client.app -t -o 4096 -c 2 -n 4
./script/kill-daemon.sh
rm -f data/*
make clean
