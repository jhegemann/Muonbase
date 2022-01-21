#!/bin/bash

kill -9 $(ps aux | grep muonbase-server | grep -v grep | awk -F' ' {'print $2'})
