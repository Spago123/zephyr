#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <stddef.h>

typedef void  (*comm_recv_fn)(const struct device *dev, void *user_data);
typedef void (*comm_send_fn)(const struct device *dev, const char *data, size_t len);

struct telemetry_comm_interface {
    comm_recv_fn recv;
    comm_send_fn send;
};

const struct telemetry_comm_interface *get_uart_comm_interface(void);

#endif // COMMUNICATION_INTERFACE_H