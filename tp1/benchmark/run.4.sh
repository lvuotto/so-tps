#!/bin/bash

for test in $BENCHDIR/ej4-*.sh; do
  nombre=$(basename $test)
  NOMBRE=${test%.sh} BINDIR=$BINDIR $test "$TESTDIR/lote.${nombre%.sh}.tsk"
done
