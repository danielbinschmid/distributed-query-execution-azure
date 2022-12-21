#!/usr/bin/env bash
set -euo pipefail
cd files
[ -e top25 ] && rm -r top25
[ -e partitions ] && rm -r partitions
mkdir top25 
mkdir partitions
cd ..

N_WORKERS=10


cmake-build-debug/coordinator "./files/filelist.csv" "4242" &

# Spawn some workers
for (( c=1; c<=$N_WORKERS; c++ ))
do
  cmake-build-debug/worker "localhost" "4242" &
done


sleep 1
ELASTIC_WORKERS=6
# Spawn MORE workers
for (( c=1; c<=$ELASTIC_WORKERS; c++ ))
do
  cmake-build-debug/worker "localhost" "4242" &
done

echo "Spawned $ELASTIC_WORKERS more workers!"

# And wait for completion
time wait

# check if the results are correct
GROUND_TRUTH_FILE=ground-truth/result.csv
RESULT_FILE=result.csv

RESULT=$(diff $GROUND_TRUTH_FILE $RESULT_FILE)

if test -z "$RESULT"
then
  echo "Success!"
else
  echo "There is an error. The  results.csv file differs from the ground truth with:"
  echo "$RESULT"
fi
