1. Các macro điều khiển chân
#define OLED_CS_HIGH()   GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define OLED_CS_LOW()    GPIO_ResetBits(GPIOB, GPIO_Pin_12)

#define OLED_DC_HIGH()   GPIO_SetBits(GPIOB, GPIO_Pin_14)
#define OLED_DC_LOW()    GPIO_ResetBits(GPIOB, GPIO_Pin_14)

#define OLED_RST_HIGH()  GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define OLED_RST_LOW()   GPIO_ResetBits(GPIOA, GPIO_Pin_8)


CS (Chip Select) – chọn/ bỏ chọn chip khi gửi SPI.

DC (Data/Command) – báo cho LCD biết byte tiếp theo là lệnh hay dữ liệu.

RST – chân reset phần cứng của LCD.

Các macro này giúp bật/tắt chân GPIO dễ hơn.

2. Cấu hình SPI2 & GPIO
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);


Bật xung cho GPIOA, GPIOB và SPI2.

Chân SPI:

PB13 = SCK (Clock)

PB15 = MOSI (Data ra)

Cấu hình Alternate Function Push-Pull, tốc độ 50 MHz.

Chân điều khiển LCD:

PB12 (CS), PB14 (DC) là Output Push-Pull.

PA8 (RST) là Output Push-Pull.

Cấu hình SPI2:

Master mode, 8 bit, CPOL = 0, CPHA = 1 (chế độ 0).

NSS = Soft (tự quản lý CS bằng code).

Tốc độ: Prescaler 4 (~9 MHz với 36 MHz APB1).

Gửi bit MSB trước.

Cuối cùng SPI_Cmd(SPI2, ENABLE) để bật SPI2.

3. Gửi dữ liệu SPI
void SPI_WriteByte(uint8_t data) {
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, data);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
}


Đợi TXE=1 (Data Register rỗng), gửi byte ra.

Đợi cờ BSY=0 (SPI không bận) trước khi thoát.

4. Gửi lệnh & dữ liệu tới LCD
void OLED_WriteCommand(uint8_t cmd) {
    OLED_CS_LOW();
    OLED_DC_LOW();     // 0 = command
    SPI_WriteByte(cmd);
    OLED_CS_HIGH();
}

void OLED_WriteData(uint8_t data) {
    OLED_CS_LOW();
    OLED_DC_HIGH();    // 1 = data
    SPI_WriteByte(data);
    OLED_CS_HIGH();
}


Khi gửi lệnh thì DC = 0, khi gửi dữ liệu thì DC = 1.

CS kéo xuống khi truyền, sau đó kéo lên lại.

5. Reset LCD
void OLED_Reset(void) {
    OLED_RST_LOW();
    for (volatile int i = 0; i < 50000; i++);
    OLED_RST_HIGH();
    for (volatile int i = 0; i < 50000; i++);
}


Kéo chân RST xuống 0 rồi lên 1 để reset cứng LCD.

6. Khởi tạo ST7735 (đơn giản hóa)
void OLED_Init(void) {
    OLED_Reset();

    OLED_WriteCommand(0x01); // Software reset
    delay...
    OLED_WriteCommand(0x11); // Sleep out
    delay...
    OLED_WriteCommand(0x29); // Display ON
}


Gửi vài lệnh cơ bản: reset mềm, thoát chế độ ngủ, bật hiển thị.

Thực tế ST7735 có rất nhiều lệnh khởi tạo (gamma, frame rate…). Ở đây chỉ làm tối thiểu để hiện được màu.

7. Tô màu màn hình
void OLED_Fill(uint16_t color) {
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


0x2A, 0x2B: đặt vùng hiển thị (x0..x1, y0..y1).

0x2C: bắt đầu ghi dữ liệu pixel.

Mỗi pixel ST7735 dùng 16 bit (5-6-5 RGB). Code gửi high byte trước, low byte sau.

Lặp 128×160 = 20 480 pixel.

8. Hàm main
SPI_Config();
OLED_Init();

OLED_Fill(0xF800); // Red
delay...
OLED_Fill(0x07E0); // Green
delay...
OLED_Fill(0x001F); // Blue

while (1) { }


Cấu hình SPI, khởi tạo LCD.

Lần lượt đổ màu đỏ, xanh lá, xanh dương để test màn hình.

