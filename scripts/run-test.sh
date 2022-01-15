#!/bin/bash

rm -f data/* && make clean && make -j16
./bin/muonbase-server.app -d -v -c config/muonbase-config.json
./bin/muonbase-client.app -t
./scripts/kill-daemon.sh
rm -f data/* && make clean