#!/bin/bash

run_type=$1

if [ "$run_type" = "build" ]; then
  cmake -H. -Bbuild
  make -C build
elif [ "$run_type" = "clean" ]; then
  rm -r build
elif [ "$run_type" = "rebuild" ]; then
  rm -r build
  cmake -H. -Bbuild
  make -C build
elif [ "$run_type" = "run" ]; then
  build/auto360cam
else
  cmake -H. -Bbuild
  make -C build
  build/auto360cam
fi
