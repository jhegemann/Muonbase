#!/bin/bash

rm -f data/* && make clean && make -j16
./bin/muonbase-server.app -d -v -c config/muonbase-config.json
./bin/muonbase-client.app -t -o 128 -c 1
./script/kill-daemon.sh
rm -f data/* && make clean