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
 * This sketch compares the added function
 * "void writeBuf1ToPageWoE(uint16_t pageAddr)" to the
 * similar "void writeBuf1ToPage(uint16_t pageAddr)".
 * The second function erases the page before writing the buffer to it, which
 * has to be done. The first function allows you to take the responsibility for
 * the erase task. What happens if you forget it?
 */


#include <Arduino.h>
#include <SPI.h>
#include <Sodaq_dataflash.h>


// Prints stored data read from the chip
void printpage(uint16_t pageAddr, uint8_t *data, size_t size) {
  Serial.print("Dumping ");
  Serial.print(size);
  Serial.print(" number of bytes directly from page number ");
  Serial.println(pageAddr);
  for (size_t i = 0; i < size; i++) {
    if (data[i] <= 0xF) Serial.print("0");
    Serial.print(data[i], HEX); Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
}


void setup()
{
  Serial.begin(57600);
  uint8_t failed = dflash.init(SS);           // Preserve returned detection result

  const size_t size = dflash.df_page_size();
  uint8_t data[size];                         // Page data, must not exceed the available SRAM space!
  const uint16_t pageadr = 500;               // Address of page
  const uint16_t addr = 0;                    // Offset address of page
  size_t i;                                   // Loop-counter

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
  Serial.print("Number bits for selecting pages: "); Serial.println(dflash.df_page_addr_bits());
  Serial.print("Number of bytes per page: "); Serial.println(dflash.df_page_size());
  Serial.print("Number of bits to address byte in page: "); Serial.println(dflash.df_page_bits());
  Serial.println();


  // Method writing to buffer 1 - random data
  for (i = 0; i < size; i++) {
    data[i] = random(256);                    //Pseudorandom! Pattern will repeat.
  }
  dflash.writeStrBuf1(addr, data, size);

  // Method transfering buffer 1 to an erased page -random data
  dflash.writeBuf1ToPage(pageadr);

  // Method reading the page directly from flash, not touching buffer 1
  dflash.readStrPage(pageadr, addr, data, size);

  // Print the result
  Serial.println("Random data");
  Serial.println();
  printpage(pageadr, data, size);


  // Writing to buffer 1 - 50% ordered data
  for (i = 0; i < size; i = i + 2) {
    data[i] = i % 256;                        // Don't exceed 255
  }
  dflash.writeStrBuf1(addr, data, size);

  // Method transfering buffer 1 to a not erased page - 50% ordered data
  dflash.writeBuf1ToPageWoE(pageadr);

  // Method reading the page directly from flash, not touching buffer 1
  dflash.readStrPage(pageadr, addr, data, size);  

  // Print the result
  Serial.println("50% ordered data, page NOT erased!");
  Serial.println();
  printpage(pageadr, data, size);


  // Method transfering buffer 1 to an erased page - 50% ordered data
  dflash.writeBuf1ToPage(pageadr);

  // Method reading the page directly from flash again
  dflash.readStrPage(pageadr, addr, data, size);  

  // Print the result
  Serial.println("50% ordered data, page erased!");
  Serial.println();
  printpage(pageadr, data, size);
}


void loop() {
}
