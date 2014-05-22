# Make file for GB-Tree
#
# Copyright (c) 2014 Edward liu

CC = gcc
CXX = g++
TARGET = gbtree
OBJS = main.o Geohash.o GBTEngine.o GBTreeIndex.o GBTreeNode.o GBTTable.o GBTFile.o GeoQuery.o TestGeoQuery.o PathManager.o Distance.o
HDR = GBTreeBase.h Tools.h
VPATH = src/test:src/gbtree:src/storagemanager:src/pathmanager:src/path:src/base:src/util


ifeq ($(BUILD),debug)
# "Debug" build - no optimization, and debugging symbols
CFLAGS = -Wall -ggdb
CXXFLAGS = -Wall -ggdb
else
 # "Release" build - optimization, and no debug symbols
CFLAGS = -O2 -s -DNDEBUG
CXXFLAGS = -O2 -s -DNDEBUG
endif


all: $(TARGET)
$(TARGET): $(OBJS) $(HDR)
	$(CXX) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(TARGET) gbtree.exe *.o *~  

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<
