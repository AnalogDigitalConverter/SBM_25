#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "mpu6050.h"

#define I2C_TX_RX           0x04U
#define MAX_MSG_COUNT       10

/*----------------------------------------------------------------------------
 *      Thread 'mpu6050': GY-521 module featuring MPU6050 accelerometer (i2C)
 *      8/DIC/24 -> Test nueva conversión de datos. OK
 *			17/DIC/24 -> Callback & flag fix
 *---------------------------------------------------------------------------*/

/*****************************************************************************/
osThreadId_t id_mpu6050_Thread;                        // thread id
void mpu6050_Thread(void *argument);                   // thread function

//I2C (1)
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

//queue dependencies
osMessageQueueId_t id_MsgQueue_mpu6050;
mpu_msg_t mpu6050_msg;

osTimerId_t id_mpu_sample_tmr;
void mpu_sample_tmr_callback(void *args);

//HW init
int mpu6050_init(void);

//Read and Write buffers, I2C require pointer to address
in_buffer_t read_buffer;
out_buffer_t conf_buffer;

//Read & Write funcions require I2C, return int
int mpu_write(out_buffer_t out_buffer);
int mpu_readout(in_buffer_t* in_buffer);

//aux functions
void in_buffer_clear(in_buffer_t* data);
void readout_to_msg(in_buffer_t in_buffer, mpu_msg_t* mpu_msg);

/*****************************************************************************/
int Init_mpu6050_Thread(void)
{
    id_mpu6050_Thread = osThreadNew(mpu6050_Thread, NULL, NULL);
    if (id_mpu6050_Thread == NULL) {
        return(-1);
    }
    return(0);
}

/*****************************************************************************/
static int Init_mpu6050_Queue(void)
{
    id_MsgQueue_mpu6050 = osMessageQueueNew(MAX_MSG_COUNT, sizeof(mpu_msg_t), NULL);
    if(id_MsgQueue_mpu6050 == NULL){
        return(-1);
    }
    return (0);
}

/*****************************************************************************/
void I2C_TX_RX_Callback(uint32_t flags)
{
	if((flags & ARM_I2C_EVENT_TRANSFER_DONE) == true){
        osThreadFlagsSet(id_mpu6050_Thread, I2C_TX_RX);
    }
}

/*****************************************************************************/
void mpu6050_Thread (void *argument)
{
    /*HW & QUEUE START CONDITION*/
    if (!Init_mpu6050_Queue()){
        if(!mpu6050_init()){
			id_mpu_sample_tmr = osTimerNew(mpu_sample_tmr_callback, osTimerPeriodic, NULL, NULL);
            while (1) {
                //Flag o modo entrada FSM a medidas.
                //Llamada desde principal con timer periódico de 1 seg?
                osThreadFlagsWait(FLAG_LECTURA_MPU, osFlagsWaitAny, osWaitForever);
							
                read_buffer.reg_addr = MPU6050_RA_ACCEL_XOUT_H;
                mpu_readout(&read_buffer);

                //tratamiento de datos
                readout_to_msg(read_buffer, &mpu6050_msg);

                //poner en cola. El FIFO ->
                osMessageQueuePut(id_MsgQueue_mpu6050, &mpu6050_msg, 0U, 0U);

                //data clear?-> limpia variables -> ayuda a debug
                in_buffer_clear(&read_buffer);

            }// loop-----------------------------------------------------------
        }
    }
}

/*****************************************************************************/
int mpu6050_init(void)
{
    I2Cdrv-> Initialize     (I2C_TX_RX_Callback);
    I2Cdrv-> PowerControl   (ARM_POWER_FULL);
    I2Cdrv-> Control        (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    I2Cdrv-> Control        (ARM_I2C_BUS_CLEAR, 0);

    uint8_t content[2];
    uint32_t flags;

    // Wake up the device
    content[0]= MPU6050_RA_PWR_MGMT_1;
    content[1] = 0x00;
    /*Send*/
    I2Cdrv->MasterTransmit(MPU_SLAVE_ADDR, content, 2, false);
		flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);

    // Configure accelerometer sensitivity and full-scale range
    content[0] = MPU6050_RA_ACCEL_CONFIG;
    content[1] = 0x00; // ±2g, 16-bit resolution
    /*Send*/
    I2Cdrv->MasterTransmit(MPU_SLAVE_ADDR, content, 2, false);
		flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);
		
    return 0;
}

int mpu_readout(in_buffer_t* in_buffer){
    uint32_t flags;
    uint8_t content_rx[8];

    //Tell reg_addr RA to mpu6050
    /*Send -> pending true*/
    I2Cdrv->MasterTransmit(MPU_SLAVE_ADDR, &in_buffer->reg_addr, 1, true);
		flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);

    //Receive data dump (all desired 8 registers are consecutive)
    /*Receive*/
    I2Cdrv->MasterReceive(MPU_SLAVE_ADDR, content_rx, sizeof(content_rx), false);
    flags = osThreadFlagsWait(I2C_TX_RX, osFlagsWaitAny, osWaitForever);


    //array to readable buffer -> data treatment
    in_buffer->data_AX_H 		= content_rx[0];
    in_buffer->data_AX_L 		= content_rx[1];
    in_buffer->data_AY_H 		= content_rx[2];
    in_buffer->data_AY_L 		= content_rx[3];
    in_buffer->data_AZ_H 		= content_rx[4];
    in_buffer->data_AZ_L 		= content_rx[5];
    in_buffer->data_TEMP_H 		= content_rx[6];
    in_buffer->data_TEMP_L 		= content_rx[7];
		
    return 0;
}

void in_buffer_clear(in_buffer_t* in_buffer)
{
    in_buffer->data_AX_H 	= 0;
    in_buffer->data_AX_L 	= 0;
    in_buffer->data_AY_H 	= 0;
    in_buffer->data_AY_L 	= 0;
    in_buffer->data_AZ_H 	= 0;
    in_buffer->data_AZ_L 	= 0;
    in_buffer->data_TEMP_H 	= 0;
    in_buffer->data_TEMP_L 	= 0;
}

void readout_to_msg(in_buffer_t in_buffer, mpu_msg_t* mpu_msg)
{
	//single "local" variables declaration 
    static int16_t accel_x, accel_y, accel_z, temp = 0;

	//data casting
    accel_x = (in_buffer.data_AX_H << 8) | (in_buffer.data_AX_L); // X
    accel_y = (in_buffer.data_AY_H << 8) | (in_buffer.data_AY_L); // Y
    accel_z = (in_buffer.data_AZ_H << 8) | (in_buffer.data_AZ_L); // Z

    // Combinar los bytes en un valor de 16 bits con signo
    temp = (in_buffer.data_TEMP_H << 8) | (in_buffer.data_TEMP_L);

    //int16_t to float units -> sign + 15bit /(+-2g range) = 2^14 = 16384
    mpu_msg->accel_x = accel_x / 16384.f;
    mpu_msg->accel_y = accel_y / 16384.f;
    mpu_msg->accel_z = accel_z / 16384.f;

    // Convertir temp-int16_t a grados Celsius float
    mpu_msg->temp = (temp / 340.f) + 36.53f;
}

void mpu_sample_tmr_callback(void *args){
	osThreadFlagsSet(id_mpu6050_Thread, FLAG_LECTURA_MPU);
}
