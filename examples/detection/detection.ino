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


void setup()
{
  Serial.begin(57600);
  uint8_t failed = dflash.init(SS);           // Preserve returned detection result

  uint8_t identity[4];                        // Chip identity bytes
  uint16_t pageAddress, blockNumber;

  if (failed != 0) {
    switch (failed) {
      case MANUFACTURER_NOT_DETECTED: Serial.println("Manufacturer not recognized!"); break;
      case FAMILY_NOT_DETECTED: Serial.println("Chip family not recognized!"); break;
      case DENSITY_NOT_DETECTED: Serial.println("Chip size not recognized!"); break;
      default: Serial.println("Chip not detected! Check your wiring!"); break;
    }
    return;
  }

  // Print the detected chips properties
  Serial.println();
  Serial.print("Number bits for selecting pages: "); Serial.println(dflash.df_page_addr_bits());
  Serial.print("Number of bytes per page: "); Serial.println(dflash.df_page_size());
  Serial.print("Number of bits to address byte in page: "); Serial.println(dflash.df_page_bits());
  Serial.println();

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
  Serial.println(dflash.bytesPerPage());
  Serial.print("Pages per block		");
  Serial.println(dflash.pagesPerBlock());
  Serial.print("Blocks per sector	");
  Serial.println(dflash.blocksPerSector());
  Serial.print("Sectors per chip	");
  Serial.println(dflash.sectorsPerChip());
  Serial.print("Bytes per chip		");
  Serial.println(dflash.bytesPerChip());

  // Show where a random page address is located
  randomSeed(analogRead(0));        // Use noise on input to generate seed
  pageAddress = random(dflash.bytesPerChip() / dflash.bytesPerPage());
  Serial.println();
  Serial.print("The page at address ");
  Serial.print(pageAddress);
  Serial.print(" is located in block number ");
  Serial.print(dflash.pageToBlockNumber(pageAddress));
  Serial.print(" and in sector number ");
  Serial.println(dflash.pageToSectorNumber(pageAddress));

  // Show where a random block number is located
  blockNumber = random((uint16_t)dflash.blocksPerSector() * dflash.sectorsPerChip());
  Serial.println();
  Serial.print("The block number ");
  Serial.print(blockNumber);
  Serial.print(" is located in sector number ");
  Serial.println(dflash.blockToSectorNumber(blockNumber));
  Serial.println();
}


void loop() {
}
