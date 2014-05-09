CC = gcc
CXX = g++
CFLAGS = -Wall -ggdb
CXXFLAGS = -Wall -ggdb
TARGET = gbtree
OBJS = main.o Geohash.o GBTEngine.o GBTreeIndex.o GBTreeNode.o GBTTable.o GBTFile.o GeoQuery.o TestGeoQuery.o PathManager.o
HDR = GBTreeBase.h
vpath %.cc src
vpath %.c src
vpath %.h src

all:	$(TARGET)
$(TARGET): $(OBJS) $(HDR)
	$(CXX) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(TARGET) gbtree.exe *.o *~  

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<
