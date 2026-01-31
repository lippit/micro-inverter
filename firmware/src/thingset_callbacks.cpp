/* ThingSet callbacks aligned with the command interface. */

#include <thingset.h>

#include "user_data_api.h"

void conf_command_cb(enum thingset_callback_reason reason)
{
    if (reason == THINGSET_CALLBACK_POST_WRITE) {
        app_apply_command();
    }
}
