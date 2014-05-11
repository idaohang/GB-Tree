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
NEAREST_QUERY=nearest
SEPERATOR=_
i=0

# test range query algorithm
if [ "$1" = "1" ]; then
	echo -e "start testing knn query in different data size..."
	for NAME in small middle large xlarge xxlarge xxxlarge 
	do
		echo -e "data set: $NAME"
		i=0
		FILE="$BENCHMARK_DIRECTORY$NEAREST_QUERY$SEPERATOR$NAME$DATA_FILE_EXTENSION"
		rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
		
		./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $NEAREST_QUERY >> "$FILE"
		while [ "$i" != "4" ]
		do
			rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
			./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $NEAREST_QUERY >> "$FILE"
			i=$(($i+1))
		done	
	done
elif [ "$1" = "2" ]; then
	echo -e "start testing knn query in different count..."
	NAME=xxlarge
	TEST_TYPE=count
	echo -e "data set: $NAME"
	FILE="$BENCHMARK_DIRECTORY$NEAREST_QUERY$SEPERATOR$NAME$SEPERATOR$TEST_TYPE$DATA_FILE_EXTENSION"
	rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
		
	./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $NEAREST_QUERY >> "$FILE"
	while [ "$i" != "4" ]
	do
		rm -f "$DATA_DIRECTORY$NAME$INDEX_EXTENSION" "$DATA_DIRECTORY$NAME$TABLE_EXTENSION"
		./gbtree $NAME "$DATA_DIRECTORY$NAME$DATA_FILE_EXTENSION" $NEAREST_QUERY >> "$FILE"
		i=$(($i+1))
	done
fi
