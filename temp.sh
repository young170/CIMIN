#!/bin/bash

# compile main file "cimin.c"
set -x
gcc -o ./bin/cimin ./src/cimin.c -I./include

# build balance test case
set -x
pushd target_programs/balance
sh build.sh
popd

# build libxml2 test case
set -x
pushd target_programs/libxml2
sh build.sh
popd

# build jsmn test case
set -x
pushd target_programs/jsmn
sh build.sh
popd
