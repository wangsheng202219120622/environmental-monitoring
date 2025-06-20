#ifndef __DHT11_H
#define __DHT11_H 
#include "sys.h"   


#define DHT11_IO_IN()  {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=8<<12;}
#define DHT11_IO_OUT() {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=3<<12;}
									   
#define	DHT11_DQ_OUT PAout(11) 
#define	DHT11_DQ_IN  PAin(11)  


u8 DHT11_Init(void);
u8 DHT11_Read_Data(u8 *temp,u8 *humi);
u8 DHT11_Read_Byte(void);
u8 DHT11_Read_Bit(void);
u8 DHT11_Check(void);
void DHT11_Rst(void); 
#endif















