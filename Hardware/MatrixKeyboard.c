
#include "stm32f10x.h" // Device header
#include "Delay.h"

#define ROW1_PIN GPIO_Pin_10  //B
#define ROW2_PIN GPIO_Pin_1		//B

#define COL1_PIN GPIO_Pin_11  //B
#define COL2_PIN GPIO_Pin_7   //A
#define COL3_PIN GPIO_Pin_6   //A

void Key_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable clock for GPIOB
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
    // Configure ROW1 pin as output
    GPIO_InitStructure.GPIO_Pin = ROW1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = ROW2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Configure COL1 pin as input with pull-up
    GPIO_InitStructure.GPIO_Pin = COL1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Input with pull-up
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = COL2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Input with pull-up
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = COL3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Input with pull-up
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
uint8_t Key_Check()
{
    uint8_t KeyNum = 0; // 定义变量，默认键码值为0

    // Check if COL1 is low (button pressed)	
		GPIO_WriteBit(GPIOB,GPIO_Pin_10,0);
    if(GPIO_ReadInputDataBit(GPIOB, ROW1_PIN) == 0&&GPIO_ReadInputDataBit(GPIOB, COL1_PIN) == 0) // 使用RESET代替逻辑非操作符!
    {
        Delay_ms(20); // Debounce delay
   
        while(GPIO_ReadInputDataBit(GPIOB, ROW1_PIN) == 0&&GPIO_ReadInputDataBit(GPIOB, COL1_PIN) == 0) ;
        Delay_ms(20); // Debounce delay
        KeyNum = 1; // Set key code to 1
    }
		if(GPIO_ReadInputDataBit(GPIOB, ROW1_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL2_PIN) == 0) // 使用RESET代替逻辑非操作符!
    {
        Delay_ms(20); // Debounce delay
   
        while(GPIO_ReadInputDataBit(GPIOB, ROW1_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL2_PIN) == 0) ;
        Delay_ms(20); // Debounce delay
        KeyNum = 2; // Set key code to 1
    }
		if(GPIO_ReadInputDataBit(GPIOB, ROW1_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL3_PIN) == 0) // 使用RESET代替逻辑非操作符!
    {
        Delay_ms(20); // Debounce delay
   
        while(GPIO_ReadInputDataBit(GPIOB, ROW1_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL3_PIN) == 0) ;
        Delay_ms(20); // Debounce delay
        KeyNum = 3; // Set key code to 1
    }
		GPIO_WriteBit(GPIOB,GPIO_Pin_10,1);
		
		GPIO_WriteBit(GPIOB,GPIO_Pin_1,0);
		if(GPIO_ReadInputDataBit(GPIOB, ROW2_PIN) == 0&&GPIO_ReadInputDataBit(GPIOB, COL1_PIN) == 0) // 使用RESET代替逻辑非操作符!
    {
        Delay_ms(20); // Debounce delay
   
        while(GPIO_ReadInputDataBit(GPIOB, ROW2_PIN) == 0&&GPIO_ReadInputDataBit(GPIOB, COL1_PIN) == 0) ;
        Delay_ms(20); // Debounce delay
        KeyNum = 4; // Set key code to 1
    }
		if(GPIO_ReadInputDataBit(GPIOB, ROW2_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL2_PIN) == 0) // 使用RESET代替逻辑非操作符!
    {
        Delay_ms(20); // Debounce delay
   
        while(GPIO_ReadInputDataBit(GPIOB, ROW2_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL2_PIN) == 0) ;
        Delay_ms(20); // Debounce delay
        KeyNum = 5; // Set key code to 1
    }
		if(GPIO_ReadInputDataBit(GPIOB, ROW2_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL3_PIN) == 0) // 使用RESET代替逻辑非操作符!
    {
        Delay_ms(20); // Debounce delay
   
        while(GPIO_ReadInputDataBit(GPIOB, ROW2_PIN) == 0&&GPIO_ReadInputDataBit(GPIOA, COL3_PIN) == 0) ;
        Delay_ms(20); // Debounce delay
        KeyNum = 6; // Set key code to 1
    }
		GPIO_WriteBit(GPIOB,GPIO_Pin_1,1);

    return KeyNum;
}


