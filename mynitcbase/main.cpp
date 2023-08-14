#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

#include <cstring>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;

  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  for (int i = 0, noOfAttrs = 0; i < relCatHeader.numEntries; i++) {
 
    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    for (int j = 0; j < relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal; j++, noOfAttrs++) {

      // declare attrCatRecord and load the attribute catalog entry into it
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord, noOfAttrs);

      /* attribute catalog entry corresponds to the current relation */
      if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
        /* get the attribute name */
        printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
      if(noOfAttrs == attrCatHeader.numSlots - 1) {
        // re initializing the slot index
        noOfAttrs = -1;
        // updating the buffer block with next block;
        attrCatBuffer = RecBuffer(attrCatHeader.rblock);
        // updating the header
        attrCatBuffer.getHeader(&attrCatHeader);
      }
    }
    printf("\n");
  }
  return 0;
}