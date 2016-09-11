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
 * This scetch compares the added function
 * "void readStrPage(uint16_t PageAdr,uint16_t addr, uint8_t *data, size_t size)"
 * to the similar "void readPageToBuf1(uint16_t PageAdr)" and
 * "void readStrBuf1(uint16_t addr, uint8_t *data, size_t size)" functions.
 * The first function reads from the flash memory directly (leaving the buffer
 * untouched), while the two last functions do the same in two steps via the SRAM buffer 1.
 */


#include <Arduino.h>
#include <SPI.h>
#include <Sodaq_dataflash.h>


void setup()
{
  Serial.begin(57600);
  uint8_t failed = dflash.init(SS);          // Preserve returned detection result

  const uint16_t pageadr = 64;               // Address of page to read
  const uint16_t addr = 0;                   // Offset address of page to read
  const size_t size = dflash.df_page_size(); // Number of bytes to read
  uint8_t data[size];                        // Received data
  
  long int timer;

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


  // Method using extra step via buffer

  timer = micros(); // Disable serial printing for better timing
  dflash.readPageToBuf1(pageadr);
  dflash.readStrBuf1(addr, data, size);
  Serial.print("Dumping "); Serial.print(size); Serial.print(" number of bytes from page "); Serial.println(pageadr);
  for (size_t i = 0; i < size; i++) {
    if (data[i] <= 0xF) Serial.print("0");
    Serial.print(data[i], HEX); Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  timer = micros() - timer; // Disable serial printing for better timing
  Serial.println();
  Serial.print("Used "); Serial.print(timer); Serial.println(" us");Serial.println();


  // Method reading directly from flash memory

  timer = micros(); // Disable serial printing for better timing
  dflash.readStrPage(pageadr, addr, data, size);  
  Serial.print("Dumping "); Serial.print(size); Serial.print(" number of bytes directly from page "); Serial.println(pageadr);
  for (size_t i = 0; i < size; i++) {
    if (data[i] <= 0xF) Serial.print("0");
    Serial.print(data[i], HEX); Serial.print(" ");
    if ((i + 1) % 8 == 0) Serial.println();
  }
  timer = micros() - timer; // Disable serial printing for better timing
  Serial.println();
  Serial.print("Used "); Serial.print(timer); Serial.println(" us");Serial.println();
}


void loop() {
}
