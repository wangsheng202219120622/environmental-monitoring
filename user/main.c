#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "dht11.h"
#include "FMQ.h"
#include "Serial.h"
#include "bsp_led.h"

#include "Servo.h"
#include "paff.h"
#include "MatrixKeyboard.h"
#include "LightSensor.h"
#include "Buzzer.h"

/**

//  xt 界面切换  qxj 键盘+rgb+光敏   ws 蜂鸣器+温湿度+舵机
   * @brief  
	 矩阵键盘 R1(B1) R2(B10)  C1(b11) C2(a7) C3(a6) 
	 蜂鸣器 b0 + -
	 rgb b12 b13 b14
	 温湿度 a11 + -
	 光敏 DO(C15) + -
	 oled b6(-) b7(+) b8 b9
	 舵机 a1 - 5.5v
 	 stlink swclk swdio gnd 3.3v
   * @param
   * @retval 
   */



void Show();
/*蜂鸣器signal：B0
温湿度signal：A11
*/

//温湿度
//float leng;
u8 temp,humi;

//舵机
uint8_t KeyNum;			//定义用于接收键码的变量
float Angle;			//定义角度变量
int signal=0; //光敏开启判断位
int main(void)
{
		/*舵机模块初始化*/
	//	OLED_Init();		//OLED初始化
		Servo_Init();		//舵机初始化
		Key_Init();			//按键初始化
		MODE=0;
	//温湿度初始化
	 uint32_t bufe[5];//bufe[0]:温度 bufe[1]:湿度
	 OLED_Init();
	 DHT11_Init();
	 mfq_Init();
	 Serial_Init();
	Buzzer_Init();
	LightSensor_Init();		//光敏传感器初始化
 
	MODE=0;         //显示模式
	

	while(1)
	{
		
		//===============温湿度==================
		DHT11_Read_Data(&temp,&humi);
	  bufe[0]=temp;
		bufe[1]=humi;

		fmq(temp,humi);

		printf("temp=%d  , humi=%d RH\r\n",temp,humi);
		//=====================================
		//===============舵机================
		
		KeyNum = Key_Check();	//获取按键键码
		
		OLED_ShowString(10,48,"light:",OLED_8X16);
		if(KeyNum==5)
		{
			if(signal==1)signal=0;
			else signal=1;
		}
		if(!signal)
		{
			OLED_ShowString(60,48,"OFF",OLED_8X16);
			if (KeyNum == 6 )				//按键6按下
			{
				Angle += 30;				//角度变量自增30
				if (Angle > 180)			//角度变量超过180后
				{
					Angle = 0;				//角度变量归零
				}
			}
			Servo_SetAngle(Angle);			//设置舵机的角度为角度变量
	  }
		else
		{
			OLED_ShowString(60,48,"ON ",OLED_8X16);
			if (KeyNum == 6 ||LightSensor_Get() == 1)				//按键6按下
			{
				Angle += 30;				//角度变量自增30
				if (Angle > 180)			//角度变量超过180后
				{
					Angle = 0;				//角度变量归零
				}
			}
			Servo_SetAngle(Angle);			//设置舵机的角度为角度变量
		}
		
		OLED_ShowNum(60, 32, Angle, 3,OLED_8X16);	//OLED显示角度变量
		//================================================
		
		//		//选择模式/显示模式，才直接修改MODE模式
		if(MODE==0||MODE==4){
		
		if(KeyNum==1)
		{
			if(MODE==0)
			{
				KeyNum=0;
				KeyNumber=0;
				MODE=4;
				Flag=0;
				PaffShow();
			}
		}
		
		if(KeyNum==2)
		{
			if(MODE==4)
			{
				MODE=0;
				Show();
				OLED_ShowChar(0,0,' ',OLED_8X16);
				OLED_ShowChar(0,16,' ',OLED_8X16);
		
			}
		}
	}
		
		switch(MODE)
		{
			case 0:
			//	Paff0Set();
			Show(temp,humi);
			break;         //显示模式
			case 1:OLED_Clear();KeyNumber=KeyNum;Paff0Set();break;     //温度设置模式
			case 2:OLED_Clear();KeyNumber=KeyNum;Paff1Set();break;     //湿度设置模式
			case 4:KeyNumber=KeyNum;Show(temp,humi);PaffShow();break;     //选择模式
		}
		
		
	}
}
void Show(uint32_t temp,uint32_t humi)
{

	OLED_ShowChinese(10, 0, "温度：");
	OLED_ShowChinese(94, 0, "℃");
	OLED_ShowChinese(10, 16, "湿度：");
	OLED_ShowString(94,16,"RH",OLED_8X16);
	 /*舵机显示静态字符串*/
	OLED_ShowString(10, 32, "Angle:",OLED_8X16);	//1行1列显示字符串Angle:
	
	OLED_ShowNum(47,0,temp,2,OLED_8X16);
	OLED_ShowNum(47,16,humi,2,OLED_8X16);
	OLED_Update();
}
