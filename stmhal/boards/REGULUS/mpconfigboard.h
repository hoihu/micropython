#define MICROPY_HW_BOARD_NAME       "REGULUS"
#define MICROPY_HW_MCU_NAME         "STM32L151CC"
#define MICROPY_PY_SYS_PLATFORM     "pyboard"

#define MICROPY_HW_HAS_SWITCH       (0)
#define MICROPY_HW_HAS_SDCARD       (0)
#define MICROPY_HW_HAS_MMA7660      (0)
#define MICROPY_HW_HAS_LIS3DSH      (0)
#define MICROPY_HW_HAS_LCD          (0)
#define MICROPY_HW_ENABLE_RNG       (0)
#define MICROPY_HW_ENABLE_RTC       (1)
#define MICROPY_HW_ENABLE_TIMER     (1)
#define MICROPY_HW_ENABLE_SERVO     (0)
#define MICROPY_HW_ENABLE_DAC       (0)
#define MICROPY_HW_ENABLE_SPI1      (0)
#define MICROPY_HW_ENABLE_SPI2      (0)
#define MICROPY_HW_ENABLE_SPI3      (0)
#define MICROPY_HW_ENABLE_CAN       (0)

// HSE is 12MHz
#define MICROPY_HW_CLK_PLLM (6)
#define MICROPY_HW_CLK_PLLN (336)
#define MICROPY_HW_CLK_PLLP (RCC_PLLP_DIV2)
#define MICROPY_HW_CLK_PLLQ (7)

#define MICROPY_HW_SPI1_NSS     (pin_A1)
#define MICROPY_HW_SPI1_SCK     (pin_A2)
#define MICROPY_HW_SPI1_MISO    (pin_A5)
#define MICROPY_HW_SPI1_MOSI    (pin_A7)



// The pyboard has a 32kHz crystal for the RTC
#define MICROPY_HW_RTC_USE_LSE      (0)

// UART config
#define MICROPY_HW_UART1_NAME "XB"
// #define MICROPY_HW_UART1_PORT (GPIOB)
// #define MICROPY_HW_UART1_PINS (GPIO_PIN_6 | GPIO_PIN_7)
#define MICROPY_HW_UART2_PORT (GPIOA)
#define MICROPY_HW_UART2_PINS (GPIO_PIN_2 | GPIO_PIN_3)
#define MICROPY_HW_UART2_RTS  (GPIO_PIN_1)
#define MICROPY_HW_UART2_CTS  (GPIO_PIN_0)

// I2C busses
// #define MICROPY_HW_I2C1_NAME "X"
// #define MICROPY_HW_I2C1_SCL (pin_B6)
// #define MICROPY_HW_I2C1_SDA (pin_B7)
// #define MICROPY_HW_I2C2_NAME "Y"
// #define MICROPY_HW_I2C2_SCL (pin_B10)
// #define MICROPY_HW_I2C2_SDA (pin_B11)

// SPI busses
// #define MICROPY_HW_SPI1_NAME "X"
// #define MICROPY_HW_SPI2_NAME "Y"

// CAN busses
// #define MICROPY_HW_CAN1_NAME "YA" // CAN1 on RX,TX = Y3,Y4 = PB8,PB9
// #define MICROPY_HW_CAN2_NAME "YB" // CAN2 on RX,TX = Y5,Y6 = PB12,PB13

// USRSW has no pullup or pulldown, and pressing the switch makes the input go low
// #define MICROPY_HW_USRSW_PIN        (pin_B3)
// #define MICROPY_HW_USRSW_PULL       (GPIO_PULLUP)
// #define MICROPY_HW_USRSW_EXTI_MODE  (GPIO_MODE_IT_FALLING)
// #define MICROPY_HW_USRSW_PRESSED    (0)

// LEDs
// #define MICROPY_HW_LED1             (pin_I1) // green
#define MICROPY_HW_LED_OTYPE        (GPIO_MODE_OUTPUT_PP)
#define MICROPY_HW_LED_ON(pin)      (pin->gpio->BSRR = pin->pin_mask)
#define MICROPY_HW_LED_OFF(pin)     (pin->gpio->BSRR = (pin->pin_mask << 16))

#define USB_CDC_ONLY

// SD card detect switch
// #define MICROPY_HW_SDCARD_DETECT_PIN        (pin_A8)
// #define MICROPY_HW_SDCARD_DETECT_PULL       (GPIO_PULLUP)
// #define MICROPY_HW_SDCARD_DETECT_PRESENT    (GPIO_PIN_RESET)

// USB config
#define MICROPY_HW_USB_VBUS_DETECT_PIN (pin_A10)
// #define MICROPY_HW_USB_OTG_ID_PIN      (pin_A10)
