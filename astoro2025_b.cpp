#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "blink.pio.h"
#include "hardware/clocks.h"

// ==== サーボ PWM設定 ====
const uint PWM_PIN = 11;
// const uint16_t STOP_PULSE_US = 1500; // 規定値　1500us
// const uint16_t CW_PULSE_US   = 1200; // 規定値　1500-700
// const uint16_t CCW_PULSE_US  = 1800; // 規定値　1500-2300
#define PWM_FREQ 50;
#define PWM_DIVIDER 100.0f
const uint16_t WRAP_VAL = 25000 - 1; // 50Hz, 1us分解能

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

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 2000
#endif

// ==== PIO Blink ====
void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);
    printf("Blinking pin %d at %d Hz\n", pin, freq);
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

// ==== パルス幅をPWMレベルに変換する関数 ====
uint16_t us_to_level(uint16_t us) {
    return (uint16_t)((float)us / 20000.0f * (WRAP_VAL + 1));
}

// add section
int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    return cyw43_arch_init();
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}


int main() {
    stdio_init_all();
    printf("Hello, world!\n");

    // ==== PWM初期化 ====
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    uint channel = pwm_gpio_to_channel(PWM_PIN);
    pwm_set_clkdiv(slice_num, PWM_DIVIDER); //ここのあたりを理解
    uint16_t wrap = (clock_get_hz(clk_sys) / PWM_DIVIDER) / PWM_FREQ;
    pwm_set_wrap(slice_num, wrap);
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

    // ==== メインループ ====
    while (true) {

        uart_puts(UART_ID, "Loop!\n");

        uart_putc(UART_ID, wrap);
        pwm_set_chan_level(slice_num, channel,4000);
        sleep_ms(1000);
        pwm_set_chan_level(slice_num, channel,1860);
        sleep_ms(1000);
        pwm_set_chan_level(slice_num, channel,1400);
        sleep_ms(1000);
        pwm_set_chan_level(slice_num, channel,1860);
        sleep_ms(1000);

        //for (int i = 0; i < wrap; i+=200)
        //{
        //    uart_putc(UART_ID, i); 
        //    uart_puts(UART_ID, "\n");
        //    // 以下挙動おかしい
        //    pwm_set_chan_level(slice_num, channel, i);
        //    sleep_ms(100);
        //    // ...
        //}

        // UARTからも送信
        // uart_puts(UART_ID, "Looping servo...\n");
        uart_puts(UART_ID, "Looping...\n");
    }

    // return 0;
}