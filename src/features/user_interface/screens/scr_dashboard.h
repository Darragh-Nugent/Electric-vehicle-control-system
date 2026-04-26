
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"

// The widget instance — added to the widget tree in main.c
extern tCanvasWidget g_sDashboardPanel;

// Paint callback — called by WidgetMessageQueueProcess() when a repaint
// is requested. Signature must match tOnPaint exactly.
void OnDashboardPaint(tWidget *psWidget, tContext *psContext);

// Call this from the GUI task to request a repaint with fresh data.
// Safe to call from any task — just calls WidgetPaint() which is
// queue-based and non-blocking.
void scr_dashboard_refresh(void);