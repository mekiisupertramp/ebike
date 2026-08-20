/*
	Copyright 2016 Benjamin Vedder	benjamin@vedder.se

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#ifndef INC_BUFFER_H_
#define INC_BUFFER_H_

#include <stdint.h>

void buffer_append_int8(uint8_t* buffer, int8_t number, int32_t *index);
void buffer_append_uint8(uint8_t* buffer, uint8_t number, int32_t *index);
void buffer_append_int16(uint8_t* buffer, int16_t number, int32_t *index);
void buffer_append_uint16(uint8_t* buffer, uint16_t number, int32_t *index);
void buffer_append_int32(uint8_t* buffer, int32_t number, int32_t *index);
void buffer_append_uint32(uint8_t* buffer, uint32_t number, int32_t *index);
void buffer_append_uint64(uint8_t* buffer, uint64_t number, int32_t *index);

#define buffer_append_LSB_first_int8 buffer_append_int8
#define buffer_append_LSB_first_uint8 buffer_append_uint8
void buffer_append_LSB_first_int16(uint8_t* buffer, int16_t number, int32_t *index);
void buffer_append_LSB_first_uint16(uint8_t* buffer, uint16_t number, int32_t *index);
void buffer_append_LSB_first_int32(uint8_t* buffer, int32_t number, int32_t *index);
void buffer_append_LSB_first_uint32(uint8_t* buffer, uint32_t number, int32_t *index);
void buffer_append_LSB_first_uint64(uint8_t* buffer, uint64_t number, int32_t *index);

void buffer_append_float16(uint8_t* buffer, float number, float scale, int32_t *index);
void buffer_append_float32(uint8_t* buffer, float number, float scale, int32_t *index);
void buffer_append_float32_auto(uint8_t* buffer, float number, int32_t *index);

int8_t buffer_get_int8(uint8_t *buffer, int32_t *index);
uint8_t buffer_get_uint8(uint8_t *buffer, int32_t *index);
int16_t buffer_get_int16(uint8_t *buffer, int32_t *index);
uint16_t buffer_get_uint16(uint8_t *buffer, int32_t *index);
int32_t buffer_get_int32(uint8_t *buffer, int32_t *index);
uint32_t buffer_get_uint32(uint8_t *buffer, int32_t *index);
float buffer_get_float16(uint8_t *buffer, float scale, int32_t *index);
float buffer_get_float32(uint8_t *buffer, float scale, int32_t *index);
float buffer_get_float32_auto(uint8_t *buffer, int32_t *index);

#endif /* INC_BUFFER_H_ */
