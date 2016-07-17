/*
 * serialize.c
 *
 *  Created on: 8/5/2016
 *      Author: utnso
 */

#include "serialize.h"

void serialize_from_int(void* buffer, int value, int* offset){
	int offset_loc = *offset;

	memcpy(buffer + offset_loc, &value, sizeof(int));

	*offset = offset_loc + sizeof(int);
}

void serialize_from_string(void* buffer, char* value, int* offset){
	int offset_loc = *offset;

	memcpy(buffer + offset_loc, value, strlen(value)+1);

	*offset = offset_loc + strlen(value)+1;
}

void deserialize_to_int(void* buffer, int* value, int* offset){

	int offset_loc = *offset;

	memcpy(value, buffer + offset_loc, sizeof(int));

	*offset = offset_loc + sizeof(int);
}

void deserialize_to_string(void* buffer, char* value, int size, int* offset){
	int offset_loc = *offset;

	memcpy(value, buffer + offset_loc, size);

	*offset = offset_loc + size;
}

