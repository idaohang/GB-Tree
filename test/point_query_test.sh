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
POINT_QUERY=point
SEPERATOR=_

# test point query algorithm
echo -e "start testing point query algorithm..."
for NAME in small middle large xlarge xxlarge xxxlarge
do
	echo -e "data set: $NAME"
	FILE="$BENCHMARK_DIRECTORY$POINT_QUERY$SEPERATOR$NAME$DATA_FILE_EXTENSION"
	rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
	
	./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $POINT_QUERY > "$FILE"
	i=0
	while [ "$i" != "4" ]
	do
		rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
		./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $POINT_QUERY >> "$FILE"
		i=$(($i+1))
	done
	echo -e "end $NAME"
done
echo -e "testing point query algorithm ends"
