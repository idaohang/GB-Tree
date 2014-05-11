/*
 * =====================================================================================
 *
 *       Filename:  GGBTreeIndex.cc
 *
 *    Description:  gb-tree index class
 *
 *        Version:  1.0
 *        Created:  04/03/2014 10:41:50 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include "GBTreeIndex.h"

GBTreeIndex::GBTreeIndex(bool duplicate)
{
	this->duplicate_key = duplicate;
}
/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RT GBTreeIndex::open(const std::string& indexname, char mode)
{
	RT rc;
	if ((rc = pf.open(indexname, mode)) < 0) return rc;

	// get rootPid and treeHeight
	if ((rc = pf.read(0, treeInfo_buffer)) < 0) {
		if (pf.endPid() == 0) { //newly created
			memset(treeInfo_buffer, 0, sizeof(treeInfo_buffer));
		} else
			return rc;
	}
	memcpy(&rootPid, treeInfo_buffer, sizeof(rootPid));
	memcpy(&treeHeight, treeInfo_buffer + sizeof(rootPid), sizeof(treeHeight));

	if (!treeHeight)
		rootPid = -1;

	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RT GBTreeIndex::close()
{
    return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RT GBTreeIndex::insert(uint64_t key, const RecordId& rid)
{
	RT rc;
	GBTLeafNode leaf(duplicate_key);

	if (rootPid == -1) { // we have an empty tree
		if ((rc = leaf.insert(key, rid)) < 0) return rc;
		
		if ((rc = leaf.write(1, pf)) < 0) return rc;

		rootPid = 1;
		treeHeight = 1;

		// update tree info
		if ((rc = updateTreeInfo()) < 0) return rc;

	} else {
		if (treeHeight == 1) { // the root is also the leaf node
			leaf.read(1, pf);

			if (leaf.getKeyCount() == GBTLeafNode::MAX_KEY_PER_NODE) { // full node
				GBTLeafNode sibling(duplicate_key);
				uint64_t siblingKey;
				if ((rc = leaf.insertAndSplit(key, rid, sibling, siblingKey)) < 0) return rc;

				// update 2 nodes
				if ((rc = leaf.setNextNodePtr(2)) < 0) return rc;
				if ((rc = leaf.write(1, pf)) < 0)	return rc;
				if ((rc = sibling.write(2, pf)) < 0) return rc;

				// create a new root
				GBTNonLeafNode root;
				if ((rc = root.initializeRoot(1, siblingKey, 2)) < 0) return rc;
				if ((rc = root.write(3, pf)) < 0) return rc;

				rootPid = 3;

				treeHeight = 2;
				// update treeInfo
				if ((rc = updateTreeInfo()) < 0) return rc;

			} else {
				if ((rc = leaf.insert(key, rid)) < 0) return rc;
				if ((rc = leaf.write(1, pf)) < 0) return rc;
			}

		} else { // we have at least 2 levels
			uint64_t midKey;
			if((rc = insertHelper(-1, 1, rootPid, key, rid, midKey)) < 0)
				return rc;
		}
	}

    return 0;
}


RT GBTreeIndex::insertHelper(PageId parentNode, int currentLevel, PageId currentNode, const uint64_t &key, const RecordId &rid, uint64_t& midKey) {
	RT rc;

	if (currentLevel == treeHeight) { // leaf
		GBTLeafNode leaf(duplicate_key);
		if ((rc = leaf.read(currentNode, pf)) < 0) return rc;
		if (leaf.getKeyCount() == GBTLeafNode::MAX_KEY_PER_NODE) { // full node
			GBTLeafNode sibling(duplicate_key);
			if ((rc = leaf.insertAndSplit(key, rid, sibling, midKey)) < 0) return rc;

			// update 2 nodes
			if ((rc = leaf.setNextNodePtr(pf.endPid())) < 0) return rc;
			if ((rc = leaf.write(currentNode, pf)) < 0)	return rc;
			if ((rc = sibling.write(pf.endPid(), pf)) < 0) return rc;

			return 1; 

		} else {
			if ((rc = leaf.insert(key, rid)) < 0) return rc;
			if ((rc = leaf.write(currentNode, pf)) < 0) return rc;
			return 0;
		}
	} else {
		// read the content of the current node
		GBTNonLeafNode non_leaf;
		if ((rc = non_leaf.read(currentNode, pf)) < 0) return rc;
		
		// determine the child node
		PageId childNode = 0;
		if ((rc = non_leaf.locateChildPtr(key, childNode)) < 0) return rc;
		
	    /**
	     * 
	     * @ Author : edward liu
	     * @ Date : 3/19/2014
	     */
		if(childNode == 0) return 0;

		// follow this child node to the leaf
		if ((rc = insertHelper(currentNode, currentLevel+1, childNode, key, rid, midKey)) <= 0) return rc;

		// rc now contains the siblingKey. We have to determine if the current non-leaf node is full
		// to push the key to the upper level

		if (non_leaf.getKeyCount() == GBTNonLeafNode::MAX_KEY_PER_NODE) { // full
			uint64_t newMidKey;
			GBTNonLeafNode nf_sibling;
			if ((rc = non_leaf.insertAndSplit(midKey, pf.endPid()-1, nf_sibling, newMidKey)) < 0) return rc;

			// update non_leaf
			if ((rc = non_leaf.write(currentNode, pf)) < 0) return rc;

			// update new sibling
			int nf_sibling_pid = pf.endPid();
			if ((rc = nf_sibling.write(nf_sibling_pid, pf)) < 0) return rc;

			// are we at the root node?
			if (currentLevel == 1) {
				// create a new root
				GBTNonLeafNode newRoot;
				if ((rc = newRoot.initializeRoot(currentNode, newMidKey, nf_sibling_pid)) < 0) return rc;

				// write to disk
				PageId newRoot_pid = pf.endPid();
				if ((rc = newRoot.write(newRoot_pid, pf)) < 0) return rc;

				// update rootPid and treeHeight
				rootPid = newRoot_pid;
				++treeHeight;

				if ((rc = updateTreeInfo()) < 0) return rc;

				return 0;
			} else {
				midKey = newMidKey;
				return 1;
			}
		} else {
			if(currentLevel == (treeHeight-1))
			{
		    	GBTLeafNode child_leaf(duplicate_key);
		    	if ((rc = child_leaf.read(childNode, pf)) < 0) return rc;
		    	if ((rc = non_leaf.insert(midKey, child_leaf.getNextNodePtr())) < 0) return rc;
		    	// write
		    	if ((rc = non_leaf.write(currentNode, pf)) < 0) return rc;
			}
			else
			{
				if((rc = non_leaf.insert(midKey, pf.endPid() - 1)) < 0) return rc;
				//write
				if((rc = non_leaf.write(currentNode, pf)) < 0) return rc;
			}

			return 0;
		}
	}

	return 0;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RT GBTreeIndex::locate(uint64_t searchKey, IndexCursor& cursor)
{
	if (treeHeight == 0) {
		return RT_NO_SUCH_RECORD;
	}
	RT rc;
	int pid = rootPid; // read the root first
	if (treeHeight != 1) {
		GBTNonLeafNode nl_node;
		for (int current_level = 1; current_level < treeHeight; ++current_level) {
			if ((rc = nl_node.read(pid, pf)) < 0) return rc;
			if ((rc = nl_node.locateChildPtr(searchKey, pid)) < 0) return rc;
		}
	}

	// read the leaf
	GBTLeafNode l_node;
	if ((rc = l_node.read(pid, pf)) < 0) return rc;
	if ((rc = l_node.locate(searchKey, cursor.eid)) < 0) return rc;
	cursor.pid = pid;

	return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RT GBTreeIndex::readForward(IndexCursor& cursor, uint64_t& key, RecordId& rid)
{
	RT rc;
	GBTLeafNode l_node;

	if ((rc = l_node.read(cursor.pid, pf)) < 0) return rc;

	if (cursor.eid == l_node.getKeyCount()) { // move to the next sibling
			int next_pid = l_node.getNextNodePtr();
			if (next_pid == 0) { // no more sibling
				return RT_END_OF_TREE;
			} else {
				cursor.pid = next_pid;
				cursor.eid = 0;
				if ((rc = l_node.read(cursor.pid, pf)) < 0) return rc;
			}
	}


	l_node.readEntry(cursor.eid, key, rid);
	cursor.eid++;

	return 0;
}
RT GBTreeIndex::loadLeafNode(PageId pid, GBTLeafNode& node)
{
	RT rt;
	if ((rt = node.read(pid, pf)) < 0)
		return rt;
	return 0;
}


/**
* Write rootPid & treeHeight to pagePid = 0
*/
RT GBTreeIndex::updateTreeInfo() {
	// update rootPid and treeHeight
	memcpy(treeInfo_buffer, &rootPid, sizeof(rootPid));
	memcpy(treeInfo_buffer + sizeof(rootPid), &treeHeight, sizeof(treeHeight));
	return pf.write(0, treeInfo_buffer);
}

RT GBTreeIndex::pointToSmallestKey(IndexCursor& cursor) {
	if (treeHeight == 0) {
		return RT_NO_SUCH_RECORD;
	}
	RT rc;
	int pid = rootPid; // read the root first
	if (treeHeight != 1) {
		GBTNonLeafNode nl_node;
		for (int current_level = 1; current_level < treeHeight; ++current_level) {
			if ((rc = nl_node.read(pid, pf)) < 0) return rc;
			if ((rc = nl_node.getLeftSiblingPid(pid)) < 0) return rc;
		}
	}

	cursor.eid = 0;
	cursor.pid = pid;

	return 0;
}

int GBTreeIndex::getTotalCount() {
	RT rc;
	IndexCursor cursor;
	int total = 0;

	if ((rc = pointToSmallestKey(cursor)) < 0) return rc;

	PageId pid = cursor.pid;
	GBTLeafNode leaf;

	do {
		if ((rc = leaf.read(pid, pf)) < 0) return rc;
		total += leaf.getKeyCount();
	} while ((pid = leaf.getNextNodePtr()) != 0);

	return total;
}

int GBTreeIndex::getPageCount()
{
	RT rc;
	IndexCursor cursor;
	int total = 0;

	if ((rc = pointToSmallestKey(cursor)) < 0) return rc;

	PageId pid = cursor.pid;
	GBTLeafNode leaf;

	do {
		if ((rc = leaf.read(pid, pf)) < 0) return rc;
		total++;
	} while ((pid = leaf.getNextNodePtr()) != 0);

	return total;
}
//debug
PageId GBTreeIndex::getRootPid()
{
	return rootPid;
}
int GBTreeIndex::getTreeHeight()
{
	return treeHeight;
}
