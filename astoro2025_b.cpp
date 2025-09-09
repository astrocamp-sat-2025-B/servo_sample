#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"

// ==== サーボ PWM設定 ====
const uint PWM_PIN = 11;
// const uint16_t STOP_PULSE_US = 1500; // 規定値　1500us
// const uint16_t CW_PULSE_US   = 1200; // 規定値　1500-700
// const uint16_t CCW_PULSE_US  = 1800; // 規定値　1500-2300
#define PWM_FREQ 400
#define PWM_DIVIDER 125.0f
#define WRAP ((clock_get_hz(clk_sys) / PWM_DIVIDER) / PWM_FREQ)

// ==== SPI設定 ====
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// ==== I2C設定 ====
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// ==== UART設定 ====
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 12 // 修正
#define UART_RX_PIN 13 // 修正

// ==== パルス幅をPWMレベルに変換する関数 ====
uint16_t us_to_level(uint16_t us) {
    const uint16_t PERIOD_US = 2500;
    return (uint16_t)((float)us / PERIOD_US * WRAP);
}


int main() {
    stdio_init_all();
    printf("Hello, world!\n");

    // ==== PWM初期化 ====
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    uint channel = pwm_gpio_to_channel(PWM_PIN);
    pwm_set_clkdiv(slice_num, PWM_DIVIDER); //ここのあたりを理解
    pwm_set_wrap(slice_num, WRAP);
    pwm_set_enabled(slice_num, true);

    // ==== SPI初期化 ====
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // ==== I2C初期化 ====
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // ==== UART初期化 ====
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_puts(UART_ID, " Hello, UART!\n");

    sleep_ms(2000);

    // ==== メインループ ====
    while (true) {

        uart_puts(UART_ID, "Loop!\n");

        printf("wrap=%d\n", WRAP);
        pwm_set_chan_level(slice_num, channel,us_to_level(700));  //CW
        sleep_ms(1000);
        pwm_set_chan_level(slice_num, channel,us_to_level(1500)); //stop
        sleep_ms(100);
        pwm_set_chan_level(slice_num, channel,us_to_level(2300)); //CCW
        sleep_ms(1000);
        pwm_set_chan_level(slice_num, channel,us_to_level(1500)); //stop
        sleep_ms(1000);

        uart_puts(UART_ID, "Looping...\n");
    }

    return 0;
}