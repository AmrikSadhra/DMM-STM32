#include "switches.h"

//Global for storing menu position
uint8_t localMenuPosition = 1;
uint8_t buttonState = 1;

void switch_init() {
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitTypeDef GPIOB_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    //Initialise Blue User button (A0)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);    //Enable clock for GPOIA
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //Initialise GPIOE
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14 |
                               GPIO_Pin_15;  // we want to configure PE8-15
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;       //As inputs
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;        //Polled at 2Mhz (Humans aren't quite fast enough to justify more)
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;    //As push / pull
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;    //With a pulldown resistor, as detecting a high level
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    //initialise GPIOB, for switching between different circuits
    GPIOB_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_15;
    GPIOB_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIOB_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;    //Polled at 2Mhz (Humans aren't quite fast enough to justify more)
    GPIOB_InitStruct.GPIO_OType = GPIO_OType_PP;    //As push / pull
    GPIOB_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;    //With a pulldown resistor, as detecting a high level
    GPIO_Init(GPIOB, &GPIOB_InitStruct);


    /* Tell system that you will use PDE8-15 to trigger interrupts */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource8);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource10);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource11);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource12);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource13);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource14);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource15);
    //And PA0 for blue button
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    /* Port E's buttons from 8 through 15 all lie on different EXTI lines*/
    EXTI_InitStruct.EXTI_Line =
            EXTI_Line8 | EXTI_Line9 | EXTI_Line10 | EXTI_Line11 | EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);

    /* Port A's button on line 0*/
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    /* Add IRQ vector to NVIC */
    /* PE8-15 are connected to EXTI_Line8-15, which have EXTI9_5_IRQn and EXTI15_10_IRQn */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = SWITCH_IQ_PRIORITY;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = SWITCH_IQ_SUBPRIORITY;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    /* Add to NVIC Again */
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    /* Add to NVIC Again again (blue button) */
    NVIC_Init(&NVIC_InitStruct);
}


void EXTI0_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        buttonState = !buttonState; //Toggle button state
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
#ifdef SWITCH_DEBUG
    printf("[Blue Button External Interrupt] Toggling button state to %d\r\n", buttonState);
#endif
}

void EXTI9_5_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
        localMenuPosition = 1;
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line8);
    } else if (EXTI_GetITStatus(EXTI_Line9) != RESET) {
        localMenuPosition = 2;
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
#ifdef SWITCH_DEBUG
    printf("[Switch External Interrupt] Menu Position: %d\r\n", localMenuPosition);
#endif
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
        localMenuPosition = 3;
        EXTI_ClearITPendingBit(EXTI_Line10);
    } else if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
        localMenuPosition = 4;
        EXTI_ClearITPendingBit(EXTI_Line11);
    } else if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
        localMenuPosition = 5;
        EXTI_ClearITPendingBit(EXTI_Line12);
    } else if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        localMenuPosition = 6;
        EXTI_ClearITPendingBit(EXTI_Line13);
    } else if (EXTI_GetITStatus(EXTI_Line14) != RESET) {
        localMenuPosition = 7;
        EXTI_ClearITPendingBit(EXTI_Line14);
    } else if (EXTI_GetITStatus(EXTI_Line15) != RESET) {
        localMenuPosition = 8;
        EXTI_ClearITPendingBit(EXTI_Line15);
    }

#ifdef SWITCH_DEBUG
    printf("[Switch External Interrupt] Menu Position: %d\r\n", localMenuPosition);
#endif
}
