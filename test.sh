#!/usr/bin/env bash
set -euo pipefail
cd files
rm -r top25
rm -r partitions
mkdir top25 
mkdir partitions
cd ..

N_WORKERS=25


cmake-build-debug/coordinator "./files/filelist.csv" "4244" &

# Spawn some workers
for (( c=1; c<=$N_WORKERS; c++ ))
do
  cmake-build-debug/worker "localhost" "4244" &
done


# And wait for completion
time wait
