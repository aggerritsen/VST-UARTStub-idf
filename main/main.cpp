// main.cpp – XIAO ESP32-S3 UART stub with sender/receiver role
// Switch role by setting XIAO_UART_IS_SENDER to 1 (sender) or 0 (receiver).

extern "C" {
    #include <stdio.h>
    #include <string.h>

    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #include "driver/uart.h"
    #include "esp_log.h"
}

// -----------------------------------------------------------------------------
// CONFIG
// -----------------------------------------------------------------------------

// 1 = XIAO is SENDER (stuurt PINGs, leest eventuele replies)
// 0 = XIAO is RECEIVER (stuurt ACK:<payload> terug)
#define XIAO_UART_IS_SENDER  1

static const char *TAG = "XIAO_UART";

// UART1 op XIAO-ESP32S3, bedraad naar T-SIM / VisionAI
#define STUB_UART_NUM        UART_NUM_1
#define STUB_UART_TX_PIN     1    // D0
#define STUB_UART_RX_PIN     2    // D1
#define STUB_UART_BAUDRATE   115200

#define RX_BUF_SIZE          256

// -----------------------------------------------------------------------------
// SENDER TASK
// -----------------------------------------------------------------------------
#if XIAO_UART_IS_SENDER

static void uart_sender_task(void *arg)
{
    uint8_t rx_data[RX_BUF_SIZE];
    uint32_t counter = 0;

    ESP_LOGI(TAG, "Running in SENDER mode");

    while (true) {
        // Build PING string
        char tx_buf[64];
        int len = snprintf(
            tx_buf,
            sizeof(tx_buf),
            "PING %lu\r\n",
            static_cast<unsigned long>(++counter)
        );

        // Send PING over UART
        int sent = uart_write_bytes(STUB_UART_NUM, tx_buf, len);
        ESP_LOGI(TAG, ">> Sent (%d bytes): '%s'", sent, tx_buf);

        // Kleine window om eventuele reply te lezen (bijv. ACK)
        TickType_t end_tick = xTaskGetTickCount() + pdMS_TO_TICKS(500);

        while (xTaskGetTickCount() < end_tick) {
            int rx_len = uart_read_bytes(
                STUB_UART_NUM,
                rx_data,
                RX_BUF_SIZE - 1,
                pdMS_TO_TICKS(50)  // 50 ms per poll
            );

            if (rx_len > 0) {
                rx_data[rx_len] = 0;  // null-terminate voor logging
                ESP_LOGI(TAG, "<< Received (%d bytes): '%s'", rx_len, (char *)rx_data);
            }
        }

        // Elke seconde een nieuwe PING
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

#else  // ----------------------------------------------------------------------
// RECEIVER TASK (ACK stub)
// -----------------------------------------------------------------------------

static void uart_receiver_task(void *arg)
{
    uint8_t rx_data[RX_BUF_SIZE];

    ESP_LOGI(TAG, "Running in RECEIVER mode");

    while (true) {
        // Lees bytes met timeout (100 ms)
        int len = uart_read_bytes(
            STUB_UART_NUM,
            rx_data,
            RX_BUF_SIZE - 1,
            pdMS_TO_TICKS(100)
        );

        if (len > 0) {
            rx_data[len] = 0; // null-terminate voor logging
            ESP_LOGI(TAG, "RX (%d bytes): '%s'", len, (char *)rx_data);

            // Bouw "ACK:" + ontvangen data
            const char prefix[] = "ACK:";
            uint8_t out[sizeof(prefix) - 1 + RX_BUF_SIZE];
            int out_len = 0;

            memcpy(out + out_len, prefix, sizeof(prefix) - 1);
            out_len += sizeof(prefix) - 1;

            memcpy(out + out_len, rx_data, len);
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

#endif  // XIAO_UART_IS_SENDER

// -----------------------------------------------------------------------------
// app_main – gemeenschappelijke init
// -----------------------------------------------------------------------------

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "XIAO UART stub starting...");

    // Robuust: zero-init hele struct om missing-field warnings te voorkomen
    uart_config_t uart_config;
    memset(&uart_config, 0, sizeof(uart_config));

    uart_config.baud_rate = STUB_UART_BAUDRATE;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    // Nieuwe flags/velden in latere IDF-versies blijven 0, dat is veilig.

    // Installeer UART driver (alleen RX buffer, TX is blocking writes)
    ESP_ERROR_CHECK(uart_driver_install(
        STUB_UART_NUM,
        RX_BUF_SIZE * 2,   // RX buffer
        0,                 // geen TX buffer
        0,                 // geen event queue
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

#if XIAO_UART_IS_SENDER
    xTaskCreate(uart_sender_task, "uart_sender_task", 4096, nullptr, 10, nullptr);
#else
    xTaskCreate(uart_receiver_task, "uart_receiver_task", 4096, nullptr, 10, nullptr);
#endif

    // main_task keert direct terug; FreeRTOS tasks doen de rest
}
