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
 * "void readStrPage(uint16_t PageAdr,uint16_t addr, uint8_t *data, size_t size)" to the
 * similar "void readStrCont(uint16_t PageAdr,uint16_t addr, uint8_t *data, size_t size)".
 * The first function reads from the flash memory directly (leaving the buffer
 * untouched), but do not cross page boundaries. The last function do the same,
 * but cross page boundaries.
 */


#include <Arduino.h>
#include <SPI.h>
#include <Sodaq_dataflash.h>


void setup()
{
  Serial.begin(57600);
  uint8_t failed = dflash.init(SS);             // Preserve returned detection result

  const uint16_t pageadr = 0;                   // Address of page to read
  const uint16_t addr = 0;                      // Offset address of page to read
  const size_t size = 1000;                     // Must not exceed the available SRAM space!
  uint8_t data[size];                           // Received data
  
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


  // Method reading directly from the same flash memory page

  dflash.readStrPage(pageadr, addr, data, size);  
  Serial.print("Dumping ");
  Serial.print(size);
  Serial.print(" number of bytes directly from the same page from ");
  Serial.println(pageadr);
  for (size_t i = 0; i < size; i++) {
    if (data[i] <= 0xF) Serial.print("0");
    Serial.print(data[i], HEX); Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();


  // Method reading directly from flash memory across page boundaries

  dflash.readStrCont(pageadr, addr, data, size);  
  Serial.print("Dumping ");
  Serial.print(size);
  Serial.print(" number of bytes directly across pages from ");
  Serial.println(pageadr);
  for (size_t i = 0; i < size; i++) {
    if (data[i] <= 0xF) Serial.print("0");
    Serial.print(data[i], HEX); Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  Serial.println();
}


void loop() {
}
