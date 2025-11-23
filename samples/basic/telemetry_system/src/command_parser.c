#include "command_parser.h"
#include "registry.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* ---------------- COMMAND LOOKUP TABLE ---------------- */
static command_process_fn command_handlers[] = {
    get_variable_names,
    get_variable,
    set_variable
};

static const char *command_strings[] = {
    "get_variable_names",
    "get_variable",
    "set_variable"
};

#define CMD_COUNT (sizeof(command_strings) / sizeof(command_strings[0]))

/* ---------------- GENERIC PARSER ---------------- */
void command_parser_process_data(struct telemetry_comm_interface *comm_interface,
                                 const char *data, size_t len)
{
    for (size_t i = 0; i < CMD_COUNT; i++) {
        size_t cmd_len = strlen(command_strings[i]);
        if (len >= cmd_len && strncmp(data, command_strings[i], cmd_len) == 0) {
            /* Expect parentheses */
            const char *args_start = data + cmd_len;
            while (*args_start == ' ') args_start++;

            if (*args_start != '(') {
                comm_interface->send(comm_interface->dev,
                                     "ERR:INVALID_FORMAT\n",
                                     strlen("ERR:INVALID_FORMAT\n"));
                return;
            }
            args_start++; // skip '('

            /* Find closing parenthesis */
            const char *args_end = strchr(args_start, ')');
            if (!args_end) {
                comm_interface->send(comm_interface->dev,
                                     "ERR:INVALID_FORMAT\n",
                                     strlen("ERR:INVALID_FORMAT\n"));
                return;
            }

            size_t args_len = args_end - args_start;
            command_handlers[i](comm_interface, args_start, args_len);
            return;
        }
    }

    comm_interface->send(comm_interface->dev,
                         "ERR:UNKNOWN_COMMAND\n",
                         strlen("ERR:UNKNOWN_COMMAND\n"));
}

/* ---------------- COMMAND IMPLEMENTATIONS ---------------- */
void get_variable_names(struct telemetry_comm_interface *comm_interface,
                        const char *data, size_t len)
{
    (void)data; (void)len;
    char response[512];
    size_t pos = 0;

    for (int i = 0; i < VAR_COUNT; i++) {
        var_entry *v = registry_get((var_id)i);
        if (!v) continue;

        pos += snprintf(response + pos, sizeof(response) - pos,
                        "%d:%s;", i, v->name);

        if (pos >= sizeof(response) - 16)
            break;
    }

    if (pos == 0) {
        comm_interface->send(comm_interface->dev,
                             "ERR:NO_VARIABLES\n",
                             strlen("ERR:NO_VARIABLES\n"));
        return;
    }

    // response[pos++] = '\n';
    comm_interface->send(comm_interface->dev, response, pos);
}

void get_variable(struct telemetry_comm_interface *comm_interface,
                  const char *data, size_t len)
{
    int index;
    if (sscanf(data, "%d", &index) != 1) {
        comm_interface->send(comm_interface->dev,
                             "ERR:INVALID_ARGUMENT\n",
                             strlen("ERR:INVALID_ARGUMENT\n"));
        return;
    }

    var_entry *v = registry_get((var_id)index);

    if (v == NULL) {
        comm_interface->send(comm_interface->dev,
                             "ERR:VARIABLE_NOT_FOUND\n",
                             strlen("ERR:VARIABLE_NOT_FOUND\n"));
        return;
    }

    char response[128];
    int len_out = snprintf(response, sizeof(response), "OK;%s:%g\n",
                           v->name, (double)*v->ptr);

    comm_interface->send(comm_interface->dev, response, len_out);
}

void set_variable(struct telemetry_comm_interface *comm_interface,
                  const char *data, size_t len)
{
    const char *ptr = data;
    int id;
    float value;
    int success_count = 0;

    while (sscanf(ptr, "%d:%f", &id, &value) == 2) {
        if (id < 0 || id >= VAR_COUNT) {
            comm_interface->send(comm_interface->dev,
                                 "ERR:INVALID_INDEX\n",
                                 strlen("ERR:INVALID_INDEX\n"));
            return;
        }

        var_entry *v = registry_get((var_id)id);
        if (!v) {
            comm_interface->send(comm_interface->dev,
                                 "ERR:VARIABLE_NOT_FOUND\n",
                                 strlen("ERR:VARIABLE_NOT_FOUND\n"));
            return;
        }

        *(v->ptr) = value;
        success_count++;

        const char *next = strchr(ptr, ';');
        if (!next) break;
        ptr = next + 1;
    }

    if (success_count == 0) {
        comm_interface->send(comm_interface->dev,
                             "ERR:INVALID_ARGUMENTS\n",
                             strlen("ERR:INVALID_ARGUMENTS\n"));
        return;
    }

    comm_interface->send(comm_interface->dev,
                         "OK\n",
                         strlen("OK\n"));
}
