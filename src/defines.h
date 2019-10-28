#ifndef DEFINES_H_
#define DEFINES_H_

#define VIEWPORT_WIDTH 1280
#define VIEWPORT_HEIGHT 720

#define LEFT_PANEL_WIDTH 350

#define INPUT_BUFFER_LEN 1024 * 5
#define INTERPOLATION_NAME_LEN 255


#define MEM_ZERO(_var)                                                                  \
    memset(&(_var), 0, sizeof((_var)));

#endif // DEFINES_H_
