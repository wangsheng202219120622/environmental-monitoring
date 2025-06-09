#include "paff.h"
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
//修改最高最低温度，是否开启LED/蜂鸣器  MODE 0关闭 1开启

//KeyNumber按键数字，MODE模式，SetSelect修改数据，SetFlashFlag闪烁标志,Flag 0表示进入温度设置，1表示进入湿度设置
unsigned char KeyNumber,MODE,SetSelect,SetFlashFlag,Flag;
//paff0，paff1分别表示温度，湿度。其中第一位表示最低标准，第二位表示最大标准，第三位表示是否开启其控件
 char paff0[]={10,20,0},paff1[]={20,80,0};

/**
  * @brief  返回paff0的值
  * @param  无
  * @retval paff0的值
  */
char* Read_Paff0(void)
{
	return paff0;
}

/**
  * @brief  返回paff1的值
  * @param  无
  * @retval paff1的值
  */
char* Read_Paff1(void)
{
	return paff1;
}

/**
  * @brief  显示现有参数
  * @param  无
  * @retval 无
  */
void PaffShow(void)        	//主屏幕
{

	//前进一位
	if(KeyNumber==3)
	{
		Flag++;
		Flag%=2;		//越界清零
	}
	
	//确定进入该设置,如果Flag为0，则进入1模式，为1则进入2模式
	if(KeyNumber==1)
	{
		if(Flag==0)
			MODE=1;
		//
		else if(Flag==1)
			MODE=2;
		
	}
	//如果Flag为0，选择温度，否则为湿度
	if(!Flag)
	{
		OLED_ShowChar(0,0,'>',OLED_8X16);
		OLED_ShowChar(0,16,' ',OLED_8X16);
	}else
	{
		OLED_ShowChar(0,0,' ',OLED_8X16);
		OLED_ShowChar(0,16,'>',OLED_8X16);
	}
}


/**
  * @brief  设置参数的函数，点击k1为向后一位选择，点击k2为向前一位选择，点击k3为该位加1，点击k4为该位减1
  * @param  无
  * @retval 无
  */
void Paff0Set(void)        	//温度设置
{
	static unsigned char flash_counter = 0; // 用于控制闪烁的静态变量
	
	if(KeyNumber==2)   //下移
	{
		SetSelect++;
		SetSelect%=3;		//越界清零
	}
	
	if(KeyNumber==3)  //加
	{
		paff0[SetSelect]++;
		if(paff0[0]>=paff0[1]){paff0[0]=paff0[1]-1;}
		if(paff0[1]>120){paff0[1]=120;}
		if(paff0[2]>1){paff0[2]=0;}
	}
	
	if(KeyNumber==4) //减
	{
		paff0[SetSelect]--;
		if(paff0[0]<-10){paff0[0]=-10;}
		if(paff0[1]<=paff0[0]){paff0[2]=paff0[0]+1;}
		if(paff0[2]>125){paff0[2]=1;}

	}
	//闪烁与显示。。。。。。
	
    // 根据SetFlashFlag决定是否显示
        OLED_ShowString(0,0,"Min_T:",OLED_8X16);
        OLED_ShowString(0,16,"Max_T:",OLED_8X16);
        OLED_ShowString(0,32,"LED:",OLED_8X16);
        OLED_ShowNum(50,0,paff0[0],3,OLED_8X16);
        OLED_ShowNum(50,16,paff0[1],3,OLED_8X16);
        if(paff0[2] == 0) {
            OLED_ShowString(40,32,"OFF",OLED_8X16);
					
        } else if(paff0[2] == 1) {
            OLED_ShowString(40,32,"ON",OLED_8X16);
        }
				
		// 闪烁逻辑
    flash_counter++;
    if (flash_counter >= 7) { // 假设每10次调用刷新一次显示，大约0.5秒闪烁一次
        flash_counter = 0;
        SetFlashFlag = !SetFlashFlag; // 切换闪烁标志
    }
    if(SetFlashFlag) 
		{
        // 清除需要闪烁的部分
			if(SetSelect==0)
        OLED_ClearArea(50,0,128,16); // 清除第一行
			else if(SetSelect==1)
        OLED_ClearArea(50,16,128,16); // 清除第二行
			else if(SetSelect==2)
        OLED_ClearArea(30,32,128,16); // 清除第三行
    }
    OLED_Update();
	

	
	
	if(KeyNumber==1)
	{
		//切换显示模式，模式0
		MODE=0;
		OLED_Clear();
	}

}

void Paff1Set(void)        	//湿度设置
{
	static unsigned char flash_counter = 0; // 用于控制闪烁的静态变量
	
	if(KeyNumber==2)   //下移
	{
		SetSelect++;
		SetSelect%=3;		//越界清零
	}
	
	if(KeyNumber==3)  //加
	{
		paff1[SetSelect]++;
		if(paff1[0]>=paff1[1]){paff1[0]=paff1[1]-1;}
		if(paff1[1]>120){paff1[1]=120;}
		if(paff1[2]>1){paff1[2]=0;}
		
	}
	
	if(KeyNumber==4) //减
	{
		paff1[SetSelect]--;
		if(paff1[0]<-10){paff1[0]=-10;}
		if(paff1[1]<=paff1[0]){paff1[1]=paff1[0]+1;}
		if(paff1[2]>125){paff1[2]=1;}

	}
	//闪烁与显示。。。。。。

		
	OLED_ShowString(0,0,"Min_H:",OLED_8X16);
	OLED_ShowString(0,16,"Max_H:",OLED_8X16);
	OLED_ShowString(0,32,"BUZ:",OLED_8X16);
	OLED_ShowNum(50,0,paff1[0],3,OLED_8X16);
	OLED_ShowNum(50,16,paff1[1],3,OLED_8X16);
	if(paff1[2]==0){OLED_ShowString(40,32,"OFF",OLED_8X16);}
	else if(paff1[2]==1){OLED_ShowString(40,32,"ON",OLED_8X16);}
	
	//闪烁逻辑
	flash_counter++;
	if (flash_counter >= 7) { // 假设每10次调用刷新一次显示，大约0.5秒闪烁一次
			flash_counter = 0;
			SetFlashFlag = !SetFlashFlag; // 切换闪烁标志
	}
	
	if(SetFlashFlag) 
	{
  // 清除需要闪烁的部分
		if(SetSelect==0)
			OLED_ClearArea(50,0,128,16); // 清除第一行
		else if(SetSelect==1)
			OLED_ClearArea(50,16,128,16); // 清除第二行
		else if(SetSelect==2)
			OLED_ClearArea(30,32,128,16); // 清除第三行
  }
	OLED_Update();
	
	if(KeyNumber==1)
	{
		//切换显示模式，模式0
		//OLED_ClearArea(0,0,128,64);
		MODE=0;
		OLED_Clear();
	}

	
}
