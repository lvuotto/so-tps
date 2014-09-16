#!/bin/bash

LOTE=$1
$BINDIR/simusched $LOTE 2 1 5 SchedRR  10 10 > "$NOMBRE-rr-c-2-m-5.dat"
$BINDIR/simusched $LOTE 2 1 5 SchedRR2 10 10 > "$NOMBRE-rr2-c-2-m-5.dat"
