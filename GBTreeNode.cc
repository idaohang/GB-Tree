/*
 * =====================================================================================
 *
 *       Filename:  GBTreeNode.cc
 *
 *    Description:  the leaf and non-leaf node of GB-Tree
 *
 *        Version:  1.0
 *        Created:  04/02/2014 08:42:50 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include "GBTreeNode.h"
/**
 * Move pointer to the beginning of the buffer;
 */

void GBTLeafNode::resetPtr() {
	buffer_ptr = (l_struct*) (buffer + sizeof(int));
}

GBTLeafNode::GBTLeafNode(bool duplicate) {
	duplicate_key = duplicate;
	memset(buffer, 0, sizeof(buffer));
	resetPtr();
}

/*
 * Read the content of the node from the page pid in the GBTFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] GBTFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTLeafNode::read(PageId pid, const GBTFile& pf)
{ 
	return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the GBTFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] GBTFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTLeafNode::write(PageId pid, GBTFile& pf)
{
	return pf.write(pid, buffer);
}

/*
 * Update total keys
 */
void GBTLeafNode::updateTotalKeys(int count) {
	memcpy(buffer, &count, sizeof(int));
}
		
/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int GBTLeafNode::getKeyCount()
{ 
	// read first four bytes;
	int count;
	memcpy(&count, buffer, sizeof(int));
	return count;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RT GBTLeafNode::insert(uint64_t key, const RecordId& rid)
{
	int total_keys = getKeyCount();

	if(total_keys == MAX_KEY_PER_NODE) {
		return RT_NODE_FULL;
	}

	resetPtr();
	int index = 0;

	while ((index < total_keys) && key > buffer_ptr->key) {
		++index;
		++buffer_ptr;
	}
	//if there is duplicate key, override the old one.
	if(!(this->duplicate_key) && (key == buffer_ptr->key))
	{
		buffer_ptr->rid = rid;
		buffer_ptr->key = key;
		return 0;
	}

	// shift all elements to the right if we don't insert at the end of the buffer
	if (index != total_keys) shift_r(index);

	// insert key & rid
	buffer_ptr->rid = rid;
	buffer_ptr->key = key;

	// update the total keys
	updateTotalKeys(total_keys + 1);

	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTLeafNode::insertAndSplit(uint64_t key, const RecordId& rid, 
                              GBTLeafNode& sibling, uint64_t& siblingKey)
{ 
	RT rc;
	bool duplicate = false;

	if (sibling.getKeyCount() != 0)
		return RT_INVALID_NODE;


	int total_keys = getKeyCount();

	// reset pointer
	resetPtr();

	// find the spot where the new key should be inserted
	int key_spot = 0;
	while (key_spot != total_keys && key > buffer_ptr->key) {
		++key_spot;
		++buffer_ptr;
	}
	//if there are duplicate keys. just override the old one.
	if((!duplicate_key) && (key == buffer_ptr->key))
		duplicate = true;

	// middle of the node
	resetPtr();
	int middle_spot = total_keys / 2;
	int insert_to_sibling = 0;
	if(this->duplicate_key){
		if((buffer_ptr->key) == ((buffer_ptr+total_keys-1)->key)){
			if(key < buffer_ptr->key){
				middle_spot = 0;
				insert_to_sibling = 0;
			}else if(key > buffer_ptr->key){
				middle_spot = total_keys;
				insert_to_sibling = 1;
			}else{
				return RT_DUPLICATE_FULL;
			}
		}
		else if((buffer_ptr+middle_spot)->key == (buffer_ptr+total_keys-1)->key){
			buffer_ptr += middle_spot;
			while(buffer_ptr->key == (buffer_ptr-1)->key){
				buffer_ptr--;
				middle_spot--;
			}
			if(key >= buffer_ptr->key)
				insert_to_sibling = 1;
		}
		else{
			buffer_ptr += middle_spot;
			while(buffer_ptr->key == (buffer_ptr+1)->key){
				buffer_ptr++;
				middle_spot++;
			}
			buffer_ptr++;
			middle_spot++;
			if(key > buffer_ptr->key)
				insert_to_sibling = 1;
		}
	}else{
		buffer_ptr += middle_spot;
		if (key_spot >= middle_spot) {
			insert_to_sibling = 1;
		}
	}

	// copy right half to sibling
	for (int i = middle_spot; i < total_keys; ++i, ++buffer_ptr) {
		if ((rc = sibling.insert(buffer_ptr->key, buffer_ptr->rid)) < 0) return rc;
	}

	// copy the next pointer if there exists one in the current Node
	if (getNextNodePtr() != 0) {
		sibling.setNextNodePtr(getNextNodePtr());
	}

	if (!insert_to_sibling) {
		updateTotalKeys(total_keys - 1); // make it non-full
		if ((rc = insert(key, rid)) < 0) return rc;
	} else {
		if ((rc = sibling.insert(key, rid)) < 0) return rc;
	}

	int newCount;
	if(duplicate && insert_to_sibling)
		newCount = total_keys - sibling.getKeyCount();
	else
		newCount = total_keys + 1 - sibling.getKeyCount();
	// update number of keys
	updateTotalKeys(newCount);

	uint64_t cur_key;
	RecordId cur_rid;
	if ((rc = readEntry( (newCount-1), cur_key, cur_rid)) < 0) return rc;

	siblingKey = cur_key;

#ifdef DEBUG
	printf("Current Node: ");
	printN();

	printf("\nNext Node:");
	sibling.printN();
#endif
	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTLeafNode::locate(uint64_t searchKey, int& eid)
{
	resetPtr();
	int total_keys = getKeyCount();
	int i;
	for (i = 0; i < total_keys; ++i, ++buffer_ptr) {
		if (buffer_ptr->key >= searchKey) {
			eid = i;
			break;
		}
	}

	if (i >= total_keys) {
		return RT_NO_SUCH_RECORD;
	}

	return 0;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTLeafNode::readEntry(int eid, uint64_t& key, RecordId& rid)
{ 
	int total_keys = getKeyCount();

	if (eid < 0 || eid >= total_keys)
		return RT_NO_SUCH_RECORD;

	resetPtr();
	buffer_ptr += eid;

	key = buffer_ptr->key;
	rid = buffer_ptr->rid;

	return 0;
}


/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId GBTLeafNode::getNextNodePtr()
{ 
	int next_pageid;
	/**
	 * @ not 2(because there is only one count size in leaf node)
	 * @ Author : edward liu
	 * @ Date : 3/19/2014
	 */
	memcpy(&next_pageid, buffer + (GBTFile::PAGE_SIZE - sizeof(int)), sizeof(int));

	return next_pageid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTLeafNode::setNextNodePtr(PageId pid)
{ 
	/**
	 * @ no 2(because there is only one count size in leaf node)
	 * @ Author : edward liu
	 * @ Date : 3/19/2014
	 */
	memcpy(buffer + (GBTFile::PAGE_SIZE - sizeof(int)), &pid, sizeof(int));

	return 0;
}

void GBTLeafNode::shift_r(int index){
	memmove(buffer_ptr + 1, buffer_ptr, (getKeyCount() - index) * SLOT_SIZE);
}


/////////// NON LEAF /////////////
/**
 * Move pointer to the beginning of the buffer;
 */

void GBTNonLeafNode::resetPtr() {
	buffer_ptr = (nl_struct*) (buffer + sizeof(int) * 2);
}

GBTNonLeafNode::GBTNonLeafNode() {
	memset(buffer, 0, sizeof(buffer));
	resetPtr();
}

void GBTNonLeafNode::updateTotalKeys(int count) {
	memcpy(buffer, &count, sizeof(int));
}

void GBTNonLeafNode::shift_r(int index){
	int size = (getKeyCount() - index) * SLOT_SIZE;
	memmove(buffer_ptr + 1, buffer_ptr, size);
}

/*
 * Read the content of the node from the page pid in the GBTFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] GBTFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTNonLeafNode::read(PageId pid, const GBTFile& pf)
{
	RT rc;
	if ((rc = pf.read(pid, buffer)) < 0) return rc;
	resetPtr();
	return 0;
}
    
/*
 * Write the content of the node to the page pid in the GBTFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] GBTFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTNonLeafNode::write(PageId pid, GBTFile& pf)
{
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int GBTNonLeafNode::getKeyCount()
{
	// read first four bytes;
	int count;
	memcpy(&count, buffer, sizeof(int));
	return count;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RT GBTNonLeafNode::insert(uint64_t key, PageId pid)
{
	int total_keys = getKeyCount();

	if(total_keys == MAX_KEY_PER_NODE) {
		return RT_NODE_FULL;
	}

	resetPtr();
	int index = 0;

	while ((index < total_keys) && key > buffer_ptr->key) {
		++index;
		++buffer_ptr;
	}

	// shift all elements to the right if we don't insert at the end of the buffer
	if (index != total_keys) shift_r(index);

	// insert key & pid
	buffer_ptr->key = key;
	buffer_ptr->pid = pid;

	// update the total keys
	updateTotalKeys(total_keys + 1);

#ifdef DEBUG
	int first_pid = 0;
	getLeftSiblingPid(first_pid);
#endif
	

	return 0;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTNonLeafNode::insertAndSplit(uint64_t key, PageId pid, GBTNonLeafNode& sibling, uint64_t& midKey)
{
	RT rc;
	
	if (sibling.getKeyCount() != 0)
		return RT_INVALID_NODE;

	int total_keys = getKeyCount();

	// reset pointer
	resetPtr();

	// find the spot where the new key should be inserted
	int key_spot = 0;
	while (key_spot != total_keys && key > buffer_ptr->key) {
		++key_spot;
		++buffer_ptr;
	}


	bool is_equal = (key_spot != total_keys && key == buffer_ptr->key);
	// middle of the node
	resetPtr();
	int middle_spot = total_keys / 2;
	buffer_ptr += middle_spot;
	int insert_to_sibling = -1;

	
	/**
	* @ if key == buffer_ptr->key , do not insert the key.
	* @ Author : edward liu
	* @ Date : 3/20/2014
	*/
	PageId firstPid;
	if(is_equal){
		firstPid = (buffer_ptr-1)->pid;
		midKey = (buffer_ptr-1)->key;
	} else if (key_spot == middle_spot){
		firstPid = pid;
		insert_to_sibling = 0;
		midKey = key;
	}else if(key_spot < middle_spot) {
		firstPid = (buffer_ptr-1)->pid;
		insert_to_sibling = 0; // insert new key to the current node
		midKey = (buffer_ptr-1)->key;
	} else {
		firstPid = (buffer_ptr-1)->pid;
		insert_to_sibling = 1;
		midKey = (buffer_ptr-1)->key;
	}

	/**
	 * @ convert 1 to 0(insert new key to the current node)
	 * @ Author : edward liu
	 * @ Date : 3/19/2014
	 */
	sibling.insertFirstPid(firstPid);
	// copy right half to sibling
	for (int i = middle_spot; i < total_keys; ++i, ++buffer_ptr) {
		sibling.insert(buffer_ptr->key, buffer_ptr->pid);
	}

	// insert new key or not?
	if (insert_to_sibling != -1) {
		if (insert_to_sibling == 0) {
			updateTotalKeys(total_keys - 1); // make it non full
			if ((rc = insert(key, pid)) < 0) return rc;
		} else {
			if ((rc = sibling.insert(key, pid)) < 0) return rc;
		}

		updateTotalKeys(total_keys + 1 - sibling.getKeyCount());
	} else {
		updateTotalKeys(total_keys - sibling.getKeyCount());
	}

#ifdef DEBUG
	printf("None Leaf Current Node: ");
	printNL();

	printf("\nNone Leaf Next Node:");
	sibling.printNL();
#endif
	return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTNonLeafNode::locateChildPtr(uint64_t searchKey, PageId& pid)
{
	resetPtr();
	int total_keys = getKeyCount();
	int index = 1;

	while (searchKey > buffer_ptr->key && (index < total_keys)) {
		++index;
		++buffer_ptr;
	}
	if(searchKey > buffer_ptr->key)
		pid = buffer_ptr->pid;  // >=  goes to the right
	else 
	{
		if(index == 1)
		{
			int tmp_pid;
			memcpy(&tmp_pid, (buffer+sizeof(int)), sizeof(PageId));
			pid = tmp_pid;
		}
		else
			pid = (--buffer_ptr)->pid; // <  goes to the left
	}

	return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RT GBTNonLeafNode::initializeRoot(PageId pid1, uint64_t key, PageId pid2)
{
	resetPtr();

	buffer_ptr->key = key;
	buffer_ptr->pid = pid2;
	memcpy((buffer+sizeof(int)), &pid1, sizeof(PageId));

	updateTotalKeys(1);

	return 0;
}

/**
 * Simple return the left sibling's pid of the current node
 */
RT GBTNonLeafNode::getLeftSiblingPid(int &pid) {
	int tmp_pid;
	memcpy(&tmp_pid, (buffer+sizeof(int)), sizeof(PageId));
	pid = tmp_pid;
	return 0;
}

/**
 * insert the fisrt Page id 
 * @param page's id
 */
RT GBTNonLeafNode::insertFirstPid(PageId id)
{
	memcpy((buffer+sizeof(int)), &id, sizeof(PageId));
	return 0;
}
void GBTLeafNode::printN() {
	printf("Leaf Node");
	resetPtr();
	int index = 0;
	int total_keys = getKeyCount();
	printf("\n");
	int total = 0;
	while (index < total_keys) {
		printf("%" PRIx64 ", ", buffer_ptr->key);
		++buffer_ptr;
		++index;
		++total;
	}
	printf("\n");
	printf("total: %d\n", total);
}
void GBTNonLeafNode::printNL()
{
	printf("None leaf Node");
	resetPtr();
	int index = 0;
	int total_keys = getKeyCount();
	printf("\n");
	int total = 0;
	while (index < total_keys) {
		printf("%" PRIx64 ", ", buffer_ptr->key);
		++buffer_ptr;
		++index;
		++total;
	}
	printf("\n");
	printf("total: %d\n", total);
}
