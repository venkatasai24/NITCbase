#include "BlockBuffer.h"
#include<cstring>
#include<iostream>
// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
	// initialise this.blockNum with the argument
	this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
int BlockBuffer::getHeader(struct HeadInfo* head) {
	unsigned char* bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS) {
		return ret;   // return any errors that might have occured in the process
	}

	// populate the numEntries, numAttrs and numSlots fields in *head
	memcpy(&head->numSlots, bufferPtr + 24, 4);
	memcpy(&head->numEntries, bufferPtr + 16, 4);
	memcpy(&head->numAttrs, bufferPtr + 20, 4);
	memcpy(&head->rblock, bufferPtr + 12, 4);
	memcpy(&head->lblock, bufferPtr + 8, 4);

	return SUCCESS;
}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute* rec, int slotNum) {

	// loading the block at this->blockNum into buffer {used here, instead of Disk::readBlock()}
	unsigned char* bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS) {
		return ret;
	}

	struct HeadInfo head;

	// get the header using this.getHeader() function
	this->getHeader(&head);

	int attrCount = head.numAttrs;
	int slotCount = head.numSlots;

	/* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
	   - each record will have size attrCount * ATTR_SIZE
	   - slotMap will be of size slotCount
	*/
	int recordSize = attrCount * ATTR_SIZE;
	unsigned char* slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

	// load the record into the rec data structure
	memcpy(rec, slotPointer, recordSize);

	return SUCCESS;
}

/* NOTE: This function will NOT check if the block has been initialised as a
   record or an index block. It will copy whatever content is there in that
   disk block to the buffer.
   Also ensure that all the methods accessing and updating the block's data
   should call the loadBlockAndGetBufferPtr() function before the access or
   update is done. This is because the block might not be present in the
   buffer due to LRU buffer replacement. So, it will need to be bought back
   to the buffer before any operations can be done.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char** buffPtr) {
	/* check whether the block is already present in the buffer
	   using StaticBuffer.getBufferNum() */
	int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

	// if present (!=E_BLOCKNOTINBUFFER),
		// set the timestamp of the corresponding buffer to 0 and increment the
		// timestamps of all other occupied buffers in BufferMetaInfo.

	// else
		// get a free buffer using StaticBuffer.getFreeBuffer()

		// if the call returns E_OUTOFBOUND, return E_OUTOFBOUND here as
		// the blockNum is invalid

		// Read the block into the free buffer using readBlock()
	if (bufferNum != E_BLOCKNOTINBUFFER) {
		StaticBuffer::metainfo[bufferNum].timeStamp = 0;

		for (int i = 0; i < BUFFER_CAPACITY; i++) {
			if (i != bufferNum && !StaticBuffer::metainfo[i].free) {
				StaticBuffer::metainfo[i].timeStamp++;
			}
		}
	}
	else {
		bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
		if (bufferNum == E_OUTOFBOUND) {
			return E_OUTOFBOUND;
		}
		Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
	}

	// store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
	*buffPtr = StaticBuffer::blocks[bufferNum];

	// return SUCCESS;
	return SUCCESS;
}

/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char* slotMap) {
	unsigned char* bufferPtr;

	// get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if (ret != SUCCESS) {
		return ret;
	}

	struct HeadInfo head;
	// get the header of the block using getHeader() function
	this->getHeader(&head);

	int slotCount = head.numSlots;

	// get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
	unsigned char* slotMapInBuffer = bufferPtr + HEADER_SIZE;

	// copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
	memcpy(slotMap, slotMapInBuffer, slotCount);

	return SUCCESS;
}


int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

	double diff;

	// if attrType == STRING
	//     diff = strcmp(attr1.sval, attr2.sval)

	// else
	//     diff = attr1.nval - attr2.nval

	if (attrType == STRING) {
		diff = strcmp(attr1.sVal, attr2.sVal);
	}
	else {
		diff = attr1.nVal - attr2.nVal;
	}

	/*
	if diff > 0 then return 1
	if diff < 0 then return -1
	if diff = 0 then return 0
	*/

	if (diff > 0) {
		return 1;
	}
	else if (diff < 0) {
		return -1;
	}
	else {
		return 0;
	}

}

/* Sets the `slotNum`th record entry of the block with the input record contents.
  rec	- Pointer to the array of union Attribute elements from which the record entry is set.
  slotNum	- Slot number of the record in the block.
*/
int RecBuffer::setRecord(union Attribute* rec, int slotNum) {
	unsigned char* bufferPtr;
	/* get the starting address of the buffer containing the block
	   using loadBlockAndGetBufferPtr(&bufferPtr). */
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

	// if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
		// return the value returned by the call.
	if (ret != SUCCESS) {
		return ret;
	}

	/* get the header of the block using the getHeader() function */
	HeadInfo head;
	this->getHeader(&head);

	// get number of attributes in the block.
	int attrCount = head.numAttrs;

	// get the number of slots in the block.
	int slotCount = head.numSlots;

	// if input slotNum is not in the permitted range return E_OUTOFBOUND.
	if (slotNum >= slotCount || slotNum < 0) {
		return E_OUTOFBOUND;
	}

	/* offset bufferPtr to point to the beginning of the record at required
		slot. the block contains the header, the slotmap, followed by all
		the records. so, for example,
		record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
		copy the record from `rec` to buffer using memcpy
		(hint: a record will be of size ATTR_SIZE * numAttrs)
	*/
	int recordSize = attrCount * ATTR_SIZE;
	unsigned char* slotPointer = bufferPtr + HEADER_SIZE + slotCount + (slotNum * recordSize);

	memcpy(slotPointer, rec, recordSize);

	// update dirty bit using setDirtyBit()
	ret = StaticBuffer::setDirtyBit(this->blockNum);

	/* (the above function call should not fail since the block is already
		in buffer and the blockNum is valid. If the call does fail, there
		exists some other issue in the code) */

		// return SUCCESS

	return SUCCESS;
}