#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>

int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  StaticBuffer buffer;
  // OpenRelTable cache;

  // RecBuffer attrCatBuffer(ATTRCAT_BLOCK);
  // HeadInfo attrCatHeader;
  // attrCatBuffer.getHeader(&attrCatHeader);

  // RecBuffer attrCatBuffer2(attrCatHeader.rblock);
  // HeadInfo attrCatHeader2;
  // attrCatBuffer2.getHeader(&attrCatHeader2);

  // for(int j = 0; j < attrCatHeader.numEntries; j++) {
  //   Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  //   attrCatBuffer.getRecord(attrCatRecord, j);

  //   if(strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0) {
  //     if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0) {
  //       unsigned char buffer[BLOCK_SIZE];
  //       Disk::readBlock(buffer, ATTRCAT_BLOCK);
  //       int recordSize = ATTRCAT_NO_ATTRS*ATTR_SIZE;
  //       unsigned char *aNamePointer = buffer + HEADER_SIZE + attrCatHeader.numSlots +  recordSize*j + ATTRCAT_ATTR_NAME_INDEX*ATTR_SIZE;
  //       memcpy(aNamePointer, "Batch", ATTR_SIZE);
  //       Disk::writeBlock(buffer, ATTRCAT_BLOCK);
  //     }
  //   }
  // }
  // for(int j = 0; j < attrCatHeader2.numEntries; j++) {
  //   Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  //   attrCatBuffer2.getRecord(attrCatRecord, j);

  //   if(strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0) {
  //     if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0) {
  //       unsigned char buffer[BLOCK_SIZE];
  //       Disk::readBlock(buffer, attrCatHeader.rblock);
  //       int recordSize = ATTRCAT_NO_ATTRS*ATTR_SIZE;
  //       unsigned char *aNamePointer = buffer + HEADER_SIZE + attrCatHeader.numSlots +  recordSize*j + ATTRCAT_ATTR_NAME_INDEX*ATTR_SIZE;
  //       memcpy(aNamePointer, "Batch", ATTR_SIZE);
  //       Disk::writeBlock(buffer, attrCatHeader.rblock);
  //     }
  //   }
  // }


  // Printing details of records
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  RecBuffer attrCatBuffer2(attrCatHeader.rblock);
  HeadInfo attrCatHeader2;
  attrCatBuffer2.getHeader(&attrCatHeader2);

  for(int i = 0; i < relCatHeader.numEntries; i++) {
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBuffer.getRecord(relCatRecord, i);
    
    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    for(int j = 0; j < attrCatHeader.numEntries; j++) {
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord, j);

      if(strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM":"STR";
        printf(" %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    }
    for(int j = 0; j < attrCatHeader2.numEntries; j++) {
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer2.getRecord(attrCatRecord, j);

      if(strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM":"STR";
        printf(" %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    }

    printf("\n");
  }
  


  // return FrontendInterface::handleFrontend(argc, argv);
  return 0;
}