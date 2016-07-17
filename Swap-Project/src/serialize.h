/*
 * serialize.h
 *
 *  Created on: 8/5/2016
 *      Author: utnso
 */

#ifndef SRC_SERIALIZE_H_
#define SRC_SERIALIZE_H_

	#include <string.h>

	void serialize_from_int(void* buffer, int value, int* offset);
	void serialize_from_string(void* buffer, char* value, int* offset);
	void deserialize_to_int(void* buffer, int* value, int* offset);
	void deserialize_to_string(void* buffer, char* value, int size, int* offset);
#endif /* SRC_SERIALIZE_H_ */
