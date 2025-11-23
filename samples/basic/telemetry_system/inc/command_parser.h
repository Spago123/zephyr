#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "communication_interface.h"

typedef void (*command_process_fn)(struct telemetry_comm_interface *comm_interface, const char *data, size_t len);

void command_parser_process_data(struct telemetry_comm_interface *comm_interface, const char *data, size_t len);

void get_variable_names(struct telemetry_comm_interface *comm_interface, const char *data, size_t len);

void get_variable(struct telemetry_comm_interface *comm_interface, const char *data, size_t len);

void set_variable(struct telemetry_comm_interface *comm_interface, const char *data, size_t len);

#endif // COMMAND_PARSER_H