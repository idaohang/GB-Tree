#!/bin/sh

rm -f data/middle.idx data/middle.tbl
rm -f data/beijing.idx data/beijing.tbl
rm -f data/small.idx data/small.tbl 
rm -f data/zj.idx data/zj.tbl

./gbtree 3 > answer.txt
