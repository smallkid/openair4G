#ifndef UI_FILTERS_H_
#define UI_FILTERS_H_

#include <stdint.h>

#include "itti_types.h"

typedef enum
{
    FILTER_MESSAGES, FILTER_ORIGIN_TASKS, FILTER_DESTINATION_TASKS,
} ui_filter_e;

typedef struct
{
    uint32_t id;
    char name[SIGNAL_NAME_LENGTH];
    uint8_t enabled;
} ui_filter_item_t;

typedef struct
{
    uint32_t allocated;
    uint32_t used;
    ui_filter_item_t *items;
} ui_filter_t;

typedef struct
{
    ui_filter_t messages;
    ui_filter_t origin_tasks;
    ui_filter_t destination_tasks;
} ui_filters_t;

extern ui_filters_t ui_filters;

int ui_init_filters(int reset, int clear_ids);

void ui_filters_add(ui_filter_e filter, uint32_t value, char *name);

#endif /* UI_FILTERS_H_ */