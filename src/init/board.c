/*
  板级使用的函数，便于理解和配置
*/
#include "include.h"
#include "common.h"
#include "stdint.h"
#include "board_config.h"
#include "adc.h"
#include "uart.h"
#include "board.h"
#include "Kalman.h"
direction dir_flag;


#define RIGHT_DEAD 10
#define LEFT_DEAD  10
const int right_dead = 10;  //电机死区
const int left_dead  = 10;


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
  
   return(180*(ACC_ZERO-ad_once(ADC0,AD8,ADC_16bit))/(3.1416*ACC_GRA));
  
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
  if(speed>0)
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


/*************************************
*   编码器初始化
*************************************/
void speed_init()
{
    DMA_count_Init(DMA_CH4, PTA24, 10000, DMA_rising_up);
    DMA_count_Init(DMA_CH5, PTA26, 10000, DMA_rising_up);
    DMA_count_Init(DMA_CH6, PTA28, 10000, DMA_rising_up);
    DMA_count_Init(DMA_CH7, PTA29, 10000, DMA_rising_up);
    
    pit_init_ms(SPEED_PIT,SPEED_SAMPLING_TIME);
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


//extern cars_status car;
void blance_comp_filter(float tg,float dt,cars_status car)
{
  
  float angle_m,gyro_m;
  angle_m = acc_data_get();
  gyro_m  = gyro_data_get();
  comp_filter(angle_m,gyro_m,tg, dt,car);
  car->blance_duty = (car->angle - car->angle_set)*car->angle_p + (gyro_m - car->gyro_set)*car->gyro_d ;
  //printf("blance_duty:%f\n",car->blance_duty);
}


/*********************************************************************************************
*       速度控制函数，根据设定速度计算占空比。
*
*
**********************************************************************************************/
void speed_control(cars_status car)
{
  float speed_err;
  static float speed_integral;
  speed_err        = car->speed_set - (car->speed_left_m +  car->speed_right_m)/2.0 ;
  speed_integral  += (car->speed_p)*speed_err;
  car->speed_duty  =  speed_integral + (car->speed_d)*speed_err;
 // printf("speed_duty:%f\n",car->speed_duty);
  
}

/*********************************************************************************************
*       电机控制。
*
*
**********************************************************************************************/


void motor_set(cars_status car)
{
  car->left_duty  = (car->blance_duty) - (car->speed_duty) - (car->direction_left_duty);
  car->right_duty = (car->blance_duty) - (car->speed_duty) + (car->direction_right_duty);
 // printf("%f\t%f\n",car->left_duty,car->right_duty);
  if(((car->left_duty)>990)||((car->left_duty)<-990)||((car->right_duty)>990)||((car->right_duty)<-990))
    {
      (car->left_duty) = (car->right_duty) = 0;
    }
  left_run_s((int32_t)(car->left_duty));
  right_run_s((int32_t)(car->right_duty));
}