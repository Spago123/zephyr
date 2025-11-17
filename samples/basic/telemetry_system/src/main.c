#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include "communication_interface.h"
#include "registry.h"
#include <math.h>

float temperature = 25.5f;
float pressure = 1013.2f;
float humidity = 49.3f;

static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

int main(void)
{
    if (!device_is_ready(uart)) {
        printk("UART device not ready\n");
        return -1;
    }

    struct telemetry_comm_interface *comm_interface = get_uart_comm_interface();
    set_telemetry_comm_device(comm_interface, uart);
    uart_irq_callback_set(uart, comm_interface->recv);
    uart_irq_rx_enable(uart);
    register_telemetry_sender(comm_interface);
    
    printk("Telemetry System Started. Waiting for data...\n");
    
    float time_step = 0.0f;
    
    while (1) {
        k_sleep(K_MSEC(1));
        
        time_step += 0.001f;
        
        temperature = 25.5f + 8.0f * sinf(time_step * 5.f);
        pressure = 25.2f + 15.0f * sinf(time_step * 10.f);
        humidity = 25.3f + 20.0f * sinf(time_step * 15.f);
    }

    return 0;
}