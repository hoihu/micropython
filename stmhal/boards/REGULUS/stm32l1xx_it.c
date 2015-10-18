/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Src/stm32l1xx_it.c
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    3-July-2015
  * @brief   Main Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
// #include "stm32l1xx_it.h"

#include STM32_HAL_H

#include "py/obj.h"
#include "pendsv.h"
#include "extint.h"
#include "timer.h"
// #include "uart.h"
// #include "storage.h"
// #include "can.h"

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

// Set the following to 1 to get some more information on the Hard Fault
// More information about decoding the fault registers can be found here:
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0646a/Cihdjcfc.html
#define REPORT_HARD_FAULT_REGS  0

#if REPORT_HARD_FAULT_REGS

#include "mphal.h"

char *fmt_hex(uint32_t val, char *buf) {
    const char *hexDig = "0123456789abcdef";

    buf[0] = hexDig[(val >> 28) & 0x0f];
    buf[1] = hexDig[(val >> 24) & 0x0f];
    buf[2] = hexDig[(val >> 20) & 0x0f];
    buf[3] = hexDig[(val >> 16) & 0x0f];
    buf[4] = hexDig[(val >> 12) & 0x0f];
    buf[5] = hexDig[(val >>  8) & 0x0f];
    buf[6] = hexDig[(val >>  4) & 0x0f];
    buf[7] = hexDig[(val >>  0) & 0x0f];
    buf[8] = '\0';

    return buf;
}

void print_reg(const char *label, uint32_t val) {
    char hexStr[9];

    mp_hal_stdout_tx_str(label);
    mp_hal_stdout_tx_str(fmt_hex(val, hexStr));
    mp_hal_stdout_tx_str("\r\n");
}
#endif // REPORT_HARD_FAULT_REGS

extern void __fatal_error(const char*);
extern PCD_HandleTypeDef pcd_handle;

/******************************************************************************/
/*             Cortex-M3 Processor Exceptions Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    #if REPORT_HARD_FAULT_REGS
        uint32_t cfsr = SCB->CFSR;

        print_reg("HFSR  ", SCB->HFSR);
        print_reg("CFSR  ", cfsr);
        if (cfsr & 0x80) {
            print_reg("MMFAR ", SCB->MMFAR);
        }
        if (cfsr & 0x8000) {
            print_reg("BFAR  ", SCB->BFAR);
        }
    #endif // REPORT_HARD_FAULT_REGS
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
        __fatal_error("HardFault");
    }

}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
        __fatal_error("MemManage");
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
        __fatal_error("BusFault");
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
        __fatal_error("UsageFault");
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void) {
    pendsv_isr_handler();
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    // Instead of calling HAL_IncTick we do the increment here of the counter.
    // This is purely for efficiency, since SysTick is called 1000 times per
    // second at the highest interrupt priority.
    extern __IO uint32_t uwTick;
    uwTick += 1;

    // Read the systick control regster. This has the side effect of clearing
    // the COUNTFLAG bit, which makes the logic in sys_tick_get_microseconds
    // work properly.
    SysTick->CTRL;
}

/******************************************************************************/
/*                 STM32L1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l1xx.s).                                               */
/******************************************************************************/

/* possible handlers are (from startup.s file copied):
.word Reset_Handler
.word NMI_Handler
.word HardFault_Handler
.word MemManage_Handler
.word BusFault_Handler
.word UsageFault_Handler
.word SVC_Handler
.word DebugMon_Handler
.word PendSV_Handler
.word SysTick_Handler
.word WWDG_IRQHandler
.word PVD_IRQHandler
.word TAMPER_STAMP_IRQHandler
.word RTC_WKUP_IRQHandler
.word FLASH_IRQHandler
.word RCC_IRQHandler
.word EXTI0_IRQHandler
.word EXTI1_IRQHandler
.word EXTI2_IRQHandler
.word EXTI3_IRQHandler
.word EXTI4_IRQHandler
.word DMA1_Channel1_IRQHandler
.word DMA1_Channel2_IRQHandler
.word DMA1_Channel3_IRQHandler
.word DMA1_Channel4_IRQHandler
.word DMA1_Channel5_IRQHandler
.word DMA1_Channel6_IRQHandler
.word DMA1_Channel7_IRQHandler
.word ADC1_IRQHandler
.word USB_HP_IRQHandler
.word USB_LP_IRQHandler
.word DAC_IRQHandler
.word COMP_IRQHandler
.word EXTI9_5_IRQHandler
.word TIM9_IRQHandler
.word TIM10_IRQHandler
.word TIM11_IRQHandler
.word TIM2_IRQHandler
.word TIM3_IRQHandler
.word TIM4_IRQHandler
.word I2C1_EV_IRQHandler
.word I2C1_ER_IRQHandler
.word I2C2_EV_IRQHandler
.word I2C2_ER_IRQHandler
.word SPI1_IRQHandler
.word SPI2_IRQHandler
.word USART1_IRQHandler
.word USART2_IRQHandler
.word USART3_IRQHandler
.word EXTI15_10_IRQHandler
.word RTC_Alarm_IRQHandler
.word USB_FS_WKUP_IRQHandler
.word TIM6_IRQHandler
.word TIM7_IRQHandler
.word TIM5_IRQHandler
.word SPI3_IRQHandler
.word DMA2_Channel1_IRQHandler
.word DMA2_Channel2_IRQHandler
.word DMA2_Channel3_IRQHandler
.word DMA2_Channel4_IRQHandler
.word DMA2_Channel5_IRQHandler
.word COMP_ACQ_IRQHandler
*/


/**
  * @brief  This function handles USB-On-The-Go global interrupt request.
  * @param  None
  * @retval None
  */
void USB_LP_IRQHandler(void) {
  HAL_PCD_IRQHandler(&pcd_handle);
}

void USB_FS_WKUP_IRQHandler(void) {
    if ((&pcd_handle)->Init.low_power_enable) {
      /* Reset SLEEPDEEP bit of Cortex System Control Register */
      SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));

      /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
      PLL as system clock source (HSE and PLL are disabled in STOP mode) */

      __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);

      /* Wait till HSE is ready */
      while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET)
      {}

      /* Enable the main PLL. */
      __HAL_RCC_PLL_ENABLE();

      /* Wait till PLL is ready */
      while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET)
      {}

      /* Select PLL as SYSCLK */
      MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_SYSCLKSOURCE_PLLCLK);

      while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_CFGR_SWS_PLL)
      {}

      /* ungate PHY clock */
    //    __HAL_PCD_UNGATE_PHYCLOCK((&pcd_handle));
    }
    /* Clear EXTI pending Bit*/
    __HAL_USB_WAKEUP_EXTI_CLEAR_FLAG();
}
extern void toogle_debug();

void TIM3_IRQHandler(void) {
    toogle_debug();
    HAL_TIM_IRQHandler(&TIM3_Handle);
}

/**
  * @brief  These functions handle the EXTI interrupt requests.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void) {
    Handle_EXTI_Irq(0);
}

void EXTI1_IRQHandler(void) {
    Handle_EXTI_Irq(1);
}

void EXTI2_IRQHandler(void) {
    Handle_EXTI_Irq(2);
}

void EXTI3_IRQHandler(void) {
    Handle_EXTI_Irq(3);
}

void EXTI4_IRQHandler(void) {
    Handle_EXTI_Irq(4);
}

void PVD_IRQHandler(void) {
    Handle_EXTI_Irq(EXTI_PVD_OUTPUT);
}

void RTC_Alarm_IRQHandler(void) {
    Handle_EXTI_Irq(EXTI_RTC_ALARM);
}

void TAMPER_STAMP_IRQHandler(void) {
    Handle_EXTI_Irq(EXTI_RTC_TIMESTAMP);
}

void RTC_WKUP_IRQHandler(void) {
    RTC->ISR &= ~(1 << 10); // clear wakeup interrupt flag
    Handle_EXTI_Irq(EXTI_RTC_WAKEUP); // clear EXTI flag and execute optional callback
}

// UART/USART IRQ handlers
void USART1_IRQHandler(void) {
    // uart_irq_handler(1);
}

void USART2_IRQHandler(void) {
    // uart_irq_handler(2);
}
void USART3_IRQHandler(void) {
    // uart_irq_handler(3);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
