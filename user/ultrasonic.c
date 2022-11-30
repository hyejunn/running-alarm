#include "ultrasonic.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"



/**
*       made by jyp
 *   How to use this driver:
 *    1. Implement EnableHCSR04PeriphClock function and turn on clock for used peripherals
 *       ex:
 *       void EnableHCSR04PeriphClock() {
 *         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
 *         RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
 *         RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 *      }
 *   2. Call InitHCSR04();
 *   3. Get distance (in mm) using HCSR04GetDistance function.
 *      Value lower than zero means no echo received (distance >3m)
 *      Measuring takes about 65ms
 *
 *   If necessary, change defines above, but be careful, US_ECHO_PIN must be tim ch1
 *   and US_TRIG_PIN must be ch3 or ch4. In case of ch4 change TIM_OC3Init into TIM_OC4Init in stm32f10x_HC-SR04.c file
 */



/**
 * Implement this function. See instruction at the top of this file.
 */

void EnableHCSR04PeriphClock() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  }

/**
 * Initialization of HCSR04's peripherals
 */


/**
 * Measure distance and get value in mm. Lower than 0 means no echo signal: distance more than ~3m.
 */



static void initMeasureTimer() {
   RCC_ClocksTypeDef RCC_ClocksStatus;
   RCC_GetClocksFreq(&RCC_ClocksStatus);
   uint16_t prescaler = RCC_ClocksStatus.SYSCLK_Frequency / 1000000 - 1; //1 tick = 1us (1 tick = 0.165mm resolution)

   TIM_DeInit(US_TIMER);
   TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
   TIM_TimeBaseInitStruct.TIM_Prescaler = prescaler;
   TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;
   TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseInit(US_TIMER, &TIM_TimeBaseInitStruct);

   TIM_OCInitTypeDef TIM_OCInitStruct;
   TIM_OCStructInit(&TIM_OCInitStruct);
   TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
   TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStruct.TIM_Pulse = 15; //us
   TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
   TIM_OC3Init(US_TIMER, &TIM_OCInitStruct);

   TIM_ICInitTypeDef TIM_ICInitStruct;
   TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
   TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
   TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
   TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
   TIM_ICInitStruct.TIM_ICFilter = 0;

   TIM_PWMIConfig(US_TIMER, &TIM_ICInitStruct);
   TIM_SelectInputTrigger(US_TIMER, US_TIMER_TRIG_SOURCE);
   TIM_SelectMasterSlaveMode(US_TIMER, TIM_MasterSlaveMode_Enable);

   TIM_CtrlPWMOutputs(US_TIMER, ENABLE);

   TIM_ClearFlag(US_TIMER, TIM_FLAG_Update);
}

static void initPins() {
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = US_TRIG_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(US_TRIG_PORT, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = US_ECHO_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(US_ECHO_PORT, &GPIO_InitStructure);
}

void InitHCSR04() {
   EnableHCSR04PeriphClock();
   initPins();
   initMeasureTimer();
}

int32_t HCSR04GetDistance() {
   (US_TIMER)->CNT = 0;
   TIM_Cmd(US_TIMER, ENABLE);
   while(!TIM_GetFlagStatus(US_TIMER, TIM_FLAG_Update));
   TIM_Cmd(US_TIMER, DISABLE);
   TIM_ClearFlag(US_TIMER, TIM_FLAG_Update);
   return (TIM_GetCapture2(US_TIMER)-TIM_GetCapture1(US_TIMER))*165/1000;
}