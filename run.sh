#!/bin/bash

cmake -H. -Bbuild
make -C build
./build/auto360cam