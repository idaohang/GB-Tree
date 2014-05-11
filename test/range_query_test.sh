#!/bin/sh
# Program:
#   Test script for GB-Tree
# History:
# 2014/5/10  edward liu  v1.0

#define some variables
DATA_DIRECTORY=data/
BENCHMARK_DIRECTORY=benchmark/
INDEX_EXTENSION=.idx
TABLE_EXTENSION=.tbl
DATA_FILE_EXTENSION=.txt
RANGE_QUERY=range
SEPERATOR=_
i=0

# test range query algorithm
if [ "$1" = "1" ]; then
	echo -e "start testing range query algorithm in varying page size..."
	echo -e "data set: zj.txt"
	NAME=zj
	i=0
	FILE="$BENCHMARK_DIRECTORY$RANGE_QUERY$SEPERATOR$NAME$DATA_FILE_EXTENSION"
	rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
	
	./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $RANGE_QUERY >> "$FILE"
	while [ "$i" != "4" ]
	do
		rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
		./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $RANGE_QUERY >> "$FILE"
		i=$(($i+1))
	done
	echo -e "testing range query algorithm in varying page size end"
fi
