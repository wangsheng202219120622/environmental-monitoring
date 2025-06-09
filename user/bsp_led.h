#ifndef __BSP_LED_H
#define __BSP_LED_H
 
#include "stm32f10x.h"   //引用库函数

// R实际引脚为B12 G实际引脚为B13 B实际引脚为B14
#define LED_G_GPIO_PIN    GPIO_Pin_12                    //编号为0的引脚定义为LED_G_GPIO_PIN
#define LED_G_GPIO_PORT   GPIOB                          //GPIOB也就是PB口定义为 LED_G_GPIO_PORT    
#define LED_G_GPIO_CLK    RCC_APB2Periph_GPIOB           //GPIOB口的时钟定义为LED_G_GPIO_CLK 
 
#define LED_B_GPIO_PIN    GPIO_Pin_14                     //定义蓝灯引脚
#define LED_B_GPIO_PORT   GPIOB                           
#define LED_B_GPIO_CLK    RCC_APB2Periph_GPIOB  

#define LED_R_GPIO_PIN    GPIO_Pin_13                     //定义红灯引脚
#define LED_R_GPIO_PORT   GPIOB                         
#define LED_R_GPIO_CLK    RCC_APB2Periph_GPIOB  
 
#define     ON         0      //输出低电平 亮
#define     OFF        1	  //输出高电平 灭
 
#define LED_G(a)  if(a) \
						GPIO_ResetBits(LED_G_GPIO_PORT, LED_G_GPIO_PIN);\
						else GPIO_SetBits(LED_G_GPIO_PORT, LED_G_GPIO_PIN);
#define LED_R(a)  if(a)\
						GPIO_ResetBits(LED_R_GPIO_PORT, LED_R_GPIO_PIN);\
						else GPIO_SetBits(LED_R_GPIO_PORT,LED_R_GPIO_PIN);
#define LED_B(a)  if(a)\
						GPIO_ResetBits(LED_B_GPIO_PORT, LED_B_GPIO_PIN);\
						else GPIO_SetBits(LED_B_GPIO_PORT,LED_B_GPIO_PIN);
 
void LED_GPIO_Config(void);
void qc(void); 
 
#endif /*__LED_H*/