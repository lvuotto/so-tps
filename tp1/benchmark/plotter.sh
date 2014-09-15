#!/bin/bash

for data in $BENCHDIR/*.dat; do
  img=$(basename $data)
  cat $data | $BENCHDIR/graphsched.py > "$IMGDIR/${img%.dat}.png"
done
