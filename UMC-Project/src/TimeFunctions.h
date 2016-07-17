/*
 * TimeFunctions.h
 *
 *  Created on: 29/5/2016
 *      Author: utnso
 */

#ifndef SRC_TIMEFUNCTIONS_H_
#define SRC_TIMEFUNCTIONS_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/temporal.h>
#include <commons/string.h>

char* get_hours(char* time);
char* get_minutes(char* time);
char* get_seconds(char* time);
char* get_actual_time();
int   get_actual_time_integer();
#endif /* SRC_TIMEFUNCTIONS_H_ */
