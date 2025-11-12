#!/bin/bash
make clean
zip -r yancc.zip *
nc -q1 141.57.6.161 1337 < yancc.zip
rm yancc.zip
#make run

