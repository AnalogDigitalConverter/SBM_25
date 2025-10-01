#ifndef __MPU6050_H
#define __MPU6050_H

#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"

#define FLAG_LECTURA_MPU            0x02U

#ifndef MPU6050_REG_H
#define MPU6050_REG_H

// Register Addresses
#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_ACCEL_CONFIG     0x1C
#define MPU6050_RA_ACCEL_XOUT_H     0x3B
#define MPU6050_RA_PWR_MGMT_1       0x6B
#define MPU6050_RA_PWR_MGMT_2       0x6C
#define MPU_SLAVE_ADDR              0x68

#endif // MPU6050_REG_H

int Init_mpu6050_Thread(void);

typedef struct{
	uint8_t reg_addr;
	uint8_t data_AX_H;
	uint8_t data_AX_L;
	uint8_t data_AY_H;
	uint8_t data_AY_L;
	uint8_t data_AZ_H;
	uint8_t data_AZ_L;
	uint8_t data_TEMP_H;
	uint8_t data_TEMP_L;
}in_buffer_t;

typedef struct{
	uint8_t reg_addr;
	uint8_t data;
}out_buffer_t;

typedef struct{
	float accel_x;
	float accel_y;
	float accel_z;
	float temp;
}mpu_msg_t;

#endif //__MPU6050_H



/*****************************************************************************/

