#include "touch.h"

struct view {
    int x;
    int y;
    int width;
    int height;
    int radius;

    void (*onClick) (struct view view);
};

typedef struct view View;
