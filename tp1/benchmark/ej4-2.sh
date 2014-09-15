#!/bin/bash

LOTE=$1

$BINDIR/simusched $LOTE 1 1 0 SchedRR  2 > "$NOMBRE-c-1-q-2.dat"
$BINDIR/simusched $LOTE 1 1 0 SchedRR  7 > "$NOMBRE-c-1-q-7.dat"
$BINDIR/simusched $LOTE 1 1 0 SchedRR 15 > "$NOMBRE-c-1-q-15.dat"

$BINDIR/simusched $LOTE 4 1 3 SchedRR  2  2  2  2 > "$NOMBRE-c-4-q-2.dat"
$BINDIR/simusched $LOTE 4 1 3 SchedRR  7  7  7  7 > "$NOMBRE-c-4-q-7.dat"
$BINDIR/simusched $LOTE 4 1 3 SchedRR 15 15 15 15 > "$NOMBRE-c-4-q-15.dat"
