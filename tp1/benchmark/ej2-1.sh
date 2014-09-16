#!/bin/bash

LOTE=$1
$BINDIR/simusched $LOTE 1 1 0 SchedFCFS > "$NOMBRE-c-1.dat"
$BINDIR/simusched $LOTE 2 1 0 SchedFCFS > "$NOMBRE-c-2.dat"
$BINDIR/simusched $LOTE 3 1 0 SchedFCFS > "$NOMBRE-c-3.dat"
