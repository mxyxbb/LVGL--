/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LPMODE_H__
#define __LPMODE_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "st7735/st7735.h"

// STOPģʽ����, ����ʱ��
void module_pwr_stop_mode_wake(void);
// ����STOPģʽ,���ⲿ�жϻ���
void module_pwr_enter_stop_mode(void);

#endif /* __LPMODE_H__ */