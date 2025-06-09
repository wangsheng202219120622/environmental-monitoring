#include "FMQ.h"
#include "paff.h"
#include "stm32f10x.h"   
#include "bsp_led.h"
#include "Buzzer.h"
/**
   * @brief  这个文件包含了一个名为mfq_Init的函数，
	 用于初始化一个GPIO引脚（GPIOB的第0脚）作为蜂鸣器的控制引脚。
	 还有一个名为fmq的函数，它接受温度和湿度作为参数，并根据这些值控制蜂鸣器的开关。
	
   * @param
   * @retval 
   */

void mfq_Init(void)	
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}

void fmq(float temp,float humi)
{
	LED_GPIO_Config();
	int min_t=paff0[0];  
	int max_t=paff0[1];
	int led=paff0[2];
	
	int Min_H=paff1[0];
	int Max_H=paff1[1];
	int buzzer=paff1[2];
		
	if(led){
		if(temp>max_t)
		{
			//添加各种条件
			qc();
			LED_R(ON);
    }
		if(temp>min_t &&temp<max_t) 
		{
			qc();
			LED_G(ON);
		}
		if(temp<min_t)
		{
			qc();
			LED_B(ON);
		}
	}
	else
	{qc();}
	
	
	if(buzzer)
	{
		if(humi>Max_H)
		{
			Buzzer_ON();	
    }
		if(humi>Min_H &&humi<Max_H) 
		{
			Buzzer_OFF();	
		}
		if(humi<Min_H)
		{
			Buzzer_ON();	
		}
	}else
	{
		Buzzer_OFF();	
	}
	
}

