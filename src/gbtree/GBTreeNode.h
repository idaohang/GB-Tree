/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 4/2/2014
 */
#ifndef GBTREENODE_H_
#define GBTREENODE_H_

#include "../storagemanager/GBTFile.h"
#include "../include/GBTreeBase.h"
#include "GBTTable.h"

/**
 * GBTLeafNode: The class representing a B+tree leaf node.
 */
typedef struct {
	RecordId rid;
	uint64_t key;
} l_struct;

class GBTLeafNode {
  public:
	static const int SLOT_SIZE = sizeof(RecordId) + sizeof(uint64_t);
	// first 4 bytes store number of keys, last 4 bytes store pageid of the sibling
	static const int MAX_KEY_PER_NODE = (GBTFile::PAGE_SIZE - sizeof(int)*2) / SLOT_SIZE;

	// constructor
	GBTLeafNode(bool duplicate = false);

   /**
    * Insert the (key, rid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param rid[IN] the RecordId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RT insert(uint64_t key, const RecordId& rid);

   /**
    * Insert the (key, rid) pair to the node
    * and split the node half and half with sibling.
    * The first key of the sibling node is returned in siblingKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert.
    * @param rid[IN] the RecordId to insert.
    * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
    * @param siblingKey[OUT] the first key in the sibling node after split.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT insertAndSplit(uint64_t key, const RecordId& rid, GBTLeafNode& sibling, uint64_t& siblingKey);

   /**
    * Find the index entry whose key value is larger than or equal to searchKey
    * and output the eid (entry id) whose key value &gt;= searchKey.
    * Remember that keys inside a B+tree node are sorted.
    * @param searchKey[IN] the key to search for.
    * @param eid[OUT] the entry number that contains a key larger              
    *                 than or equalty to searchKey.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT locate(uint64_t searchKey, int& eid);

   /**
    * Read the (key, rid) pair from the eid entry.
    * @param eid[IN] the entry number to read the (key, rid) pair from
    * @param key[OUT] the key from the slot
    * @param rid[OUT] the RecordId from the slot
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT readEntry(int eid, uint64_t& key, RecordId& rid);


   /**
    * Return the pid of the next slibling node.
    * @return the PageId of the next sibling node 
    */
    PageId getNextNodePtr();


   /**
    * Set the next slibling node PageId.
    * @param pid[IN] the PageId of the next sibling node 
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT setNextNodePtr(PageId pid);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();

   /**
    * Update the number of keys in the node
    */
	void updateTotalKeys(int count);
 
   /**
    * Read the content of the node from the page pid in the GBTFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] GBTFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT read(PageId pid, const GBTFile& pf);
    
   /**
    * Write the content of the node to the page pid in the GBTFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] GBTFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT write(PageId pid, GBTFile& pf);


    // debug
    void printN();

  private:


   /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */
    char buffer[GBTFile::PAGE_SIZE];  // buffer contains 1024 characters (1KB), PAGE_SIZE=1024;

    /**
     * Shift all elements in buffer to the right beginning from where buffer_ptr points to
     */
    void shift_r(int index);

    /**
     * Move the pointer to the start of the buffer
     */
    void resetPtr();

    l_struct* buffer_ptr;
	
	/* *
	 * Whether support duplicate keys.
	 * */
	bool duplicate_key;
}; 


/**
 * GBTNonLeafNode: The class representing a B+tree nonleaf node.
 */

/** Some notes
 * 1. the first four bytes used to store # of keys
 * 2. the second four bytes used for the first pid
 */

typedef struct {
	uint64_t key;
	PageId pid;
} nl_struct;

class GBTNonLeafNode {
  public:
	GBTNonLeafNode();

	// Non-leaf node
	static const int SLOT_SIZE = sizeof(nl_struct);
	static const int MAX_KEY_PER_NODE = (GBTFile::PAGE_SIZE - sizeof(int) * 2) / SLOT_SIZE;

   /**
    * Insert a (key, pid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RT insert(uint64_t key, PageId pid);

   /**
    * Insert the (key, pid) pair to the node
    * and split the node half and half with sibling.
    * The sibling node MUST be empty when this function is called.
    * The middle key after the split is returned in midKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
    * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT insertAndSplit(uint64_t key, PageId pid, GBTNonLeafNode& sibling, uint64_t& midKey);

   /**
    * Given the searchKey, find the child-node pointer to follow and
    * output it in pid.
    * Remember that the keys inside a B+tree node are sorted.
    * @param searchKey[IN] the searchKey that is being looked up.
    * @param pid[OUT] the pointer to the child node to follow.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT locateChildPtr(uint64_t searchKey, PageId& pid);

   /**
    * Initialize the root node with (pid1, key, pid2).
    * @param pid1[IN] the first PageId to insert
    * @param key[IN] the key that should be inserted between the two PageIds
    * @param pid2[IN] the PageId to insert behind the key
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT initializeRoot(PageId pid1, uint64_t key, PageId pid2);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();
	void updateTotalKeys(int count);

   /**
    * Read the content of the node from the page pid in the GBTFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] GBTFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT read(PageId pid, const GBTFile& pf);
    
   /**
    * Write the content of the node to the page pid in the GBTFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] GBTFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RT write(PageId pid, GBTFile& pf);

   /**
    * Simple return the left sibling's pid of the current node
    */
    RT getLeftSiblingPid(int &pid);

	//debug
	void printNL();

  private:
   /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */
    char buffer[GBTFile::PAGE_SIZE];

    /*
     * Shift all elements in buffer to the right beginning from where buffer_ptr points to
     */
    void shift_r(int index);

    /**
     * insert the fisrt Page id 
     * @param page's id
     */
	RT insertFirstPid(PageId id);

    /**
     * Move the pointer to the start of the buffer
     */
    void resetPtr();

    nl_struct* buffer_ptr;

}; 


#endif
