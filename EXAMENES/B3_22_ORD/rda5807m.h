#ifndef __RDA_H
#define __RDA_H
#include "cmsis_os2.h"

//ADDRESSES
#define RDA_WR_ADD 0x20 >> 1
#define RDA_RD_ADD 0x22 >> 1
//Starting values
#define RDA_DHIZ 0x8000
#define RDA_MUTE 0x6000
#define RDA_MONO 0x4000
#define RDA_SEEK_UP 0x0300
#define RDA_SEEK_DOWN 0x0100
#define RDA_NEW_MET 0x0004
#define RDA_PWR_ON 0x0001
//RDA TUNE FOR POWERON
#define RDA_TUNE_ON 0x0010
//Reg4
#define RDA_AFC_DIS 0x0100
//Reg5
#define RDA_INT_MOD 0x8000
#define RDA_SNR_TRESH 0x0400
#define RDA_LNA_DUAL 0x00C0
#define RDA_LNA_2_8mA 0x0020
#define RDA_LNA_2_1mA 0x0010
#define RDA_LNA_1_8mA 0x0000
#define RDA_INIT_VOL 1//14
//Reg6
#define RDA_CONF_OPEN_MODE 0x4000

int Init_ThRDA(void);

static osThreadId_t tid_ThRDA;


#endif
