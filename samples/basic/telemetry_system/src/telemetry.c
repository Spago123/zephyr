#include "registry.h"
#include "communication_interface.h"
#include <stdio.h>
#include <string.h>

struct registered_telemetry_senders_t registered_telemetry_senders;

#if !defined(CONFIG_TELEMETRY_ENABLE) || (CONFIG_TELEMETRY_ENABLE == 0)
#warning "Telemetry disabled by config"
#endif

#define TELEMETRY_INTERVAL_MS CONFIG_TELEMETRY_INTERVAL_MS

static inline void send_messages_over_senders(const char* message, size_t len) {
    for(size_t i = 0; i < registered_telemetry_senders.count; i++) {
        const struct telemetry_comm_interface* iface = registered_telemetry_senders.interfaces[i];
        iface->send(iface->dev, message, len);
    }
}

void telemetry_sender_task(void *a, void *b, void *c) {
    (void)a; (void)b; (void)c;
    char line[512] = {'\0'};
    while (1) {
        size_t pos = 0;
        for (int i = 0; i < VAR_COUNT; i++) {
            var_entry *v = registry_get((var_id)i);
            pos += snprintf(line + pos, sizeof(line) - pos, "%s:%g;", v->name, (double)*v->ptr);
            if (pos >= sizeof(line) - 32) break;
        }
        if (pos > 0) {
            line[pos++] = '\n';
            send_messages_over_senders(line, pos);
        }
        k_msleep(TELEMETRY_INTERVAL_MS);
    }
}

K_THREAD_DEFINE(telemetry_sender_tid, 1024, telemetry_sender_task, NULL, NULL, NULL, 5, 0, 0);
