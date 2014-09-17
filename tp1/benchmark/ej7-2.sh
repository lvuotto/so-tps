#!/bin/bash

LOTE=$1

for q in 2 7 15; do
  resultado=$($BINDIR/simusched $LOTE 1 1 2 SchedRR $q | grep EXIT)
  tiempo=$(echo "$resultado" | tail -n 1 | sed 's/EXIT \(.*\) .* .*/\1/')
  cantidad=$(echo "$resultado" | wc -l)
  echo "1 núcleo, quantum $q -> $(echo "$cantidad/$tiempo" | bc -l)"
done

for q in 2 7 15; do
  resultado=$($BINDIR/simusched $LOTE 4 1 2 SchedRR $q $q $q $q | grep EXIT)
  tiempo=$(echo "$resultado" | tail -n 1 | sed 's/EXIT \(.*\) .* .*/\1/')
  cantidad=$(echo "$resultado" | wc -l)
  echo "4 núcleo, quantum $q -> $(echo "$cantidad/$tiempo" | bc -l)"
done
