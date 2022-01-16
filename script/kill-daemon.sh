#!/bin/bash

kill -9 $(ps aux | grep muonbase-server.app | grep -v grep | awk -F' ' {'print $2'})
