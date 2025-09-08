#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "blink.pio.h"

// ==== サーボ PWM設定 ====
const uint PWM_PIN = 11;
const uint16_t STOP_PULSE_US = 1500;
const uint16_t CW_PULSE_US   = 1800;
const uint16_t CCW_PULSE_US  = 1200;
const float PWM_FREQ = 50;

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
#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

// ==== PIO Blink ====
void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);
    printf("Blinking pin %d at %d Hz\n", pin, freq);
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

int main() {
    stdio_init_all();

    // ==== PWM初期化 ====
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    //以下赤線の修正
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    uint channel = pwm_gpio_to_channel(PWM_PIN);
    float div = 125000000.0f / (PWM_FREQ * 4096.0f);
    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, 4095);
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

    // ==== PIO LED点滅 ====
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
#ifdef PICO_DEFAULT_LED_PIN
    blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 3);
#else
    blink_pin_forever(pio, 0, offset, 6, 3);
#endif

    // ==== UART初期化 ====
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_puts(UART_ID, " Hello, UART!\n");

    // ==== メインループ ====
    while (true) {
        // サーボを順番に回す
        printf("Rotating CW...\n");
        pwm_set_chan_level(slice_num, channel, (uint16_t)(4095.0f * (CW_PULSE_US / 20000.0f)));
        sleep_ms(2000);

        printf("Stopping...\n");
        pwm_set_chan_level(slice_num, channel, (uint16_t)(4095.0f * (STOP_PULSE_US / 20000.0f)));
        sleep_ms(2000);

        printf("Rotating CCW...\n");
        pwm_set_chan_level(slice_num, channel, (uint16_t)(4095.0f * (CCW_PULSE_US / 20000.0f)));
        sleep_ms(2000);

        printf("Stopping...\n");
        pwm_set_chan_level(slice_num, channel, (uint16_t)(4095.0f * (STOP_PULSE_US / 20000.0f)));
        sleep_ms(2000);

        // UARTからも送信
        uart_puts(UART_ID, "Looping servo + peripherals...\n");

        // デバッグ用
        printf("Hello, world!\n");
    }

    return 0;
}
