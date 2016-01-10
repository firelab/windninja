#!/bin/bash

# Helper script for full build
#
# Prerequesites: none

./apt.sh
./zlib.sh
./msmpi.sh
./foam.sh
./ninjafoam.sh
./zip.sh

