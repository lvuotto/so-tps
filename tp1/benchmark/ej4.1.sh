#!/bin/bash

BINDIR=$1
LOTE=$2

$BINDIR/simusched $LOTE 1 1 0 SchedRR 5
