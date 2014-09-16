#!/bin/bash

LOTE=$1
for i in {1..10}; do
  $BINDIR/simusched $LOTE 1 1 0 SchedLottery  10 $i > "$NOMBRE-s-s-$i.dat"
done
