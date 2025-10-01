#include "Control.h"
#include "stdio.h"
#include "Thlcd.h"
#include "Temp.h"
#include "Thjoy.h"
#include "rda5807m.h"
#include "pwm.h"
#include "vol.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 //hilo
osThreadId_t tid_ThCtrl;                        // thread id
void ThCtrl (void *argument);                   // thread function

//Control States
typedef enum {REPOSO, MANUAL, MEMORIA,PROG_HORA} states;
static states control_states;
//Prog_hora states
typedef enum {HORA, MIN, SEC,BM} states2;
static states2 hora_states;

//conexiones
static osMessageQueueId_t JOY_MsgQueue;
static osMessageQueueId_t LCD_MsgQueue;
static osMessageQueueId_t TEMP_MsgQueue;
//status control
static uint8_t status1;
//com struct
int Init_MsgQueue_COM (void);
osMessageQueueId_t COM_MsgQueue;

typedef struct{
  char txt[20];
  uint32_t text_long;
}MSGQUEUE_COM;
MSGQUEUE_COM com_send;

//


int Init_MsgQueue_COM (void) {
 
  COM_MsgQueue = osMessageQueueNew(1, sizeof(MSGQUEUE_COM), NULL);
  if (COM_MsgQueue == NULL) {
  }
  return(0);
}
//RDA QUEUE
int Init_MsgQueue_RDA (void);
osMessageQueueId_t RDA_MsgQueue;

typedef struct{
  uint32_t vol_level;
}MSGQUEUE_RDA;
MSGQUEUE_RDA RDA_send;

//


int Init_MsgQueue_RDA (void) {
 
  RDA_MsgQueue = osMessageQueueNew(1, sizeof(MSGQUEUE_RDA), NULL);
  if (RDA_MsgQueue == NULL) {
  }
  return(0);
}

//joy
static MSGQUEUE_JOY_t mensaje_joystick;
static MSGQUEUE_TEMP_t mensaje_temperatura;
static MSGQUEUE_LCD_t msg_send;

//vol
extern osMessageQueueId_t VOL_MsgQueue;
typedef struct {
  uint32_t volume;
} MSGQUEUE_VOL;
MSGQUEUE_VOL vol_rec;

//variables

bool mensaje_corto = false;

//extern global
extern int lcd_refresh;
extern int segundos;
extern int segundos_co;//segundos cociente
extern int segundos_res;//segundos resto
extern int minutos;
extern int minutos_co;// minutos cociente
extern int minutos_res;// minutos resto
extern int horas;
extern int horas_co;// horas cociente
extern int horas_res; //horas resto 
//extern id
extern osThreadId_t  tid_ThPwm;


int Init_MsgQueue_LCD(void) {
  LCD_MsgQueue = osMessageQueueNew(LCD_TEMP_COUNT, sizeof(MSGQUEUE_LCD_t), NULL);
  status1 = 0;
  if (LCD_MsgQueue == NULL) {return -1;}
  return(0);
}
 
void ThCtrl (void *argument) {
  while (1) {
    switch(control_states){
      case REPOSO:
        status1 = osThreadFlagsWait(FLAG_BUZZER, osFlagsWaitAny, 0U);
        if (status1 == FLAG_BUZZER) {
          osThreadFlagsSet(tid_ThPwm, FLAG_PWM);
          }
        osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
        osMessageQueueGet(TEMP_MsgQueue, &mensaje_temperatura, NULL, 0U);
        sprintf(msg_send.mensaje1,"      SBM2024 ");
        sprintf(msg_send.mensaje2,"      %d%d:%d%d:%d%d   ",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
        osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
        if(mensaje_joystick.corto_larga){
          mensaje_joystick.corto_larga = false;
          control_states = MANUAL;}
      break;
      
      case MANUAL:
        status1 = osThreadFlagsWait(FLAG_BUZZER, osFlagsWaitAny, 0U);
        if (status1 == FLAG_BUZZER) {
          osThreadFlagsSet(tid_ThPwm, FLAG_PWM);
          }
        osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
        osMessageQueueGet(TEMP_MsgQueue, &mensaje_temperatura, NULL, 0U);
        osMessageQueueGet(VOL_MsgQueue, &vol_rec, NULL, 0U);
//        sprintf(msg_send.mensaje1,"%d%d:%d%d:%d%d   L%.1lf$C",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res,mensaje_temperatura.temp);
//        sprintf(msg_send.mensaje2,"Freq sinc Vol level%d",vol_rec.volume);

        sprintf(msg_send.mensaje1,"     M - PWM %d ",vol_rec.volume);
        sprintf(msg_send.mensaje2,"      %d%d:%d%d:%d%d",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
        osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
        RDA_send.vol_level = vol_rec.volume;
        osMessageQueuePut(RDA_MsgQueue, &RDA_send, 0U, 0U);
        if(mensaje_joystick.corto_larga){
          mensaje_joystick.corto_larga = false;
          control_states = MEMORIA;}

      break;
      
      case MEMORIA:
        osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
        sprintf(msg_send.mensaje1,"%d%d:%d%d:%d%d   T:%.1lf$C",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res,mensaje_temperatura.temp);
        sprintf(msg_send.mensaje2,"Nro Memoria  Freq sinc  Vol level.");
        osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
        if(mensaje_joystick.corto_larga){
          mensaje_joystick.corto_larga = false;
          control_states = PROG_HORA;}
      break;
      
      case PROG_HORA:
        switch(hora_states){
          case HORA:
          osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
          sprintf(msg_send.mensaje1,"PROG_HORA");
          sprintf(msg_send.mensaje2,"%d%d:%d%d:%d%d   BM:0/1",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
          osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
          if(mensaje_joystick.pulsacion==1024){
            mensaje_joystick.pulsacion=0;
            horas_res++;}
          if(mensaje_joystick.pulsacion==4096){
            mensaje_joystick.pulsacion=0;
            horas_res--;}
          if(mensaje_joystick.pulsacion==2048){
            mensaje_joystick.pulsacion=0;
            hora_states = MIN;}
          if(mensaje_joystick.corto_larga){
            mensaje_joystick.corto_larga = false;
            control_states = REPOSO;}
          break;
          case MIN:
          osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
          sprintf(msg_send.mensaje1,"PROG_HORA");
          sprintf(msg_send.mensaje2,"%d%d:%d%d:%d%d   BM:0/1",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
          osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
          if(mensaje_joystick.pulsacion==1024){
            mensaje_joystick.pulsacion=0;
            minutos_res++;}
          if(mensaje_joystick.pulsacion==4096){
            mensaje_joystick.pulsacion=0;
            minutos_res--;}
          if(mensaje_joystick.pulsacion==16384){
            mensaje_joystick.pulsacion=0;
            hora_states = HORA;}
          if(mensaje_joystick.pulsacion==2048){
            mensaje_joystick.pulsacion=0;
            hora_states = SEC;}
          if(mensaje_joystick.corto_larga){
            mensaje_joystick.corto_larga = false;
            control_states = REPOSO;}
          break;
          case SEC:
          osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
          sprintf(msg_send.mensaje1,"PROG_HORA");
          sprintf(msg_send.mensaje2,"%d%d:%d%d:%d%d   BM:0/1",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
          osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
          if(mensaje_joystick.pulsacion==1024){
            mensaje_joystick.pulsacion=0;
            segundos_res++;}
          if(mensaje_joystick.pulsacion==4096){
            mensaje_joystick.pulsacion=0;
            segundos_res--;}
          if(mensaje_joystick.pulsacion==16384){
            mensaje_joystick.pulsacion=0;
            hora_states = MIN;}
          if(mensaje_joystick.pulsacion==2048){
            mensaje_joystick.pulsacion=0;
            hora_states = BM;}
          if(mensaje_joystick.corto_larga){
            mensaje_joystick.corto_larga = false;
            control_states = REPOSO;}
          break;
          case BM:
          osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
          sprintf(msg_send.mensaje1,"PROG_HORA");
          sprintf(msg_send.mensaje2,"%d%d:%d%d:%d%d   BM:0/1",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
          osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
          if(mensaje_joystick.pulsacion==1024){
            mensaje_joystick.pulsacion=0;
            control_states = REPOSO;}
          break;
        }
      break;
    }
//    status1 = osThreadFlagsWait(FLAG_BUZZER, osFlagsWaitAny, 0U);
//    if (status1 == FLAG_BUZZER) {
//    osThreadFlagsSet(tid_ThPwm, FLAG_PWM);
//    }
//    osMessageQueueGet(JOY_MsgQueue, &mensaje_joystick, NULL, 0U);
//    osMessageQueueGet(TEMP_MsgQueue, &mensaje_temperatura, NULL, 0U);
//    sprintf(msg_send.mensaje1,"SBM2022   T:%.1lf$C",mensaje_temperatura.temp);
//    sprintf(msg_send.mensaje2,"joy%d   %d%d:%d%d:%d%d",mensaje_joystick.pulsacion,horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
//    osMessageQueuePut(LCD_MsgQueue, &msg_send, 0U, 0U);
//    com_send.text_long = 20;
//    sprintf(com_send.txt,"%d%d:%d%d:%d%d",horas_co,horas_res,minutos_co,minutos_res,segundos_co,segundos_res);
//    osMessageQueuePut(COM_MsgQueue, &com_send, 0U, osWaitForever);
  }
}

osMessageQueueId_t Init_ThCtrl (osMessageQueueId_t JOY_Queue_id, osMessageQueueId_t TEMP_Queue_id){
  Init_MsgQueue_LCD();
  JOY_MsgQueue = JOY_Queue_id;
  TEMP_MsgQueue = TEMP_Queue_id;
  Init_MsgQueue_COM();
  Init_MsgQueue_RDA();
  tid_ThCtrl = osThreadNew(ThCtrl,NULL,NULL);
    if(tid_ThCtrl==NULL){
      return NULL;
    }
    return LCD_MsgQueue;
}
