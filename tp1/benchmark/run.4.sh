#!/bin/bash

for test in $BENCHDIR/ej4.*.sh; do
  nombre=$(basename $test)
  $test $BINDIR "$TESTDIR/lote.${nombre%.sh}.tsk" | \
    $BENCHDIR/graphsched.py > \
    $IMGDIR/${nombre%.sh}.png
done
