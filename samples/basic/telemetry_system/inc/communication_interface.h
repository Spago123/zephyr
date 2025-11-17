#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <stddef.h>

typedef void  (*comm_recv_fn)(const struct device *dev, void *user_data);
typedef void (*comm_send_fn)(const struct device *dev, const char *data, size_t len);

struct telemetry_comm_interface {
    struct device const *dev;
    comm_recv_fn recv;
    comm_send_fn send;
};

struct registered_telemetry_senders_t {
    const struct telemetry_comm_interface* interfaces[4]; // Hard coded for now
    size_t count;
};

extern struct registered_telemetry_senders_t registered_telemetry_senders;

struct telemetry_comm_interface *get_uart_comm_interface(void);

static inline void register_telemetry_sender(const struct telemetry_comm_interface *iface) {
    if (registered_telemetry_senders.count < 4) {
        registered_telemetry_senders.interfaces[registered_telemetry_senders.count++] = iface;
    }
}

static inline void set_telemetry_comm_device(struct telemetry_comm_interface *iface, const struct device *dev) {
    iface->dev = dev;
}

#endif // COMMUNICATION_INTERFACE_H