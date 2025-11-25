#include "command_processor.h"
#include <string.h>

struct command_msg {
    struct telemetry_comm_interface *comm_interface;
    size_t len;
    char buf[CMD_MAX_LEN];
};

K_MSGQ_DEFINE(cmd_msgq, sizeof(struct command_msg), CMD_QUEUE_LEN, 4);

static struct k_work cmd_work;

static void cmd_work_handler(struct k_work *work)
{
    struct command_msg msg;

    while (k_msgq_get(&cmd_msgq, &msg, K_NO_WAIT) == 0) {
        command_parser_process_data(
            msg.comm_interface,
            msg.buf,
            msg.len
        );
    }
}


void command_processor_init(void)
{
    k_work_init(&cmd_work, cmd_work_handler);
}

void command_processor_submit(struct telemetry_comm_interface *iface,
                              const char *cmd,
                              size_t len)
{
    struct command_msg msg = {0};

    msg.comm_interface = iface;
    msg.len = len;
    memcpy(msg.buf, cmd, MIN(len, CMD_MAX_LEN - 1));

    k_msgq_put(&cmd_msgq, &msg, K_NO_WAIT);

    /* Schedule worker */
    k_work_submit(&cmd_work);
}

