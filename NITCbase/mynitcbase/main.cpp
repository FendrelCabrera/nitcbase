#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  RelCatEntry rce;
  AttrCatEntry ace;

  for(int i = 0; i < 2; i++) {
    RelCacheTable::getRelCatEntry(i, &rce);
    printf("Relation: %s\n", rce.relName);

    for(int j = 0; j < rce.numAttrs; j++) {
      AttrCacheTable::getAttrCatEntry(i, j, &ace);
      printf("  %s: %s\n", ace.attrName, (ace.attrType == NUMBER) ? "NUM" : "STR");
    }
    printf("\n");
  }

  return 0;
}