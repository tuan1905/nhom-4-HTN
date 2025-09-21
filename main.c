#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_usart.h"
#include <string.h>

/*==== OLED ST7735 pin ====*/
#define OLED_CS_HIGH()   GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define OLED_CS_LOW()    GPIO_ResetBits(GPIOB, GPIO_Pin_12)

#define OLED_DC_HIGH()   GPIO_SetBits(GPIOB, GPIO_Pin_14)
#define OLED_DC_LOW()    GPIO_ResetBits(GPIOB, GPIO_Pin_14)

#define OLED_RST_HIGH()  GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define OLED_RST_LOW()   GPIO_ResetBits(GPIOA, GPIO_Pin_8)

/*==== SPI Init ====*/
void SPI_Config(void) {
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    // PB13 = SCK, PB15 = MOSI
    gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &gpio);

    // PB12 = CS, PB14 = DC
    gpio.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_14;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &gpio);

    // PA8 = RST
    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &gpio);

    // SPI2 config
    spi.SPI_Direction = SPI_Direction_1Line_Tx;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // ~9MHz
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &spi);

    SPI_Cmd(SPI2, ENABLE);

    OLED_CS_HIGH();
}

/*==== SPI Write ====*/
void SPI_WriteByte(uint8_t data) {
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, data);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
}

/*==== OLED Functions ====*/
void OLED_WriteCommand(uint8_t cmd) {
    OLED_CS_LOW();
    OLED_DC_LOW();
    SPI_WriteByte(cmd);
    OLED_CS_HIGH();
}

void OLED_WriteData(uint8_t data) {
    OLED_CS_LOW();
    OLED_DC_HIGH();
    SPI_WriteByte(data);
    OLED_CS_HIGH();
}

void OLED_Reset(void) {
    OLED_RST_LOW();
    for (volatile int i = 0; i < 50000; i++);
    OLED_RST_HIGH();
    for (volatile int i = 0; i < 50000; i++);
}

/*==== ST7735 Init Sequence (simplified) ====*/
void OLED_Init(void) {
    OLED_Reset();

    OLED_WriteCommand(0x01); // Software reset
    for (volatile int i = 0; i < 500000; i++);

    OLED_WriteCommand(0x11); // Sleep out
    for (volatile int i = 0; i < 500000; i++);

    OLED_WriteCommand(0x29); // Display ON
}

/*==== Fill Screen ====*/
void OLED_Fill(uint16_t color) {
    uint16_t i, j;

    OLED_WriteCommand(0x2A); // Column addr set
    OLED_WriteData(0);
    OLED_WriteData(0);
    OLED_WriteData(0);
    OLED_WriteData(127);

    OLED_WriteCommand(0x2B); // Row addr set
    OLED_WriteData(0);
    OLED_WriteData(0);
    OLED_WriteData(0);
    OLED_WriteData(159);

    OLED_WriteCommand(0x2C); // Memory write

    for (i = 0; i < 128; i++) {
        for (j = 0; j < 160; j++) {
            OLED_WriteData(color >> 8);
            OLED_WriteData(color & 0xFF);
        }
    }
}

/*==== Main ====*/
int main(void) {
    SPI_Config();
    OLED_Init();

    OLED_Fill(0xF800); // Fill Red
    for (volatile int d = 0; d < 2000000; d++);

    OLED_Fill(0x07E0); // Fill Green
    for (volatile int d = 0; d < 2000000; d++);

    OLED_Fill(0x001F); // Fill Blue

    while (1) {
    }
}
