#ifndef DEFINES_H_
#define DEFINES_H_

#define VIEWPORT_WIDTH 1280
#define VIEWPORT_HEIGHT 720

#define LEFT_PANEL_WIDTH 350
#define LEFT_PANEL_HEIGHT VIEWPORT_HEIGHT

#define GRAPH_HEIGHT 500

#define MIDDLE_PANEL_WIDTH (VIEWPORT_WIDTH - LEFT_PANEL_WIDTH)
#define MIDDLE_PANEL_HEIGHT (GRAPH_HEIGHT + 10)

#define BOTTOM_PANEL_WIDTH MIDDLE_PANEL_WIDTH
#define BOTTOM_PANEL_HEIGHT VIEWPORT_HEIGHT - MIDDLE_PANEL_HEIGHT

#define INPUT_BUFFER_LEN 1024 * 5
#define INTERPOLATION_NAME_LEN 255

#define ZERO_MEM(_var)                                                                  \
    memset(&(_var), 0, sizeof((_var)));

#endif // DEFINES_H_
