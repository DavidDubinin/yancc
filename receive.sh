#!/bin/bash
nc -l -q1 -p 1337 > yancc.zip
unzip yancc.zip
mkdir -p build/bin
rm yancc.zip
#make run
