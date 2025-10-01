#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "mpu6050.h"

#ifndef MPU6050_REG_H
#define MPU6050_REG_H

// Register Addresses
#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_ACCEL_CONFIG     0x1C

#define MPU6050_RA_ACCEL_XOUT_H     0x3B
#define MPU6050_RA_ACCEL_YOUT_H     0x3D
#define MPU6050_RA_ACCEL_ZOUT_H     0x3F
#define MPU6050_RA_TEMP_OUT_H       0x41

#define MPU6050_RA_PWR_MGMT_1       0x6B
#define MPU6050_RA_PWR_MGMT_2       0x6C

#define MPU_SLAVE_ADDR              0x68

#endif // MPU6050_REG_H

#define FLAG_LECTURA 	0x02U
#define I2C_TX_RX			0x04U
#define MAX_MSG_COUNT 10

typedef struct{
	uint8_t reg_addr;
  uint8_t data_H;
  uint8_t data_L;
}in_buffer_t;

typedef struct{
  uint8_t reg_addr;
  uint8_t data;
}out_buffer_t;


/*----------------------------------------------------------------------------
 *      Thread 'mpu6050': GY-521 module featuring MPU6050 accelerometer (i2C)
 *---------------------------------------------------------------------------*/

/*****************************************************************************/

extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

static osMessageQueueId_t id_MsgQueue_mpu6050;
static mpu_msg_t mpu6050_msg;

in_buffer_t x_data, y_data, z_data, temp_data;
out_buffer_t conf_buffer;

static int mpu6050_init(void);


int mpu_write(out_buffer_t out_buffer);
int mpu_read(in_buffer_t* in_buffer);
void in_buffer_clear(in_buffer_t* data);


/*****************************************************************************/
osThreadId_t id_mpu6050_Thread;                        // thread id

void mpu6050_Thread(void *argument);                   // thread function

int Init_mpu6050_Thread(void) {

  id_mpu6050_Thread = osThreadNew(mpu6050_Thread, NULL, NULL);
  if (id_mpu6050_Thread == NULL) {
    return(-1);
  }
  return(0);
}

/*****************************************************************************/
static int Init_mpu6050_Queue(void){

  id_MsgQueue_mpu6050 = osMessageQueueNew(MAX_MSG_COUNT, sizeof(mpu_msg_t), NULL);
  if(id_MsgQueue_mpu6050 == NULL){
    return(-1);
  }
  return (0);
}

/*****************************************************************************/
void I2C_TX_RX_Callback(uint32_t flags){
	osThreadFlagsSet(id_mpu6050_Thread, I2C_TX_RX);
}

/*****************************************************************************/
void mpu6050_Thread (void *argument) {

  /*HW & QUEUE START CONDITION*/
  if (!Init_mpu6050_Queue()){ 
		if(!mpu6050_init()){
			while (1) {
				//Flag o modo entrada FSM a medidas. Llamada desde principal con timer periódico de 1 seg?
				//osThreadFlagsWait(FLAG_LECTURA, osFlagsWaitAny, osWaitForever);

				 //leer accel(x, y, z)
				x_data.reg_addr = MPU6050_RA_ACCEL_XOUT_H;
				mpu_read(&x_data);
				
				y_data.reg_addr = MPU6050_RA_ACCEL_YOUT_H;
				mpu_read(&y_data);
				
				z_data.reg_addr = MPU6050_RA_ACCEL_ZOUT_H;
				mpu_read(&z_data);

				//leer temp
				temp_data.reg_addr = MPU6050_RA_TEMP_OUT_H;
				mpu_read(&temp_data);

				//tratamiento de datos, generalmente en Ca1 o Ca2
				mpu6050_msg.accel_x = (x_data.data_H | x_data.data_L);
				mpu6050_msg.accel_y = (y_data.data_H | y_data.data_L);
				mpu6050_msg.accel_z = (z_data.data_H | z_data.data_L);

				mpu6050_msg.temp = (temp_data.data_H | temp_data.data_L);

				//poner en cola. El FIFO ->
				//Lo ponemos en cola
				osMessageQueuePut(id_MsgQueue_mpu6050, &mpu6050_msg, 0U, 0U);

				//data clear?->limpia variables -> ayuda a debug
				in_buffer_clear(&x_data);
				in_buffer_clear(&y_data);
				in_buffer_clear(&z_data);
				in_buffer_clear(&temp_data);
			}// loop-------------------------------------------------------------------
		}
	}
}

/*****************************************************************************/

static int mpu6050_init(void)
{
	I2Cdrv-> Initialize   (I2C_TX_RX_Callback);
	I2Cdrv-> PowerControl (ARM_POWER_FULL);
	I2Cdrv-> Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
	I2Cdrv-> Control      (ARM_I2C_BUS_CLEAR, 0);
	I2Cdrv ->Control			(ARM_I2C_OWN_ADDRESS, 0x68);

  // Wake up the device
	conf_buffer.reg_addr = MPU6050_RA_PWR_MGMT_1;
	conf_buffer.data = 0x00; // Reset all registers
  mpu_write(conf_buffer);


  // Configure accelerometer sensitivity and full-scale range
	conf_buffer.reg_addr = MPU6050_RA_ACCEL_CONFIG;
	conf_buffer.data = 0x00;// ±2g, 16-bit resolution
  mpu_write(conf_buffer);
  
  //self test on RA_ACCEL_CONFIG  0xE0

  /* Sleep mode while config (Sensors off, not measuring) */
  //Note: The device will come up in sleep mode upon power-up
  //data = {MPU6050_RA_PWR_MGMT_1, 0x40}//Sleep mode enable (copy when desired)

  /* Sets the Cyclic Measurement to 5Hz and disables all Gyroscopes */
  // data = {MPU6050_RA_PWR_MGMT_1, 0x20}// Cyclic SLEEP-MEAS MODE
  // mpu_write(data);
  // data = {MPU6050_RA_PWR_MGMT_2, 0x47}// Cyclic SLEEP-MEAS 5Hz, Gyros STBY
  // mpu_write(data);

  return 0;
}

int mpu_write(out_buffer_t out_buffer){
	uint32_t flags;
	
	uint8_t content[2];
	content[0] = out_buffer.reg_addr;
	content[1] = out_buffer.data;
	
  I2Cdrv->MasterTransmit(MPU_SLAVE_ADDR, content, sizeof(content), false);

	flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, osWaitForever);
	if((flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;
	return 0;
	
}

int mpu_read(in_buffer_t* in_buffer){
	uint32_t flags;

	uint8_t content[] = {0, 0};
	
  //Single-Byte Write Sequence without P signal -> Comm Ongoing
  //Place onto desired register to read MPU_SLAVE_ADDR << 1 | 0
  I2Cdrv->MasterTransmit(MPU_SLAVE_ADDR, &in_buffer->reg_addr , 1, true);
  	flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, osWaitForever);
	if((flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)return -1;

  //read & populate in_buffer with MSB & LSB sequential bytes given by I2C MPU_SLAVE_ADDR << 1 | 1
  I2Cdrv->MasterReceive(MPU_SLAVE_ADDR, content, 2, false);
	flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, osWaitForever);
	if((flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (flags & ARM_I2C_EVENT_TRANSFER_DONE) != true){
		return -1;
	}
	
	in_buffer->data_H = content[0];
	in_buffer->data_L = content[1];

	return 0;
}

void in_buffer_clear(in_buffer_t* data)
{
  data->data_H = 0;
  data->data_L = 0;
}




