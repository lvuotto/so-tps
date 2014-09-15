#!/bin/bash

LOTE=$1
$BINDIR/simusched $LOTE 1 1 0 SchedRR 5 > "$NOMBRE.dat"
