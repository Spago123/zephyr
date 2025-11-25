#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <zephyr/kernel.h>
#include "command_parser.h"
#include "communication_interface.h"

#define CMD_QUEUE_LEN 10

void command_processor_init(void);

void command_processor_submit(struct telemetry_comm_interface *iface, const char *cmd, size_t len);

#endif // COMMAND_PROCESSOR_H
