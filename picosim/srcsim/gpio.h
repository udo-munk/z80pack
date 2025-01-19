/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 */

#ifndef GPIO_INC
#define GPIO_INC

/*
 * Additional to the GPIO pins defined here the code uses the
 * default UART defined in the board definition files. For
 * Pico boards this is UART0 with pins 0 and 1.
 */

#define SWITCH_BREAK 15	/* pin with switch we use to interrupt the system */
#define WS2812_PIN 14	/* pin with RGB LED */

#define DS3231_I2C_PORT i2c0	/* I2C instance used for DS3231 RTC */
#define DS3231_I2C_SDA_PIN 20	/* I2C data pin used for DS3231 RTC */
#define DS3231_I2C_SCL_PIN 21	/* I2C clock pin used for DS3231 RTC */

#define SD_SPI_PORT spi0	/* SPI instance used for SD card */
#define SD_SPI_CLK 18		/* SPI clk pin used for SD card */
#define SD_SPI_SI 19		/* SPI si pin used for SD card */
#define SD_SPI_SO 16		/* SPI so pin used for SD card */
#define SD_SPI_CS 17		/* SPI cs pin used for SD card */

#endif /* !GPIO_INC */
