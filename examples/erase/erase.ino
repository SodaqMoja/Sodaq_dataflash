/*
 * Copyright (c) 2013 Kees Bakker.  All rights reserved.
 *
 * This file is part of Sodaq_dataflash.
 *
 * Sodaq_dataflash is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published bythe Free Software Foundation, either version 3 of
 * the License, or(at your option) any later version.
 *
 * Sodaq_dataflash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Sodaq_dataflash.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/*
 * Since the library now detects the chip by default, use
 * uint8_t df_page_addr_bits(), uint16_t df_page_size() and uint8_t df_page_bits(),
 * rather then
 * #define DF_PAGE_ADDR_BITS, #define DF_PAGE_SIZE and #define DF_PAGE_BITS!
 * Alternatively, disable detection with the init() function and #define chip!
 */

/*
 * This sketch demonstrates the different detection related functions.
 */


#include <Arduino.h>
#include <SPI.h>
#include <Sodaq_dataflash.h>


// Fill a range of pages with random data
void writePageRange(uint16_t startPage,uint16_t numPages, uint16_t pageSize)
{
  uint8_t data[pageSize];                     // Page data, must not exceed the available SRAM space!
  for (uint16_t i = 0; i < numPages; i++) {
    for (uint16_t j = 0; j < pageSize; j++) {
      data[j] = random(256);                  //Pseudorandom! Pattern will repeat.
    }
    // Method writing to buffer 1 - random data
    dflash.writeStrBuf1(0, data, pageSize);
    // Method transfering to buffer 1 -random data
    dflash.writeBuf1ToPage(startPage + i);
  }
}

// Read and print a range of pages
void readPageRange(uint16_t startPage,uint16_t numPages, uint16_t pageSize)
{
  uint8_t data[pageSize];                     // Page data, must not exceed the available SRAM space!
  for (uint16_t i = 0; i < numPages; i++) {
    // Method reading the page directly from flash, not touching buffer 1
    dflash.readStrPage(startPage + i, 0, data, pageSize);
    Serial.print("Dumping ");
    Serial.print(pageSize);
    Serial.print(" number of bytes directly from page number ");
    Serial.println(startPage + i);
    for (uint16_t j = 0; j < pageSize; j++) {
      if (data[j] <= 0xF) Serial.print("0");
      Serial.print(data[j], HEX); Serial.print(" ");
      if ((j + 1) % 8 == 0) Serial.println();
    }
    Serial.println();
  }
}


void setup()
{
  Serial.begin(57600);
  uint8_t failed = dflash.init(SS);           // Preserve returned detection result

  uint8_t identity[4];                        // Chip identity bytes
  uint16_t pageAddress;
  uint16_t pause = 10000;                      // Delay after print
  uint32_t totalBytes;
  uint16_t pageSize, pagesInBlock, blocksInSector, pagesInChip;
  uint8_t blockNumber, sectorNumber;

  if (failed != 0) {
    switch (failed) {
      case MANUFACTURER_NOT_DETECTED: Serial.println("Manufacturer not recognized!"); break;
      case FAMILY_NOT_DETECTED: Serial.println("Chip family not recognized!"); break;
      case DENSITY_NOT_DETECTED: Serial.println("Chip size not recognized!"); break;
      default: Serial.println("Chip not detected! Check your wiring!"); break;
    }
    return;
  }

  // Print chip type
  Serial.print("Chip			");
  dflash.readID(identity);
  switch (identity[1] & 0x0F) {
    case 0x02: Serial.println("AT45DB011D"); break;
    case 0x03: Serial.println("AT45DB021D"); break;
    case 0x04: Serial.println("AT45DB041D"); break;
    case 0x05: Serial.println("AT45DB081D"); break;
    case 0x06: Serial.println("AT45DB161D"); break;
    case 0x07: Serial.println("AT45DB321D"); break;
    case 0x08: Serial.println("AT45DB642D"); break;
  }

  // Print memory organization
  Serial.print("Bytes per page		");
  pageSize = dflash.bytesPerPage();           // Used later
  Serial.println(pageSize);
  Serial.print("Pages per block		");
  pagesInBlock = dflash.pagesPerBlock();      // Used later
  Serial.println(pagesInBlock);
  Serial.print("Blocks per sector	");
  blocksInSector = dflash.blocksPerSector();  // Used later
  Serial.println(blocksInSector);
  Serial.print("Sectors per chip	");
  Serial.println(dflash.sectorsPerChip());
  Serial.print("Bytes per chip		");
  totalBytes = dflash.bytesPerChip();         // Used later
  Serial.println(totalBytes);

  // Find a random page
  randomSeed(analogRead(0));                  // Use noise on input to generate seed
  pageAddress = random(totalBytes / pageSize);

  // Show where a random page address is located
  Serial.println();
  Serial.print("The page at address ");
  Serial.print(pageAddress);
  Serial.print(" is located in block number ");
  blockNumber = dflash.pageToBlockNumber(pageAddress);   // Used later
  Serial.print(blockNumber);
  Serial.print(" and in sector number ");
  sectorNumber = dflash.pageToSectorNumber(pageAddress); // Used later
  Serial.println(sectorNumber);
  Serial.println();


  // Fill the page with random data
  writePageRange(pageAddress, 1, pageSize);

  // Print the page
  Serial.print("Random data at page address ");
  Serial.println(pageAddress);
  readPageRange(pageAddress, 1, pageSize);
  delay(pause);

  // Erase the page
  dflash.pageErase(pageAddress);

  // Print the page again
  Serial.print("Erased data at page address ");
  Serial.println(pageAddress);
  readPageRange(pageAddress, 1, pageSize);
  delay(pause);


  // Fill the block with random data
  pageAddress = (uint16_t) blockNumber * pagesInBlock;
  writePageRange(pageAddress, pagesInBlock, pageSize);

  // Print the block
  Serial.print("Random data in block number ");
  Serial.println(blockNumber);
  readPageRange(pageAddress, pagesInBlock, pageSize);
  delay(pause);

  // Erase the block
  dflash.blockErase(pageAddress);

  // Print the block again
  Serial.print("Erased data in block number ");
  Serial.println(blockNumber);
  readPageRange(pageAddress, pagesInBlock, pageSize);
  delay(pause);


  // Fill the sector with random data
  pageAddress = (uint16_t) sectorNumber * blocksInSector * pagesInBlock;
  writePageRange(pageAddress, (uint16_t) blocksInSector * pagesInBlock, pageSize);

  // Print the sector
  Serial.print("Random data in sector number ");
  Serial.println(sectorNumber);
  readPageRange(pageAddress, (uint16_t) blocksInSector * pagesInBlock, pageSize);
  delay(pause);

  // Erase the sector
  dflash.sectorErase(pageAddress);

  // Print the sector again
  Serial.print("Erased data in sector number ");
  Serial.println(sectorNumber);
  readPageRange(pageAddress, (uint16_t) blocksInSector * pagesInBlock, pageSize);
  delay(pause);


  // Fill the entire chip with random data
  Serial.println("Fill the entire chip with random data");
  pagesInChip = totalBytes / pageSize;        // Used later
  writePageRange(0, pagesInChip, pageSize);

  // Print 10 random pages
  Serial.println("Random data from 10 random pages");
  for (uint8_t i=0; i < 10; i++) {
    readPageRange(random(pagesInChip), 1, pageSize);
  }
  delay(pause);

  /* 
   * From AT45DB321D and AT45DB642D datasheets:
   *
   * Errata - Chip Erase - Issue
   *
   * In a certain percentage of units, the Chip Erase feature may
   * not function correctly and may adversely affect device operation.
   * Therefore, it is recommended that the Chip Erase commands
   * (opcodes C7H, 94H, 80H, and 9AH) not be used.
   *
   * Workaround
   * Use Block Erase (opcode 50H) as an alternative. The Block Erase
   * function is not affected by the Chip Erase issue.
   */
  // Erase the chip (chipErase() takes care of the workaround for
  // AT45DB321D and AT45DB642D if detection is on).
  dflash.chipErase();

  // Print 10 random pages again
  Serial.println("Erased data from 10 random pages");
  for (uint8_t i=0; i < 10; i++) {
    readPageRange(random(pagesInChip), 1, pageSize);
  }
}


void loop() {
}
