#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "grlib.h"
#include "../../data.h"

void scr_dashboard_init(tContext *ctx);   // called once at startup
void scr_dashboard_draw(void);            // full redraw
void scr_dashboard_update(void);          // partial update, changed data only