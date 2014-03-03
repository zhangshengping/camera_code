/*
  板级使用的函数，便于理解和配置
*/


#include "common.h"
#include "stdint.h"
#include "board_config.h"
#include "adc.h"
//#include "FTM.h"
#include "uart.h"
#include "board.h"


direction dir_flag;

/*******************************************
 *
 * 角度获取函数配置
 *
********************************************/

//陀螺仪数据获取,获取AD值
uint16_t gyro_data_get(void)
{
  return(ad_once(ADC1,AD9,ADC_16bit));
}


//加速度计数据获取，获取AD值
uint16_t acc_data_get(void)
{
  return(ad_once(ADC0,AD8,ADC_16bit));
}



//*****加速度计角度获取*****
//****用theta代替sin(theta)
//**********加速度计角度获取************
//******用theta代替sin(theta)
float	acc_angle_get()
{
	u16	 data;
	data = ad_once(ADC0,AD8,ADC_16bit);
	
//	return( (ACC_ZERO - data)/ACC_GRA );   //sin = (data - ACC_ZERO)/ACC_GRA , 弧度制输出
	return( 57.3 * (ACC_ZERO - data)/ACC_GRA ); 	//  180/pi = 57.3 , 角度制输出
}

//*****陀螺仪角速度获取*****
float  gyro_angular_get()
{
	u16 data;
	data = ad_once(ADC1,AD9,ADC_16bit);
	
	return((GYRO_ZERO - data)/GYRO_SCALE);		//(data - GRYO_ZERO)/GYRO_SCALE,单位deg/sec,角度制
}


/*************左电机速度控制，包含方向*****************/

void right_run(uint32_t speed,direction d)
{
  if(d == ahead)
  {
    FTM_PWM_Duty(RIGHT_B_FTM,RIGHT_B_CH,0);
    FTM_PWM_Duty(RIGHT_A_FTM,RIGHT_A_CH,speed);
  }
  else
  {
    FTM_PWM_Duty(RIGHT_A_FTM,RIGHT_A_CH,0);
    FTM_PWM_Duty(RIGHT_B_FTM,RIGHT_B_CH,speed);
  }
}







/*************左电机速度控制，包含方向*****************/

void left_run(uint32_t speed,direction d)
{
  if(d == ahead)
  {
    FTM_PWM_Duty(LEFT_B_FTM,LEFT_B_CH,0);
    FTM_PWM_Duty(LEFT_A_FTM,LEFT_A_CH,speed);
  }
  else
  {
    FTM_PWM_Duty(LEFT_A_FTM,LEFT_A_CH,0);
    FTM_PWM_Duty(LEFT_B_FTM,LEFT_B_CH,speed);
  }
}





/*
 *************************************************************************************************************
*
*  函数名   : sent_to_computer
*  功能    : 向Labvew上位机发送数据用于观察滤波效果。
*  参数   ：data1,data2,data3.数据类型均为 uint16_t，三个数据按顺发送，并按顺序在上位机上显示
*  注意  ：在上位机显示时，第一数据位陀螺仪的输出数据，第二个数据为加速度计的输出数据，第三个数据为
*         联合滤波后的输出，虽然数据发送顺序没有任何关系，但为了便于观察数据请在按上位机的显示的顺序发送数据。
*       
 *************************************************************************************************************
*/

void sent_to_computer(uint16_t data1 , uint16_t data2 , uint16_t  data3)
{
  
     uart_putchar(UART0,0xff);                //  发送第一个字节
     uart_putchar(UART0,0xff);                //  发送第二个个字节
     uart_putchar(UART0,(char)(data1 >> 8));       //  发送第一个数据的高八位
     uart_putchar(UART0,(char)(data1 & 0x00ff));   //  发送第一个数据的低第八位
     uart_putchar(UART0,(char)(data2 >> 8));        //  发送第二个数据的高八位
     uart_putchar(UART0,(char)(data2 & 0x00ff));    //  发送第二个数据的低第八位
     uart_putchar(UART0,(char)(data3 >> 8));        //  发送第三个数据的高八位
     uart_putchar(UART0,(char)(data3 & 0x00ff));    //  发送第三个数据的低第八位
}