// main.cpp – XIAO-ESP32S3 UART stub (ESP-IDF, C++)

extern "C" {
    #include <stdio.h>
    #include <string.h>

    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #include "driver/uart.h"
    #include "driver/gpio.h"
    #include "esp_log.h"
}

static const char *TAG = "XIAO_STUB";

// UART1 on XIAO-ESP32S3, connected to T-SIM
#define STUB_UART_NUM      UART_NUM_1
//#define STUB_UART_TX_PIN   43   // D6
//#define STUB_UART_RX_PIN   44   // D7
#define STUB_UART_TX_PIN   1    // D0
#define STUB_UART_RX_PIN   2    // D1
#define STUB_UART_BAUDRATE 115200

#define RX_BUF_SIZE 256

static void uart_stub_task(void *arg)
{
    uint8_t data[RX_BUF_SIZE];

    while (true) {
        // Read bytes with timeout (100 ms)
        int len = uart_read_bytes(
            STUB_UART_NUM,
            data,
            RX_BUF_SIZE - 1,
            pdMS_TO_TICKS(100)
        );

        if (len > 0) {
            data[len] = 0; // null-terminate for logging
            ESP_LOGI(TAG, "RX (%d): '%s'", len, (char *)data);

            // Build "ACK:" + received data
            const char prefix[] = "ACK:";
            uint8_t out[sizeof(prefix) - 1 + RX_BUF_SIZE];
            int out_len = 0;

            memcpy(out + out_len, prefix, sizeof(prefix) - 1);
            out_len += sizeof(prefix) - 1;

            memcpy(out + out_len, data, len);
            out_len += len;

            int sent = uart_write_bytes(
                STUB_UART_NUM,
                reinterpret_cast<const char *>(out),
                out_len
            );
            ESP_LOGI(TAG, "TX ACK (%d bytes)", sent);
        }
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "XIAO UART stub starting...");

    // Robust: zero-init struct, then set fields explicitly
    uart_config_t uart_config;
    memset(&uart_config, 0, sizeof(uart_config));

    uart_config.baud_rate = STUB_UART_BAUDRATE;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    // Any extra fields/flags in newer IDF versions remain zero, which is safe.

    // Install UART driver (RX buffer only, blocking writes)
    ESP_ERROR_CHECK(uart_driver_install(
        STUB_UART_NUM,
        RX_BUF_SIZE * 2,  // rx buffer
        0,                // no tx buffer
        0,                // no event queue
        nullptr,
        0
    ));

    ESP_ERROR_CHECK(uart_param_config(STUB_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(
        STUB_UART_NUM,
        STUB_UART_TX_PIN,
        STUB_UART_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    ));

    ESP_LOGI(TAG, "UART1 configured: TX=%d, RX=%d, %d baud",
             STUB_UART_TX_PIN, STUB_UART_RX_PIN, STUB_UART_BAUDRATE);

    xTaskCreate(uart_stub_task, "uart_stub_task", 4096, nullptr, 10, nullptr);
}
