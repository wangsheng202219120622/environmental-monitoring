#include "bsp_led.h"                             //载入引脚定义的库函数.

void LED_GPIO_Config(void){                      //引脚初始化的函数.
	GPIO_InitTypeDef GPIO_InitStruct;              //GPIO_InitTypeDef 是一个结构体类型，用于配置 STM32F103 系列芯片的 GPIO 端口。在上述代码中，GPIO_InitStruct 是一个变量，用于保存 GPIO 端口的配置信息。
	
	RCC_APB2PeriphClockCmd(LED_G_GPIO_CLK,ENABLE); //GPIOB口的时钟称为使能状态
	RCC_APB2PeriphClockCmd(LED_B_GPIO_CLK,ENABLE); //GPIOB口的时钟称为使能状态
	RCC_APB2PeriphClockCmd(LED_R_GPIO_CLK,ENABLE); //GPIOB口的时钟称为使能状态
	
	GPIO_InitStruct.GPIO_Pin =  LED_G_GPIO_PIN|LED_B_GPIO_PIN|LED_R_GPIO_PIN;   //选择三个待设置引脚
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;                               //模式设置为推挽输出
 	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;                              //输出速率为50mhz
	
	GPIO_Init(LED_G_GPIO_PORT, &GPIO_InitStruct);  //通过调用 GPIO_Init() 函数来将配置应用到相应的 GPIO 端口。
	
}
 
void qc(void){
	LED_G(OFF);
	LED_R(OFF);
	LED_B(OFF);
}