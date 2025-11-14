#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include "communication_interface.h"

#define RX_BUF_SIZE 128

struct rx_data {
    uint8_t buffer[RX_BUF_SIZE];
    size_t index;
};

static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

int main(void)
{
    if (!device_is_ready(uart)) {
        printk("UART device not ready\n");
        return -1;
    }

    const struct telemetry_comm_interface *comm_interface = get_uart_comm_interface();

    uart_irq_callback_set(uart, comm_interface->recv);
    uart_irq_rx_enable(uart);

    printk("Telemetry System Started. Waiting for data...\n");

    while (1) {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
