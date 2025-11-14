#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#define RX_BUF_SIZE 128

struct rx_data {
    uint8_t buffer[RX_BUF_SIZE];
    size_t index;
};

static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static uint8_t rx_buffer[RX_BUF_SIZE];
static size_t rx_index = 0;

static void uart_cb(const struct device *dev, void *user_data)
{
    uint8_t c;

    if (uart_irq_update(dev) && uart_irq_rx_ready(dev)) {
        while (uart_fifo_read(dev, &c, 1)) {
            if (rx_index < RX_BUF_SIZE - 1) {
                rx_buffer[rx_index++] = c;
                if (c == '\n') {
                    rx_buffer[rx_index] = '\0'; // Null-terminate the string
                    printk("Received: %s", rx_buffer);
                    rx_index = 0; // Reset index for next message
                }
            } else {
                // Buffer overflow, reset index
                rx_index = 0;
            }
        }
    }
}

int main(void)
{
    if (!device_is_ready(uart)) {
        printk("UART device not ready\n");
        return -1;
    }

    uart_irq_callback_set(uart, uart_cb);
    uart_irq_rx_enable(uart);

    printk("Telemetry System Started. Waiting for data...\n");

    while (1) {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
