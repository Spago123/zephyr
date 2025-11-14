#include "communication_interface.h"
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

static struct {
    char buffer[128];
    size_t index;
} rx_data;

static void uart_receive_callback(const struct device *dev, void *user_data)
{
    char c;
    while (uart_poll_in(dev, (unsigned char *)&c) == 0) {
        if (rx_data.index < sizeof(rx_data.buffer) - 1) {
            rx_data.buffer[rx_data.index++] = c;
            if (c == '\n') {
                rx_data.buffer[rx_data.index] = '\0'; // Null-terminate
                // Process the received line (for example, print it)
                printk("Received: %s", rx_data.buffer);
                rx_data.index = 0; // Reset for next line
            }
        } else {
            // Buffer overflow, reset index
            rx_data.index = 0;
        }
    }
}

static void uart_send_data(const struct device *dev, const char *data, size_t len)
{
    (void)dev; // Unused parameter
    (void)len; // Unused parameter
    printk("%s", data);
}

const struct telemetry_comm_interface uart_comm_interface = {
    .recv = uart_receive_callback,
    .send = uart_send_data,
};

const struct telemetry_comm_interface *get_uart_comm_interface(void)
{
    return &uart_comm_interface;
}