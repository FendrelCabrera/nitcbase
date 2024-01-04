#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>

int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  // StaticBuffer buffer;
  // OpenRelTable cache;

  unsigned char buffer[BLOCK_SIZE];
  for(int i = 0; i < 4; i++) {
    Disk::readBlock(buffer, i);
    for(int x: buffer)
      std::cout << x;
    std::cout << std::endl;
  }

  // return FrontendInterface::handleFrontend(argc, argv);
  return 0;
}