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
#include "Kalman.h"
direction dir_flag;

const int right_dead = 3;  //电机死区
const int left_dead  = 3;


/*******************************************
 *
 * 角度获取函数配置
 *
********************************************/

//陀螺仪数据获取,获取AD值
float gyro_data_get(void)
{
  
  return(-((GYRO_ZERO - ad_once(ADC1,AD9,ADC_16bit)) / GYRO_SCALE));
  
}


//加速度计数据获取，获取AD值
float acc_data_get(void)
{
  
   return(180*(ACC_ZERO-ad_once(ADC0,AD8,ADC_16bit))/(3.14*ACC_GRA));
  
}

//****陀螺仪和加速度计初始化
void angle_get_init()
{

  adc_init(ADC0,AD8); 	//加速度计的AD通道初始化

  adc_init(ADC1,AD9);  //陀螺仪AD通道初始化
}



/*************电机驱动初始化*****************/
void motor_init(void)
{
  FTM_PWM_init(RIGHT_A_FTM,RIGHT_A_CH,MOTOR_FRE,INIT_DUTY);
  FTM_PWM_init(RIGHT_B_FTM,RIGHT_B_CH,MOTOR_FRE,INIT_DUTY);
  FTM_PWM_init(LEFT_A_FTM,LEFT_A_CH,MOTOR_FRE,INIT_DUTY);
  FTM_PWM_init(LEFT_B_FTM,LEFT_B_CH,MOTOR_FRE,INIT_DUTY);
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


void right_run_s(int32_t speed)       //speed的符号体现方向
{
  direction dir;
  if(speed > 0)
  {
    dir = ahead;
//    speed = speed +right_dead;
  }
  else if(speed <0)
  {
    dir = back;
    speed = -speed + right_dead;
  }
  else
  {
    speed = 0;
  }
  right_run(speed,dir);
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



void left_run_s(int32_t speed)   //speed的符号体现方向
{
  direction dir;
  if(speed > 0)
  {
    dir = ahead;
    speed = speed +right_dead;
  }
  else if(speed <0)
  {
    dir = back;
    speed = -speed + right_dead;
  }
  else
  {
    speed = 0;
  }
  left_run(speed,dir);
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



//float PID_implement(float data_in,pid_struct pid)
//{
//  pid->SetDate = data_in;
//  pid->err =pid->SetData-pid->ActualData;
//  pid->intergal +=pid->err;
//  pid->Implement
//}

/***********************************************
*   将字符串转换为浮点数
*   str为字符串首地址，n为字符串长度
***********************************************/

float str2num(char * str,u8 n)
{
    u8 point = 0;       //查询小数点，返回小数点的位置，若无则为0
    int len = 0;               //字符串长度
    float inc = 1;
    u8 p,l;
    float num;
    
    for(len=0; len<n; len++)
    {
        if(str[point] != '.')
           point ++;
    }
    
    if(point > len)      //无小数点
    {
        for(len=n-1; len>=0; len--)       //最后一个字符的标号
        {
            num = num + ((u8)str[len]-48)*inc;
            
            inc = inc*10;
        }
    }
    else
    {
        len = n-1;              //最后一个字符的标号
        p = point;
        l = len;
        while(point < len)
        {
            point++;
            inc = inc*0.1;
            
            num = num + ((u8)str[point]-48)*inc;
        }
        point = p;
        len = l;
        inc = 1;
        while(point > 0)
        {
            point--;
            
            num = num+((u8)str[point]-48)*inc;
            
            inc = inc*10;
        }
    }
    
    return num;
}


//extern cars_status car;
void blance_comp_filter(float tg,float dt,cars_status car)
{
  
  float angle_m,gyro_m;
  angle_m = acc_data_get();
  gyro_m  = gyro_data_get();
  comp_filter(angle_m,gyro_m,tg, dt,car);
  (car->left_duty)=(car->right_duty) = (car->angle - car->angle_set)*car->angle_p + (gyro_m - car->gyro_set)*car->gyro_d;  
   right_run_s((int32_t)car->right_duty);
   left_run_s((int32_t)car->left_duty);
}
