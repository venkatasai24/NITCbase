#include "Schema.h"

#include <cmath>
#include <cstring>

int Schema::openRel(char relName[ATTR_SIZE]) {
	int ret = OpenRelTable::openRel(relName);

	// the OpenRelTable::openRel() function returns the rel-id if successful
	// a valid rel-id will be within the range 0 <= relId < MAX_OPEN and any
	// error codes will be negative
	if (ret >= 0) {
		return SUCCESS;
	}

	//otherwise it returns an error message
	return ret;
}

int Schema::closeRel(char relName[ATTR_SIZE]) {
	/* relation is relation catalog or attribute catalog */
	if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0) {
		return E_NOTPERMITTED;
	}

	// this function returns the rel-id of a relation if it is open or
	// E_RELNOTOPEN if it is not. we will implement this later.
	int relId = OpenRelTable::getRelId(relName);

	if (relId == E_RELNOTOPEN) {
		return E_RELNOTOPEN;
	}

	return OpenRelTable::closeRel(relId);
}

/* This method changes the relation name of specified relation to new name as specified in arguments. */
int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]) {
	// if the oldRelName or newRelName is either Relation Catalog or Attribute Catalog,
		// return E_NOTPERMITTED
		// (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
		// you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
	if (
		strcmp(newRelName, RELCAT_RELNAME) == 0 ||
		strcmp(newRelName, ATTRCAT_RELNAME) == 0 ||
		strcmp(oldRelName, RELCAT_RELNAME) == 0 ||
		strcmp(oldRelName, ATTRCAT_RELNAME) == 0
		) {
		return E_NOTPERMITTED;
	}

	// if the relation is open
	//    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
	//    return E_RELOPEN
	int retVal = OpenRelTable::getRelId(oldRelName);
	if (retVal != E_RELNOTOPEN) {
		return E_RELOPEN;
	}

	// retVal = BlockAccess::renameRelation(oldRelName, newRelName);
	retVal = BlockAccess::renameRelation(oldRelName, newRelName);
	// return retVal
	return retVal;
}

/* This method changes the name of an attribute/column present in
 a specified relation, to new name as specified in arguments. */
int Schema::renameAttr(char* relName, char* oldAttrName, char* newAttrName) {
	// if the relName is either Relation Catalog or Attribute Catalog,
		// return E_NOTPERMITTED
		// (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
		// you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
	if (
		strcmp(relName, RELCAT_RELNAME) == 0 ||
		strcmp(relName, ATTRCAT_RELNAME) == 0
		) {
		return E_NOTPERMITTED;
	}

	// if the relation is open
		//    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
		//    return E_RELOPEN
	int retVal = OpenRelTable::getRelId(relName);
	if (retVal == E_RELOPEN) {
		return E_RELOPEN;
	}

	// Call BlockAccess::renameAttribute with appropriate arguments.

	// return the value returned by the above renameAttribute() call
	return BlockAccess::renameAttribute(relName, oldAttrName, newAttrName);
}

/*  creates a new relation with the name, attribute/column list as specified in arguments.
	Verifying the maximum number of attributes in a relation is to be checked by the caller
	of this function (Frontend Interface) and is not handled by this function. */
int createRel(char relName[], int nAttrs, char attrs[][ATTR_SIZE], int attrtype[]) {

	// declare variable relNameAsAttribute of type Attribute
	// copy the relName into relNameAsAttribute.sVal

	// declare a variable targetRelId of type RecId

	// Reset the searchIndex using RelCacheTable::resetSearhIndex()
	// Search the relation catalog (relId given by the constant RELCAT_RELID)
	// for attribute value attribute "RelName" = relNameAsAttribute using
	// BlockAccess::linearSearch() with OP = EQ

	// if a relation with name `relName` already exists  ( linearSearch() does
	//                                                     not return {-1,-1} )
	//     return E_RELEXIST;

	// compare every pair of attributes of attrNames[] array
	// if any attribute names have same string value,
	//     return E_DUPLICATEATTR (i.e 2 attributes have same value)

	/* declare relCatRecord of type Attribute which will be used to store the
	   record corresponding to the new relation which will be inserted
	   into relation catalog */
	Attribute relCatRecord[RELCAT_NO_ATTRS];
	// fill relCatRecord fields as given below
	// offset RELCAT_REL_NAME_INDEX: relName
	// offset RELCAT_NO_ATTRIBUTES_INDEX: numOfAttributes
	// offset RELCAT_NO_RECORDS_INDEX: 0
	// offset RELCAT_FIRST_BLOCK_INDEX: -1
	// offset RELCAT_LAST_BLOCK_INDEX: -1
	// offset RELCAT_NO_SLOTS_PER_BLOCK_INDEX: floor((2016 / (16 * nAttrs + 1)))
	// (number of slots is calculated as specified in the physical layer docs)

	// retVal = BlockAccess::insert(RELCAT_RELID(=0), relCatRecord);
	// if BlockAccess::insert fails return retVal
	// (this call could fail if there is no more space in the relation catalog)

	// iterate through 0 to numOfAttributes - 1 :
	{
		/* declare Attribute attrCatRecord[6] to store the attribute catalog
		   record corresponding to i'th attribute of the argument passed*/
		   // (where i is the iterator of the loop)
		   // fill attrCatRecord fields as given below
		   // offset ATTRCAT_REL_NAME_INDEX: relName
		   // offset ATTRCAT_ATTR_NAME_INDEX: attrNames[i]
		   // offset ATTRCAT_ATTR_TYPE_INDEX: attrTypes[i]
		   // offset ATTRCAT_PRIMARY_FLAG_INDEX: -1
		   // offset ATTRCAT_ROOT_BLOCK_INDEX: -1
		   // offset ATTRCAT_OFFSET_INDEX: i

		   // retVal = BlockAccess::insert(ATTRCAT_RELID(=1), attrCatRecord);
		   /* if attribute catalog insert fails:
				delete the relation by calling deleteRel(targetrel) of schema layer
				return E_DISKFULL
				// (this is necessary because we had already created the
				//  relation catalog entry which needs to be removed)
		   */
	}

	// return SUCCESS
}

/* This method deletes the relation with name as specified in arguments. */
int Schema::deleteRel(char* relName) {
	// if the relation to delete is either Relation Catalog or Attribute Catalog,
	//     return E_NOTPERMITTED
		// (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
		// you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

	// get the rel-id using appropriate method of OpenRelTable class by
	// passing relation name as argument

	// if relation is opened in open relation table, return E_RELOPEN

	// Call BlockAccess::deleteRelation() with appropriate argument.

	// return the value returned by the above deleteRelation() call

	/* the only that should be returned from deleteRelation() is E_RELNOTEXIST.
	   The deleteRelation call may return E_OUTOFBOUND from the call to
	   loadBlockAndGetBufferPtr, but if your implementation so far has been
	   correct, it should not reach that point. That error could only occur
	   if the BlockBuffer was initialized with an invalid block number.
	*/
}