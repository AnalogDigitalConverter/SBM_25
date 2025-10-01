#include "rda5807m.h"
#include "Driver_I2C.h" 
#include "stm32f4xx_hal.h"

//Write 
static uint16_t wr_reg2;
static uint16_t wr_reg3;
static uint16_t wr_reg4;
static uint16_t wr_reg5;
static uint16_t wr_reg6;
static uint16_t wr_reg7;
static uint16_t wr_reg8;


extern ARM_DRIVER_I2C Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;

static osThreadId_t tid_ThRDA;


extern osMessageQueueId_t RDA_MsgQueue;
typedef struct{
  uint32_t vol_level;
}MSGQUEUE_RDA;
MSGQUEUE_RDA RDA_rec;


static void Init_RDA(void);
static int WriteAll(void);
static void Set_Vol(void);
static void Power_On(void);
static void Seek_Up(void);

static void ThRDA(void *argument);

static void callback_i2c(uint32_t event){
  osThreadFlagsSet(tid_ThRDA, event);
}

static void Init_RDA(){
  I2Cdrv-> Initialize   (callback_i2c);
  I2Cdrv-> PowerControl (ARM_POWER_FULL);
  I2Cdrv-> Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv-> Control      (ARM_I2C_BUS_CLEAR, 0);
  
  wr_reg2 = (0x0000 |RDA_DHIZ|RDA_MUTE|RDA_MONO|RDA_NEW_MET);
  wr_reg3 = (0x0000);
  wr_reg4 = (RDA_AFC_DIS);
  wr_reg5 = (RDA_INT_MOD | RDA_SNR_TRESH | RDA_LNA_DUAL | RDA_LNA_2_1mA | RDA_INIT_VOL);
  wr_reg6 = (RDA_CONF_OPEN_MODE);
  wr_reg7 = (0x0000);
  WriteAll();
}

static int WriteAll(){
    static uint32_t flags;
    static uint8_t buffer[30];
//Buffer even values
    buffer[0]  = wr_reg2 >> 8;
    buffer[2]  = wr_reg3 >> 8;
    buffer[4]  = wr_reg4 >> 8;
    buffer[6]  = wr_reg5 >> 8;
    buffer[8]  = wr_reg6 >> 8;
    buffer[10] = wr_reg7 >> 8;
//Buffer odd values
    buffer[3]  = wr_reg3 & 0x00FF;
    buffer[1]  = wr_reg2 & 0x00FF;
    buffer[5]  = wr_reg4 & 0x00FF;
    buffer[7]  = wr_reg5 & 0x00FF;
    buffer[9]  = wr_reg6 & 0x00FF;
    buffer[11] = wr_reg7 & 0x00FF;
    I2Cdrv->MasterTransmit(RDA_WR_ADD, buffer, 30, false);
    flags = osThreadFlagsWait(0xFFFF, osFlagsWaitAny, osWaitForever);
    if((flags & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U && (flags & ARM_I2C_EVENT_TRANSFER_DONE) != true)
      return -1;
    return 0;
}

static void Power_On(){
  wr_reg2 = wr_reg2 | RDA_PWR_ON;
  wr_reg3 =wr_reg3 | RDA_TUNE_ON;
  WriteAll();
  wr_reg3 = wr_reg3 & ~RDA_TUNE_ON ;
}


int Init_ThRDA(void){
  tid_ThRDA = osThreadNew(ThRDA, NULL, NULL);
  if(tid_ThRDA == NULL)
    return(-1);
  return 0;
}

static void ThRDA(void *argument){
  Init_RDA();
  while(1){
    osMessageQueueGet(RDA_MsgQueue, &RDA_rec, 0U, 0U);
    if (RDA_rec.vol_level != 0){
      RDA_rec.vol_level = 0; // "FLAG DEL STATUS"
      Power_On();
      Seek_Up();
      osDelay(10000);
    }
  }
}
static void Seek_Up(){
  wr_reg2 = wr_reg2 | RDA_SEEK_UP;
  WriteAll();
}

static void Set_Vol(){
  wr_reg5= (wr_reg5 & 0xFFF0) | RDA_rec.vol_level;//NotWoking
  WriteAll();
}
