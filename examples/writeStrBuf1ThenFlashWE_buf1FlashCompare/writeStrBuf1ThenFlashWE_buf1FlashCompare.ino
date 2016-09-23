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
 * This sketch compares the added function
 * "void writeStrBuf1ThenFlashWE(uint16_t pageAddr, uint16_t addr, uint8_t *data, size_t size)"
 * to the similar "void writeStrBuf1(uint16_t addr, uint8_t *data, size_t size)" and
 * "void writeBuf1ToPage(uint16_t pageAddr)" functions.
 * The first function does what the two last functions do, in one operation.
 * Another new function "bool buf1FlashCompare(uint16_t pageAddr)" demonstrates how it can be
 * used and also checks that the first functions work correctly.
 */


#include <Arduino.h>
#include <SPI.h>
#include <Sodaq_dataflash.h>


void setup()
{
  Serial.begin(57600);
  uint8_t failed = dflash.init(SS);          // Preserve returned detection result

  uint8_t identity[4];                       // Chip identity bytes
  const uint16_t pageadr = 64;               // Address of page to be written
  const size_t size = dflash.df_page_size(); // Number of bytes to write
  uint8_t data[size];                        // Data to be trasmitted to the chip
  uint8_t bufferByte;                        // Byte read and written to buffer
  

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
  Serial.print("Chip                    ");
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
  Serial.print("Bytes per page          ");
  Serial.println(dflash.bytesPerPage());
  Serial.print("Pages per block         ");
  Serial.println(dflash.pagesPerBlock());
  Serial.print("Blocks per sector       ");
  Serial.println(dflash.blocksPerSector());
  Serial.print("Sectors per chip        ");
  Serial.println(dflash.sectorsPerChip());
  Serial.print("Bytes per chip          ");
  Serial.println(dflash.bytesPerChip());


  // Generate data to transmit to the chip
  randomSeed(analogRead(0));                  // Use noise on input to generate seed
  Serial.println();
  Serial.println("Generating random data ...");
  for (size_t i = 0; i < size; i++) {
    data[i] = random(256);                    // Pseudorandom! Pattern will repeat.
  }


  // Method writing to buffer 1 - random data
  Serial.println("Transmitting data to buffer ...");
  dflash.writeStrBuf1(0, data, size);

  // Method transfering buffer 1 to an erased page -random data
  Serial.println("Transfer buffer to flash ...");
  dflash.writeBuf1ToPage(pageadr);

  // Method comparing buffer 1 to written page in flash
  Serial.println("Comparing data in flash to buffer ...");
  if (dflash.buf1FlashCompare(pageadr)) {
    Serial.println("... data integrity comfirmed!");
  }
  else {
    Serial.println("... data are corrupt!!!");
  }
  Serial.println();


  // Method combining "writeStrBuf1()" and "writeBuf1ToPage()" above in one step
  // We reuse random generated data in string
  Serial.println("Transmitting the same data to buffer and transfer to flash in one operation ...");
  dflash.writeStrBuf1ThenFlashWE(pageadr, 0, data, size);

  // Method comparing buffer 1 to written page in flash
  Serial.println("Comparing data in flash to buffer ...");
  if (dflash.buf1FlashCompare(pageadr)) {
    Serial.println("... data integrity comfirmed!");
  }
  else {
    Serial.println("... data are corrupt!!!");
  }
  Serial.println();


  // Invert one bit in the first byte of the buffer

  // Method reading one byte from the buffer
  Serial.println("We will now change only one bit in the buffer");
  Serial.println("Reading ...");
  bufferByte = dflash.readByteBuf1(0);
  Serial.print("First byte of buffer:		");
  Serial.println(bufferByte, BIN);
  bufferByte = bufferByte ^ 0x01;             // Invert only one bit
  Serial.print("First byte of buffer after xor:	");
  Serial.println(bufferByte, BIN);

  // Method writing one byte to the buffer
  Serial.println("Writing ...");
  dflash.writeByteBuf1(0, bufferByte);

  // Comparing again
  Serial.println("Comparing data in flash to buffer ...");
  if (dflash.buf1FlashCompare(pageadr)) {
    Serial.println("... data integrity comfirmed!");
  }
  else {
    Serial.println("... data are corrupt!!!");
  }
  Serial.println();
}


void loop() {
}
