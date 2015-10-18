#include <stdint.h>
#include <stdbool.h>

#include STM32_HAL_H


#define LED_GREEN_PIN                       GPIO_PIN_2
#define LED_GREEN_GPIO_PORT                 GPIOA
#define LED_GREEN_GPIO_CLK_ENABLE()         __GPIOA_CLK_ENABLE()
#define LED_GREEN_GPIO_CLK_DISABLE()        __GPIOA_CLK_DISABLE()

#define LED_RED_PIN                         GPIO_PIN_7
#define LED_RED_GPIO_PORT                   GPIOB
#define LED_RED_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()
#define LED_RED_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()

#define POWER_ON_PIN                         GPIO_PIN_12
#define POWER_ON_GPIO_PORT                   GPIOB
#define POWER_ON_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()
#define POWER_ON_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()

#define USB_DPLUS_PIN                         GPIO_PIN_3
#define USB_DPLUS_GPIO_PORT                   GPIOB
#define USB_DPLUS_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()
#define USB_DPLUS_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()


#define DEBUG_PIN                         GPIO_PIN_2
#define DEBUG_GPIO_PORT                   GPIOA
#define DEBUG_GPIO_CLK_ENABLE()           __GPIOA_CLK_ENABLE()
#define DEBUG_GPIO_CLK_DISABLE()          __GPIOA_CLK_DISABLE()


void init_power_on_pin(void);
void init_debug_pin(void);
void toogle_debug(void);

void REGULUS_board_early_init(void) {
    init_power_on_pin();
    init_debug_pin();
    toogle_debug();
}

void init_usb_dplus_pin(void) {
    GPIO_InitTypeDef  gpioinitstruct = {0};
    /* Enable the GPIO Clock */
    USB_DPLUS_GPIO_CLK_ENABLE();

    /* Configure the USB D+ pin */
    gpioinitstruct.Pin    = USB_DPLUS_PIN;
    gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull   = GPIO_NOPULL;
    gpioinitstruct.Speed  = GPIO_SPEED_LOW;
    HAL_GPIO_Init(USB_DPLUS_GPIO_PORT, &gpioinitstruct);

    // enable pullup - usb master will poll no
    HAL_GPIO_WritePin(USB_DPLUS_GPIO_PORT, USB_DPLUS_PIN, GPIO_PIN_SET);
}

void init_debug_pin(void) {
    GPIO_InitTypeDef  gpioinitstruct = {0};
    /* Enable the GPIO Clock */
    DEBUG_GPIO_CLK_ENABLE();

    /* Configure the USB D+ pin */
    gpioinitstruct.Pin    = DEBUG_PIN;
    gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull   = GPIO_NOPULL;
    gpioinitstruct.Speed  = GPIO_SPEED_LOW;
    HAL_GPIO_Init(DEBUG_GPIO_PORT, &gpioinitstruct);

    // enable pullup - usb master will poll no
}

void toogle_debug(void) {
    HAL_GPIO_WritePin(DEBUG_GPIO_PORT, DEBUG_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DEBUG_GPIO_PORT, DEBUG_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DEBUG_GPIO_PORT, DEBUG_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DEBUG_GPIO_PORT, DEBUG_PIN, GPIO_PIN_RESET);

}

void init_power_on_pin(void) {
    GPIO_InitTypeDef  gpioinitstruct = {0};
    /* Enable the GPIO Clock */
    POWER_ON_GPIO_CLK_ENABLE();

    /* Configure the POWER_ON pin */
    gpioinitstruct.Pin    = POWER_ON_PIN;
    gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull   = GPIO_NOPULL;
    gpioinitstruct.Speed  = GPIO_SPEED_LOW;
    HAL_GPIO_Init(POWER_ON_GPIO_PORT, &gpioinitstruct);

    // enable power on pin (pullup)
    HAL_GPIO_WritePin(POWER_ON_GPIO_PORT, POWER_ON_PIN, GPIO_PIN_SET);
}

/********************************************************************/
// UART


// assumes Init parameters have been set up correctly
// bool uart_init2(pyb_uart_obj_t *uart_obj) {
//     GPIO_InitTypeDef  GPIO_InitStruct = {0};
//
//     /*##-1- Enable peripherals and GPIO Clocks #################################*/
//     /* Enable GPIO TX/RX clock */
//     USARTx_TX_GPIO_CLK_ENABLE();
//     USARTx_RX_GPIO_CLK_ENABLE();
//
//     /* Enable USARTx clock */
//     USARTx_CLK_ENABLE();
//
//     /*##-2- Configure peripheral GPIO ##########################################*/
//     /* UART TX GPIO pin configuration  */
//     GPIO_InitStruct.Pin       = USARTx_TX_PIN;
//     GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull      = GPIO_PULLUP;
//     GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
//     GPIO_InitStruct.Alternate = USARTx_TX_AF;
//
//     HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
//
//     /* UART RX GPIO pin configuration  */
//     GPIO_InitStruct.Pin = USARTx_RX_PIN;
//     GPIO_InitStruct.Alternate = USARTx_RX_AF;
//
//     HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
//
//     uart_obj->uart.Instance        = USARTx;
//     uart_obj->uart.Init.BaudRate   = 115200;
//     uart_obj->uart.Init.WordLength = UART_WORDLENGTH_9B; // UART_WORDLENGTH_9B?
//     uart_obj->uart.Init.StopBits   = UART_STOPBITS_1;
//     uart_obj->uart.Init.Parity     = UART_PARITY_NONE;
//     uart_obj->uart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
//     uart_obj->uart.Init.Mode       = UART_MODE_TX;
//     // UartHandle.Init.Mode       = UART_MODE_TX_RX;
//
//     if (HAL_UART_Init(&(uart_obj->uart)) != HAL_OK) {
//         init_debug_pin();
//         init_debug_pin();
//     }
//
//     // init UARTx
//     uart_obj->uart_id = 2;
//     uart_obj->is_enabled = true;
//     return true;
// }
//
//
// bool uart_init(pyb_uart_obj_t *uart_obj, uint32_t baudrate) {
//     return uart_init2(uart_obj);
// }
//
// bool uart_rx_any(pyb_uart_obj_t *uart_obj) {
//     return __HAL_UART_GET_FLAG(&(uart_obj->uart), UART_FLAG_RXNE);
// }
//
//
// int uart_rx_char(pyb_uart_obj_t *uart_obj) {
//     uint8_t ch;
//     if (HAL_UART_Receive(&(uart_obj->uart), &ch, 1, 0) != HAL_OK) {
//         ch = 0;
//     }
//     return ch;
// }
//
// void uart_tx_char(pyb_uart_obj_t *uart_obj, int c) {
//     uint8_t ch = c;
//     HAL_UART_Transmit(&(uart_obj->uart), &ch, 1, 100000);
// }
