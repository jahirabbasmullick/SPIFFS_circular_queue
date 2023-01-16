/* 
  Logger.cpp - esp32 library that saves data in SPIFFS

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
#include "SPIFFS_Logger.h"

    Logger::Logger(const char *file_name, long len)
{
    String file_path = "/" + String(file_name);
    this->file_size = 0;
    this->pos_free = 0;
    this->head_index = 0;
    this->tail_index = 0;
    // this->file_buffer = fs::__null;

    if (SPIFFS.exists(file_path))
    {
        this->file_buffer = SPIFFS.open(file_path, "r+");
        if (!this->file_buffer)
        {
            DEBUG_PRINTLN("LOGGER: file open failed");
            // return std::nullptr_t;
        }
        else
        {
            this->recall();
        }
    }
    else
    { // if file does not exist
        this->file_buffer = SPIFFS.open("/" + String(file_name), "w+");
        if (!this->file_buffer)
        {
            DEBUG_PRINTLN("LOGGER: file create failed");
            // return nullptr_t;
        }
        else
        {
            this->file_size = len;
            this->pos_free = len - jump_header();
            this->head_index = jump_header();
            this->tail_index = jump_header();
            // reserve file
            for (int __n = len; __n > 0; __n--)
                this->file_buffer.print(" ");
            this->store();
        }
    }
}

Logger::~Logger()
{
    // TODO: Save pending stuff to file
    // TODO: Save header to file
    // Close file
    this->store();
    this->file_buffer.close();
}

bool Logger::enqueue(const uint8_t *data, long len)
{
    if ((this->pos_free) >= len)
    {
        if (this->file_buffer.seek(this->head_index, SeekSet))
            DEBUG_PRINTLN("SUCCESS");
        if ((this->file_size - this->head_index) >= len)
        {
            this->file_buffer.write(data, len);
            this->head_index += len;
        }
        else
        {
            this->file_buffer.seek(this->head_index, SeekSet); 
            long done = this->file_buffer.write(data, (this->file_size - this->head_index));
            this->head_index = this->jump_header();
            this->file_buffer.seek(this->head_index, SeekSet); 
            this->file_buffer.write(&(data[done]), len - done);
        }
        this->pos_free -= len;
    }
    else
    {
        DEBUG_PRINTLN("LOGGER: no space left in file to write");
        return false;
    }
    this->store();
    return true;
}

bool Logger::dequeue(uint8_t *data, long len)
{
    int ret_byte;

    if (this->num_items() >= len)
    {
        this->file_buffer.seek(this->tail_index, SeekSet);
        while (len--)
        {
            if (this->tail_index == this->file_size)
            {
                this->tail_index = jump_header();
                this->file_buffer.seek(this->tail_index, SeekSet);
            }
            // TODO: Change this to try and real all at once if possible or in two phases
            ret_byte = this->file_buffer.read();
            if (-1 == ret_byte)
            {
                DEBUG_PRINTLN("LOGGER: internal error: READ_BYTES_FAIL");
                return false;
            }
            *data = (uint8_t)ret_byte;
            (this->pos_free)++;
            this->tail_index = (this->tail_index == this->file_size) ? this->jump_header() : this->tail_index + 1;
        }
    }
    else
    {
        DEBUG_PRINTLN("LOGGER: cant read amount of data requested");
        return false;
    }
    return true;
}

void Logger::store()
{
    Serial1.println("THIS IS MY POSITION" + String(this->file_buffer.position()));
    this->file_buffer.seek(0, SeekSet);
    this->file_buffer.printf("%ld|%ld|%ld|%ld|\r\n", this->head_index, this->tail_index, this->file_size, this->pos_free);
}

void Logger::recall()
{
    char term = '|';

    this->file_buffer.seek(0, SeekSet);
    this->head_index = this->file_buffer.readStringUntil(term).toInt();
    this->tail_index = this->file_buffer.readStringUntil(term).toInt();
    this->file_size = this->file_buffer.readStringUntil(term).toInt();
    this->pos_free = this->file_buffer.readStringUntil(term).toInt();
}
