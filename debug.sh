#!/bin/sh

rm -f data/beijing.idx data/beijing.tbl
rm -f data/small.tbl data/small.idx
rm -f data/zj.tbl data/zj.idx
rm -f data/hz.tbl data/hz.idx

gdb ./gbtree 
