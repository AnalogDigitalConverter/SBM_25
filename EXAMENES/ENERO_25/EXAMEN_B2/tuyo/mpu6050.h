#ifndef __MPU6050_H
#define __MPU6050_H

#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"

int Init_mpu6050_Thread(void);

typedef struct{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    double temp;
}mpu_msg_t;

#endif


/*****************************************************************************/


/*****************************************************************************/
// principal -> medidas_t fifo_buffer[10]
// typedef struct{
//     int16_t accel_x_sample;
//     int16_t accel_y_sample;
//     int16_t accel_z_sample;
//     double temp_sample;
//     uint8_t hora_sample
//     uint8_t minuto_sample
//     uint8_t segundo_sample
// }medidas_t;

