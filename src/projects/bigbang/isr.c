/******************** (C) COPYRIGHT 2011 野火嵌入式开发工作室 ********************
 * 文件名       ：isr.c
 * 描述         ：中断处理例程
 *
 * 实验平台     ：野火kinetis开发板
 * 库版本       ：
 * 嵌入系统     ：
 *
 * 作者         ：野火嵌入式开发工作室
 * 淘宝店       ：http://firestm32.taobao.com
 * 技术支持论坛 ：http://www.ourdev.cn/bbs/bbs_list.jsp?bbs_id=1008
**********************************************************************************/	



#include  "common.h"
#include  "include.h"
#include  "LED_CORE.h"
#include  "board.h"
#include  "image.h"

/****************************************************图像采集*****************************************************/

extern cars_status car;
volatile u8 vert_flag = 0;       //场中断判别标志
volatile u16  row_count = 0;     //行计数
u8   image[ROW][COL] = {0};   //图像存放区
u8  image_handle_flag = 0;      //图像处理标志
//u8 err[COL] = {255};

void PORTE_IRQHandler()     //场中断来临
{
    if(PORTE_ISFR & (1 << 11))
    {
        PORTE_ISFR |= (1 << 11);     
    
//        if(vert_flag != 0) return;
        
//        printf("%d\n",row_count);
        
        vert_flag = 1;  //场标志置位
        row_count = 0;  //行计数清零
        DMA_count_reset(DMA_CH0);
    }
}

unsigned int count2_temp;
void PORTD_IRQHandler()     //行中断来临
{
    u8 i; //计数。
   static unsigned int count2;
    
    if(PORTD_ISFR & (1<<14))
    {
        PORTD_ISFR |= (1 << 14);
        if(vert_flag == 0)   return;     //不足一场时返回        
        row_count = DMA_count_get(DMA_CH0);
//        printf("%d\n",row_count);
        /*******************控制部分*********************/
        if(row_count%80 == 1)           //5ms已到
        {
            /*********滤波及控制算法**********/
            //最小也大约有2.5ms的时间做控制
            
              car->angle_m = acc_data_get();
              car->gyro_m = gyro_data_get();
              blance_kalman_filter(car);

//              OutData[0] = car->angle;
//              OutData[1] = car->angle_m;
//              OutData[2] = car->gyro;
//              OutData[3] = car->gyro_m;
//              send_toscope();
              
//              printf("%f\t%f\t%f\t%f\n",car->angle,car->angle_m,car->gyro,car->gyro_m);
         
           count2++;
           count2_temp = count2;
         //  printf("%d\n",count2);
          if(count2>=20)
          {
               car->speed_right_m  =  right_speed();
               car->speed_left_m   =  (car->speed_right_m >= 0)? left_speed(): -left_speed();
//             printf("%f\t%f\n",car->speed_left_m,car->speed_right_m);
               speed_control(car);
               count2 = 0;
          }
         speed_control_output(car);
         direction_control_output(car);   //方向控制平滑输出，方向控制在下面图像处理部分。
         car->left_duty     = car->blance_duty - car->speed_duty + car->direction_left_duty;
         car->right_duty    = car->blance_duty - car->speed_duty - car->direction_right_duty;
         motor_set(car);  
         
//         printf("%d\n",row_count);
    //     gpio_turn(PORTC,14);

        }
        
        /********************图像处理部分*********************/
        if((row_count > 161)&&(image_handle_flag == 0))         //第三次控制算法已完成且图像未处理
        {
            /**********图像处理************/
        
            for(i=0;i<40;i++)
            {
              edge_l[i] = image_left_offset(image , i);
              edge_r[i] = image_right_offset(image , i);
            }
            image_err(car, 0 ,39);
            direction_control(car);

            printf("%f\n",car->direction_err_new);
            image_handle_flag = 1;      //图像处理标志置1,图像处理及方向计算完成
        }
         
        /******************图像采集部分*******************/
        if(row_count < ROW_START)   return;     //未到需要采集的行
        else if(row_count > ROW_END)
        {
            DMA_DIS(DMA_CH4);
            return;
        }
        else
        {
    //          camera_wait();      //越过消隐区
            DMA_PORTx2BUFF_Init (DMA_CH4, (void *)&PTE_BYTE0_IN, image[row_count-ROW_START], PTE27, DMA_BYTE1, COL, DMA_rising);
            //----使能DMA，初始化的时候禁止DMA
            DMA_EN(DMA_CH4); 
        }
        
        if(row_count == ROW_END)
        {
            image_handle_flag = 0;      //图像采集完成，图像处理标志清零
            
//            vert_flag = 2;
        }
    }

}

void DMA_CH4_Handler(void)
{
    //DMA通道4
    DMA_IRQ_CLEAN(DMA_CH4); //清除通道传输中断标志位    (这样才能再次进入中断)
    
    DMA_EN(DMA_CH4);                                    //使能通道CHn 硬件请求      (这样才能继续触发DMA传输)
}


/************************************************************************************************************************************/


u32 a,b,c,d;

void PIT_CH0_Handler()
{
    PIT_Flag_Clear(PIT0);
//    car->speed_left_m=pulse_cnt_left();
//    car->speed_right_m= pulse_cnt_right();
//    speed_control(car);
}

void PIT_CH1_Handler(void)
{
   
    PIT_Flag_Clear(PIT1); 
    static unsigned int count1,count2;
    switch(count1)
    {
    case 0:
              break;
    case 1:   
              car->angle_m = acc_data_get();
              car->gyro_m = gyro_data_get();
              blance_kalman_filter(car);
//              OutData[0] = car->angle;
//              OutData[1] = car->angle_m;
//              OutData[2] = car->gyro;
//              OutData[3] = car->gyro_m;
//              send_toscope();
//              printf("%f\t%f\t%f\t%f\n",car->angle,car->angle_m,car->gyro,car->gyro_m);
//              printf("%d\n",ad_ave(ADC1,SE10,ADC_16bit,20));
              break;
    case 2:
              break;
    case 3:
             
              break;
    case 4:
           count2++;
          if(count2==20)
          {
               car->speed_right_m  =  right_speed();
               car->speed_left_m   =  (car->speed_right_m >= 0)? left_speed(): -left_speed();
               speed_control(car);
               count2 = 0;
 //            printf("%d\t%d\n",car->speed_left_m,car->speed_right_m);
          }
         speed_control_output(car);
         car->left_duty     = car->blance_duty - car->speed_duty + car->direction_left_duty;
         car->right_duty    = car->blance_duty - car->speed_duty + car->direction_right_duty;
//         printf("%f\t%f\t%f\n",car->blance_duty,car->speed_duty,car->left_duty);
         motor_set(car);
         break;
    default:
              break;
    }
    
  count1++;
  if(count1 == 5)
      count1 = 0;
       
}

