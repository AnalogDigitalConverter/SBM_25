#ifndef __PRINCIPAL_H
#define __PRINCIPAL_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define CIRC_BUFFER_SIZE 	10
#define MAX_STRING_SIZE 	42

typedef struct{
	char data [MAX_STRING_SIZE];
}sample_data_t;

typedef struct{
	sample_data_t sample_data[CIRC_BUFFER_SIZE];
	uint8_t head;
	uint8_t tail;
	uint8_t size;
}circ_buffer_t;

int Init_Principal (void);



#endif
