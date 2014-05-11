/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 4/2/2014
 */
#ifndef GBTREEINDEX_H_
#define GBTREEINDEX_H_

#include <string>
#include "GBTFile.h"
#include "GBTreeBase.h"
#include "GBTreeNode.h"
#include "GBTTable.h"
/**
 * The data structure to point to a particular entry at a b+tree leaf node.
 * An IndexCursor consists of pid (PageId of the leaf node) and 
 * eid (the location of the index entry inside the node).
 * IndexCursor is used for index lookup and traversal.
 */
typedef struct {
  // PageId of the index entry
  PageId  pid;  
  // The entry number inside the node
  int     eid;  
} IndexCursor;

/**
 * Implements a B-Tree index for bruinbase.
 * 
 */
class GBTreeIndex {
 public:

	/* *
	 * constructor for gbtree index.
	 * */
	 GBTreeIndex(bool duplicate = false);

  /**
   * Open the index file in read or write mode.
   * Under 'w' mode, the index file should be created if it does not exist.
   * @param indexname[IN] the name of the index file
   * @param mode[IN] 'r' for read, 'w' for write
   * @return error code. 0 if no error
   */
  RT open(const std::string& indexname, char mode);

  /**
   * Close the index file.
   * @return error code. 0 if no error
   */
  RT close();
    
  /**
   * Insert (key, RecordId) pair to the index.
   * @param key[IN] the key for the value inserted into the index
   * @param rid[IN] the RecordId for the record being inserted into the index
   * @return error code. 0 if no error
   */
  RT insert(uint64_t key, const RecordId& rid);

  /**
   * Find the leaf-node index entry whose key value is larger than or
   * equal to searchKey and output its location (i.e., the page id of the node
   * and the entry number in the node) as "IndexCursor."
   * IndexCursor consists of pid (page id of the node that contains the 
   * searchKey) and eid (the entry number inside the node)
   * to indicate the location of a particular index entry in the B+tree.
   * Note that, for range queries, we need to scan the B+tree leaf nodes.
   * For example, if the query is "key > 1000", we should scan the leaf
   * nodes starting with the key value 1000. For this reason,
   * this function returns the location of the leaf node entry
   * for a given searchKey, instead of returning the RecordId
   * associated with the searchKey.
   * Using the returned "IndexCursor", you will have to call readForward()
   * to retrieve the actual (key, rid) pair from the index.
   * @param key[IN] the key to find
   * @param cursor[OUT] the cursor pointing to the first index entry
   * with the key value
   * @return error code. 0 if no error.
   */
  RT locate(uint64_t searchKey, IndexCursor& cursor);

  /**
   * Read the (key, rid) pair at the location specified by the index cursor,
   * and move foward the cursor to the next entry.
   * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
   * @param key[OUT] the key stored at the index cursor location
   * @return error code. 0 if no error
   */
  RT readForward(IndexCursor& cursor, uint64_t& key, RecordId& rid);

  /* *
   * load the leaf node from pid
   * @param pid[IN] page id
   * @param node[OUT] leaf node 
   * */
  RT loadLeafNode(PageId pid, GBTLeafNode& node);

  
  /**
   * Go to the very left node. Make cursor points to this node
   */
  RT pointToSmallestKey(IndexCursor& cursor);

  /**
   * get total number of entries
   */
  int getTotalCount();
  
  /**
   * get total number of entries
   */
  int getPageCount();
  //debug
  PageId getRootPid();
  int getTreeHeight();

 private:
	 char* treeInfo_buffer[GBTFile::PAGE_SIZE]; // mainly used to store the pagePid = 0, which contains

	/**
	 * Helper function for insert() which supports recursive algorithm
	 * @param currentLevel used to detect if the current Node is a leaf
	 * @param currentNode start from this node to travel through the tree
	 * @param key[OUT] the key stored at the index cursor location
	 * @return rid[OUT] the RecordId stored at the index cursor location
	 */
	 RT insertHelper(PageId parentNode, int currentLevel, PageId currentNode, const uint64_t &key, const RecordId &rid, uint64_t& midKey);

	  /**
	  * Write rootPid & treeHeight to pagePid = 0
	  */
	  RT updateTreeInfo();
	/* *
	  * whether the b+ tree Index supports duplicate keys
	  * */
	 bool duplicate_key;

	 GBTFile pf;         /// the GBTFile used to store the actual b+tree in disk

	 // rootPid and treeHeight
	 PageId   rootPid;    /// the PageId of the root node
	 int      treeHeight; /// the height of the tree
	 /// Note that the content of the above two variables will be gone when
	 /// this class is destructed. Make sure to store the values of the two  
	 /// variables in disk, so that they can be reconstructed when the index
	 /// is opened again later.
};


#endif
