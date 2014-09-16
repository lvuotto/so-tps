#!/bin/bash

LOTE=$1

for i in {1..1000}; do
  $BINDIR/simusched $LOTE 1 1 0 SchedLottery 5 $i > "$NOMBRE-q-5-s-$i.dat"
done
