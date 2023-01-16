/* 
  Logger.h - esp32 library that saves data in SPIFFS

  Copyright (c) 2023 Jahir Abbas Mullick. All rights reserved.


  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#ifndef SPIFFS_LOGGER_H
#define SPIFFS_LOGGER_H

#include <stdint.h>
#include "FS.h"
#include "SPIFFS.h"

#define FS_PAGE_SIZE 128
#define NUM_PAGES 256
#define RING_BUFFER_SIZE (NUM_PAGES * FS_PAGE_SIZE)

// Uncomment to enable printing out nice debug messages.
#define LOGGER_DEBUG
// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

#if (RING_BUFFER_SIZE & (RING_BUFFER_SIZE - 1)) != 0
#error "RING_BUFFER_SIZE must be a power of two"
#endif

#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1)

// Setup debug printing macros.
#ifdef LOGGER_DEBUG
#define DEBUG_PRINT(...)          \
  {                               \
    DEBUG_PRINTER.print(__VA_ARGS__); \
  }
#define DEBUG_PRINTLN(...)          \
  {                                 \
    DEBUG_PRINTER.println(__VA_ARGS__); \
  }
#else
#define DEBUG_PRINT(...) \
  {                      \
  }
#define DEBUG_PRINTLN(...) \
  {                        \
  }
#endif

class Logger
{
private:
  File file_buffer;                         ///!> Pointer to the file used as the circular buffer
  long head_index;                          ///!> Head identification index
  long tail_index;                          ///!> Tail identification index
  long file_size;                           ///!> Total size allocated to the file circular buffer
  long pos_free;                            ///!> Free space available for writing
  void store();                             ///!> Saves Structure Header Information to File
  void recall();                            ///!> Loads File Header Information into Structure
  inline long jump_header() { return 64; }; ///!> Returns byte size of the header

public:
  Logger(const char file_name);
  Logger(const char *file_name, long len);
  ~Logger();
  /**
   * @brief   Adds a byte to the ring buffer.
   * @param   data The byte to place.
   * @retval  true if success.
   * @retval  false otherwise.
   */
  bool enqueue(uint8_t data);
  /**
   * @brief   Adds an array of bytes to the ring buffer.
   * @param   data A pointer to the array of bytes to place in the queue.
   * @param   len The size of the array.
   * @retval  true if success.
   * @retval  false otherwise.
   */
  bool enqueue(const uint8_t *data, long len);
  /**
   * @brief   Returns the oldest byte in the ring buffer.
   * @param   data A pointer to the location at which the data should be placed.
   * @retval  true if data was returned;
   * @retval  false otherwise.
   */
  bool dequeue(uint8_t *data);

  /**
   * @brief   Returns the <em>len</em> oldest bytes in the ring buffer.
   * @param   data  A pointer to the array at which the data should be placed.
   * @param   len   The maximum number of bytes to return.
   * @retval  true  if data was returned;
   * @retval  false Otherwise.
   */
  bool dequeue(uint8_t *data, long len);

  /**
   * @brief   Peeks the ring buffer, i.e. returns an element without removing it.
   * @param   data  A pointer to the location at which the data should be placed.
   * @param   index The index to peek.
   * @retval  true  if data was returned;
   * @retval  false Otherwise.
   */
  bool peek(uint8_t *data, long index);

  /**
   * @brief   Returns if buffer is empty.
   * @retval  true  if empty;
   * @retval  false otherwise.
   */
  inline bool is_empty()
  {
    return (this->head_index == this->tail_index);
  }

  /**
   * @brief   Returns if buffer is full.
   * @retval  true  if full;
   * @retval  false otherwise.
   */
  inline bool is_full()
  {
    return (this->pos_free == 0);
  }

  /**
   * @brief   Returns the number of items in the ring buffer.
   * @return  The number of items in the ring buffer.
   */
  inline long num_items()
  {
    return (this->file_size - this->pos_free - this->jump_header());
  }

  /**
   * @brief   Returns the number of available bytes in the ring buffer.
   * @return  The number of available bytes in the ring buffer.
   */
  inline long free_space()
  {
    return this->pos_free;
  }
};

#endif
