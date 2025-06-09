#ifndef __PAFF_H__
#define __PAFF_H__

extern unsigned char KeyNumber,MODE,SetSelect,SetFlashFlag,Flag;
extern  char paff0[],paff1[];
char* Read_Paff0(void);
char* Read_Paff1(void);

void PaffShow(void);
void Paff0Set(void);
void Paff1Set(void);


#endif
