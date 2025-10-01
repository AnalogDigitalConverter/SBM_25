#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "principal.h"
#include "lcd.h"
#include "com.h"
#include "clock.h"
#include "joy.h"
#include "leds_N.h"
#include "mpu6050.h"

/*----------------------------------------------------------------------------
 *      Thread Principal
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Principal;                        // thread id
extern osThreadId_t tid_leds_N;

extern osTimerId_t id_mpu_sample_tmr;
extern osTimerId_t id_clock_tmr;

extern osMessageQueueId_t id_joy_MsgQueue;
extern osMessageQueueId_t msg_comReceive;
extern osMessageQueueId_t msg_comSend;
extern osMessageQueueId_t msg_lcd;
extern osMessageQueueId_t id_MsgQueue_mpu6050;

extern tiempo_t tiempo;
static double ax_ref = 1;
static double ay_ref = 1;
static double az_ref = 1;

/*---------------------------------------------------------------------------*/
circ_buffer_t sample_buffer;



/*---------------------------------------------------------------------------*/
void Principal (void *argument);                   // thread function

int Init_Principal (void) {

  tid_Principal = osThreadNew(Principal, NULL, NULL);
  if (tid_Principal == NULL) {
    return(-1);
  }

  return(0);
}
/*---------------------------------------------------------------------------*/

void Principal (void *argument) {

  MSG_LCD lcd_prin;
  MSG_JOY joy_prin;
  MSG_COM com_prin;
  mpu_msg_t accel_prin;

  uint8_t prog_mode = 0;
  uint8_t estado = 0x00;
  uint8_t press_joy = 0;
  int8_t cursor = 0;
  bool confirm, increase, decrease = false;

  /*--------------*/
  //buffer
  circ_buffer_t circ_buffer;
  circ_buffer.head = 0;
  circ_buffer.tail = 0;
	circ_buffer.size = CIRC_BUFFER_SIZE;

  /*--------------*/
  Init_clock_Thread();
  Init_com_Thread();
  Init_joy_Thread();
  Init_lcd();
  Init_leds_N();
  Init_mpu6050_Thread();
  /*--------------*/

  while (1) {
    if (osOK == osMessageQueueGet(id_joy_MsgQueue, &joy_prin, 0, 10U))
    {
      osMessageQueueReset(id_joy_MsgQueue);

      if (joy_prin.button == 'C' && joy_prin.long_press == 1){
        estado++;
      }else if (estado == 0x03){
        estado = 0x00;
      }
    }

    switch(estado){
      case 0x00: //REPOSO
        if (!osTimerIsRunning(id_clock_tmr))
        {
          osTimerStart(id_clock_tmr, 1000U);
        }

        //pantalla linea 1
        sprintf(lcd_prin.cadena,"     SBM 2024      ");
        lcd_prin.linea = 1;
        osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);

        //pantalla linea 2
        sprintf(lcd_prin.cadena,"      %02i:%02i:%02i    ", tiempo.horas, tiempo.minutos, tiempo.segundos);
        lcd_prin.linea = 2;
        osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);

        if (osOK == osMessageQueueGet(id_joy_MsgQueue, &joy_prin, 0, 10u))
        {
          osMessageQueueReset(id_joy_MsgQueue);
          if (joy_prin.button == 'C' && joy_prin.long_press == 1){
            estado = 1;
          }
          break;

      case 0x01: //ACTIVO
        //arrancar timer de lectura si not running
        osTimerStart(id_mpu_sample_tmr,1000U);
        osMessageQueueGet(id_MsgQueue_mpu6050, &accel_prin, 0, osWaitForever);
        osMessageQueueReset(id_MsgQueue_mpu6050);


        //buffer_write: if its not full then
        //lleno -> nada
        if ((circ_buffer.head + 1) % circ_buffer.size == circ_buffer.tail){}

        else{ //write
          sprintf(circ_buffer.sample_data[circ_buffer.head].data,"%02i:%02i:%02i--Tm:%.1f-Ax:%.1f-Ay:%.1f-Az:%.1f", tiempo.horas, tiempo.minutos, tiempo.segundos, accel_prin.temp, accel_prin.accel_x, accel_prin.accel_y, accel_prin.accel_z);
          circ_buffer.head = (circ_buffer.head + 1) % circ_buffer.size;
        }


        //pantalla linea 1
        sprintf(lcd_prin.cadena,"   ACTIVO-- T:%.1f'     ", accel_prin.temp);
        lcd_prin.linea = 1;
        osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);

        //pantalla linea 2
        sprintf(lcd_prin.cadena,"   X:%.1f Y:%.1f Z:%.1f", accel_prin.accel_x, accel_prin.accel_y, accel_prin.accel_z);
        lcd_prin.linea = 2;
        osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);


        if(accel_prin.accel_x > ax_ref){
          osThreadFlagsSet(tid_leds_N, 0x01);
        }else if(accel_prin.accel_y > ay_ref){
          osThreadFlagsSet(tid_leds_N, 0x02);
        }else if(accel_prin.accel_z > az_ref){
          osThreadFlagsSet(tid_leds_N, 0x04);
        }else if(accel_prin.accel_x < ax_ref & accel_prin.accel_y < ay_ref & accel_prin.accel_z < az_ref){
          osThreadFlagsSet(tid_leds_N, 0x08);
        }



        if (osOK == osMessageQueueGet(id_joy_MsgQueue, &joy_prin, 0, 0))
        {
          osMessageQueueReset(id_joy_MsgQueue);
          if (joy_prin.button == 'C' && joy_prin.long_press == 1){
            estado = 2;
            prog_mode = 0;
            cursor = 0;
          }
        }

        break;
        case 0x02: //PROGRAMACIÓN
          osTimerStop(id_mpu_sample_tmr);

          //if - JOY FSM
          if (osOK == osMessageQueueGet(id_joy_MsgQueue, &joy_prin, 0, 900U)){
            osMessageQueueReset(id_joy_MsgQueue);
            confirm  = false;
            increase = false;
            decrease = false;
            if(joy_prin.long_press == 0){
              switch (joy_prin.button){
                case 'C':
                  confirm = true;
                  break;
                case 'U':
                  increase = true;
                  break;
                case 'D':
                  decrease = true;
                  break;
                case 'L':
                  cursor--;
                  break;
                case 'R':
                  cursor++;
                  break;
              } //switch

              //cursor_fsm == -1 -> 1; 2 -> 0
              cursor = (cursor == -1)? 1 : cursor % 2;
            } //if short

            else if (joy_prin.button == 'C' && joy_prin.long_press == 1)
            {
              estado = 0;
            }
          } //if - JOY FSM

          //pantalla linea 1
          sprintf(lcd_prin.cadena,"       ---P&D---    ");
          lcd_prin.linea = 1;
          osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);

          //CAMBIOS EN LINEA 2: COM y PROG
          /*-------------------------COM-------------------------------*/
          if (osOK == osMessageQueueGet(msg_comReceive, &com_prin, 0, 0U)){
            switch (com_prin.comando){

              case 0x20:
                //La trama tiene todo en ASCII y hay que pasarlo a decimal entonces:
                //le resto '0' = 0x30 y lo multiplico por 10 porque son las decenas y para unidades solo le resto 0x30 para conseguir las unidades
                com_prin.trama[1] = 0xDF;
                com_prin.trama[2] = 0x0C;

                if (osTimerIsRunning(id_clock_tmr))
                {
                  osTimerStop(id_clock_tmr);
                }

                tiempo.horas    = (com_prin.trama[3] - '0')*10 + (com_prin.trama[4] - '0');
                tiempo.minutos  = (com_prin.trama[6] - '0')*10 + (com_prin.trama[7] - '0');
                tiempo.segundos = (com_prin.trama[9] - '0')*10 + (com_prin.trama[10] - '0');

                sprintf(lcd_prin.cadena,"      H:%02i:%02i:%02i", tiempo.horas, tiempo.minutos, tiempo.segundos);
                lcd_prin.linea = 2;
                osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);
                osMessageQueuePut(msg_comSend, &com_prin, 0, 100U);

                break;

                /*La primera unidad la enconntramos en la trama 3 en ASCII, y le restamos 0x30 para que nos de la unidad
                 *              el primer decimal está en [5], (en [4] está el punto), y le decimos que es un double y lo dividimos entre 10
                 *              para que su resultado sea el primer decimal
                 *              Lo mismo hacemos con el segundo decimal y el resultado es sumarlo todo*/
                case 0x25:
                  com_prin.trama[1] = 0xDA;
                  com_prin.trama[2] = 0x08;
                  ax_ref = (com_prin.trama[3] - '0') + ((double)(com_prin.trama[5] - '0'))/10 + ((double)(com_prin.trama[6] - '0'))/100;
                    
                  osMessageQueuePut(msg_comSend, &com_prin, 0, 100U);
                  break;

                case 0x26:
                  com_prin.trama[1] = 0xD9;
                  com_prin.trama[2] = 0x08;
                  ay_ref = (com_prin.trama[3] - '0') + ((double)(com_prin.trama[5] - '0'))/10 + ((double)(com_prin.trama[6] - '0'))/100;
                    
                  osMessageQueuePut(msg_comSend, &com_prin, 0, 100U);
                  break;

                case 0x27:
                  com_prin.trama[1]=0xD8;
                  com_prin.trama[2] = 0x08;
                  az_ref = (com_prin.trama[3] - '0') + ((double)(com_prin.trama[5] - '0'))/10 + ((double)(com_prin.trama[6] - '0'))/100;
                    
                  osMessageQueuePut(msg_comSend, &com_prin, 0, 100U);
                  break;

                case 0x55:
                  //buffer - show all
                  com_prin.trama[1] = 0xAF;
                  com_prin.trama[2] = 0x27;

                  for(int f = 0; f < CIRC_BUFFER_SIZE; f++)
                  {
                    if (circ_buffer.sample_data[f].data[0] != '\0')
                    {
                      for(int g = 0; g < MAX_STRING_SIZE && circ_buffer.sample_data[f].data[g] != '\0'; g++)
                      { //HASTA QUE SE ENCUENTRE EL '\0'
                        com_prin.trama[g+3] = circ_buffer.sample_data[f].data[g];
                      }
											osMessageQueuePut(msg_comSend, &com_prin, 0, 10U);
                    }
                  }

                  break;

                case 0x60:
                  com_prin.trama[1] = 0x9F;
                  com_prin.trama[2] = 0x04;
                  com_prin.trama[3] = 0xFE;

                  //clear buffer
                  for (int i = circ_buffer.tail; i <= circ_buffer.head; i++){
                    for (int j = 0; j < MAX_STRING_SIZE; j++)
                    {
                      circ_buffer.sample_data[i].data[j] = '\0';
                    }
                  }

                  osMessageQueuePut(msg_comSend, &com_prin, 0, 100U);
                  break;
            } //switch



          }
          /*-------------------------COM-END---------------------------*/

          if (confirm){
            confirm = false;
            prog_mode++;
            if (prog_mode > 5){prog_mode = 0;}
            cursor = 0;
          }

          //cursor select
          if (prog_mode < 3)
          {
            if (osTimerIsRunning(id_clock_tmr))
            {
              osTimerStop(id_clock_tmr);
            }
            // casteo errores:
            if(tiempo.horas == 24){tiempo.horas = 0;}
            else if(tiempo.horas > 24){tiempo.horas = 23;}
            if(tiempo.minutos == 60){tiempo.minutos = 0;}
            else if(tiempo.minutos > 60){tiempo.minutos = 59;}
            if(tiempo.segundos == 60){tiempo.segundos = 0;}
            else if (tiempo.segundos > 60){tiempo.segundos = 59;}

            sprintf(lcd_prin.cadena,"     H: %02i:%02i:%02i", tiempo.horas, tiempo.minutos, tiempo.segundos);
            lcd_prin.linea = 2;
            osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);
            osDelay(290);

            if(prog_mode == 0){
              if(cursor == 0)
              {
                sprintf(lcd_prin.cadena,"     H: _%1i:%02i:%02i", tiempo.horas % 10,tiempo.minutos, tiempo.segundos);

                if (increase)
                {
                  increase = false;
                  tiempo.horas+=10;
                }
                else if (decrease)
                {
                  decrease = false;
                  tiempo.horas-=10;
                }
              }
              else if (cursor == 1)
              {
                sprintf(lcd_prin.cadena,"     H: %1i_:%02i:%02i", tiempo.horas / 10,tiempo.minutos, tiempo.segundos);
                if (increase)
                {
                  increase = false;
                  tiempo.horas++;
                }
                else if (decrease)
                {
                  decrease = false;
                  tiempo.horas--;
                }
              }
            }
            else if(prog_mode == 1){
              if(cursor == 0)
              {
                sprintf(lcd_prin.cadena,"     H: %02i:_%1i:%02i", tiempo.horas,tiempo.minutos % 10, tiempo.segundos);

                if (increase)
                {
                  increase = false;
                  tiempo.minutos+=10;
                }
                else if (decrease)
                {
                  decrease = false;
                  tiempo.minutos-=10;
                }
              }
              else if (cursor == 1)
              {
                sprintf(lcd_prin.cadena,"     H: %02i:%1i_:%02i", tiempo.horas,tiempo.minutos / 10, tiempo.segundos);
                if (increase)
                {
                  increase = false;
                  tiempo.minutos++;
                }
                else if (decrease)
                {
                  decrease = false;
                  tiempo.minutos--;
                }
              }
            }
            else if(prog_mode == 2){
              if(cursor == 0)
              {
                sprintf(lcd_prin.cadena,"     H: %02i:%02i:_%1i", tiempo.horas, tiempo.minutos, tiempo.segundos % 10);
                if (increase)
                {
                  increase = false;
                  tiempo.segundos+= 10;
                }
                else if (decrease)
                {
                  decrease = false;
                  tiempo.segundos-= 10;
                }
              }
              else if (cursor == 1)
              {
                sprintf(lcd_prin.cadena,"     H: %02i:%02i:%1i_", tiempo.horas,tiempo.minutos, tiempo.segundos / 10);
                if (increase)
                {
                  increase = false;
                  tiempo.segundos++;
                }
                else if (decrease)
                {
                  decrease = false;
                  tiempo.segundos--;
                }
              }
            }
          } //prog-mode < 3
          if(prog_mode >= 3)
          {
            if(ax_ref > 2){ax_ref = -2.0;}
            else if(ax_ref < -2){ax_ref = 2.0;}
            else if(ay_ref > 2){ay_ref = -2.0;}
            else if(ay_ref < -2){ay_ref = 2.0;}
            else if(az_ref > 2){az_ref = -2.0;}
            if(az_ref < -2){az_ref = 2.0;}
            
            sprintf(lcd_prin.cadena,"x=%.2f y=%.2f z=%.2f", ax_ref, ay_ref, az_ref);
            lcd_prin.linea = 2;
            osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);
            osDelay(290);
            
            if (prog_mode == 3){
              //aceleraciones: modo limpito
              //olvídate de cursores
              //sprintf(lcd_prin.cadena,"x=__ y=%.2f z=%.2f", ay_ref, az_ref);
              //incrementar de 0.1 en 0.1
              if(cursor == 0)
              {
                sprintf(lcd_prin.cadena,"x=_%.1f y=%.2f z=%.2f", ax_ref - (int)ax_ref, ay_ref, az_ref);
                if (increase)
                {
                  increase = false;
                  ax_ref++;
                }
                else if (decrease)
                {
                  decrease = false;
                  ax_ref--;
                }
              }
              else if (cursor == 1)
              {
                sprintf(lcd_prin.cadena,"x=%i._ y=%.2f z=%.2f", (int)ax_ref, ay_ref, az_ref);
                if (increase)
                {
                  increase = false;
                  ax_ref+=0.1;
                }
                else if (decrease)
                {
                  decrease = false;
                  ax_ref-=0.1;
                }
              }
            }
            else if (prog_mode == 4){
              //aceleraciones: modo limpito
              //olvídate de cursores
              //sprintf(lcd_prin.cadena,"x=%.2f y=__ z=%.2f", ax_ref, az_ref);
              //incrementar de 0.1 en 0.1
              if(cursor == 0)
              {
                sprintf(lcd_prin.cadena,"x=%.2f y=_%.1f z=%.2f", ax_ref, ay_ref - (int)ay_ref, az_ref);
                if (increase)
                {
                  increase = false;
                  ay_ref++;
                }
                else if (decrease)
                {
                  decrease = false;
                  ay_ref--;
                }
              }
              else if (cursor == 1)
              {
                sprintf(lcd_prin.cadena,"x=%.2f y=%i._ z=%.2f", ax_ref, (int)ay_ref, az_ref);
                if (increase)
                {
                  increase = false;
                  ay_ref+=0.1;
                }
                else if (decrease)
                {
                  decrease = false;
                  ay_ref-=0.1;
                }
              }
            }
            else if (prog_mode == 5){
              //aceleraciones: modo limpito
              //olvídate de cursores
              //sprintf(lcd_prin.cadena,"x=%.2f y=%.2f z=__", ax_ref, ay_ref);
              //incrementar de 0.1 en 0.1
              if(cursor == 0)
              {
                sprintf(lcd_prin.cadena,"x=%.2f y=%.2f z=_%.1f", ax_ref, ay_ref, az_ref - (int)az_ref);
                if (increase)
                {
                  increase = false;
                  az_ref++;
                }
                else if (decrease)
                {
                  decrease = false;
                  az_ref--;
                }
              }
              else if (cursor == 1)
              {
                sprintf(lcd_prin.cadena,"x=%.2f y=%.2f z=%i._", ax_ref, ay_ref, (int)az_ref);
                if (increase)
                {
                  increase = false;
                  az_ref+=0.1;
                }
                else if (decrease)
                {
                  decrease = false;
                  az_ref-=0.1;
                }
              } //cursor 1
            } //prog_mode 5
          } //prog_mode >= 3

          lcd_prin.linea = 2;
          osMessageQueuePut(msg_lcd, &lcd_prin, 0, osWaitForever);
        }
        break;  //PROG
    }// switch(estado)
  } //while(1)
} // Principal
