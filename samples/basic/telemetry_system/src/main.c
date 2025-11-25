#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include "command_processor.h"
#include <math.h>

float temperature = 25.5f;
float pressure = 1013.2f;
float humidity = 49.3f;
float led0_status = 0.0f;
float led1_status = 0.0f;

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static void uart_receive_callback(const struct device *dev, void *user_data);
static void uart_send_data(const struct device *dev, const char *data, size_t len);

struct telemetry_comm_interface comm_interface;

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
                command_processor_submit(&comm_interface, rx_data.buffer, rx_data.index);
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


void update_led(const struct gpio_dt_spec* led, float led_status)
{
    if (led_status > 0.5f) {
        gpio_pin_set_dt(led, 1);  // LED ON
    } else {
        gpio_pin_set_dt(led, 0);  // LED OFF
    }
}

int main(void)
{
    if (!device_is_ready(led0.port)) {
        printk("Error: LED device %s not ready\n", led0.port->name);
        return -1;
    }

    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    if (!device_is_ready(led1.port)) {
        printk("Error: LED device %s not ready\n", led1.port->name);
        return -1;
    }

    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);

    if (!device_is_ready(uart)) {
        printk("UART device not ready\n");
        return -1;
    }

    comm_interface.dev = (struct device *)uart;
    comm_interface.recv = uart_receive_callback;
    comm_interface.send = uart_send_data;

    uart_irq_callback_set(uart, comm_interface.recv);
    uart_irq_rx_enable(uart);
    register_telemetry_sender(&comm_interface);

    command_processor_init();
    
    printk("Telemetry System Started. Waiting for data...\n");
    
    float time_step = 0.0f;
    
    while (1) {
        k_sleep(K_MSEC(1));
        
        time_step += 0.001f;
        
        temperature = 25.5f + 8.0f * sinf(time_step * 5.f);
        pressure = 25.2f + 15.0f * sinf(time_step * 10.f);
        humidity = 25.3f + 20.0f * sinf(time_step * 15.f);

        update_led(&led0, led0_status);
        update_led(&led1, led1_status);
    }

    return 0;
}