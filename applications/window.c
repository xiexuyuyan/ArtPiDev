#include <rtthread.h>
#include <rtdevice.h>
#include "ft6236.h"
#include "touch.h"
#include "drv_common.h"

#include "drv_spi_ili9488.h"
#include "lcd_spi_port.h"

#define DBG_TAG "window"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "window.h"

rt_device_t touch;

void dispatchClickEvent(struct rt_touch_data touchDown, struct rt_touch_data touchUp) {
    rt_uint16_t down_x = touchDown.x_coordinate;
    rt_uint16_t down_y = touchDown.y_coordinate;
    rt_uint16_t up_x = touchUp.x_coordinate;
    rt_uint16_t up_y = touchUp.y_coordinate;
        
    for (int i = 0; i < ROOT_VIEW_SIZE; i++) {
        View view = rootView[i];

        int x1 = view.x;
        int x2 = view.x + view.width;
        int y1 = view.y;
        int y2 = view.y + view.height;

        if (down_x >= x1 && down_x <= x2 \
            && down_y >= y1 && down_y <= y2 \
            && up_x >= x1 && up_x <= x2 \
            && up_y >= y1 && up_y <= y2) {
                view.onClick(view);
                break;
        }
    }
}

void ft6236_thread_entry(void *parameter) {
    struct DownTrigger {
        int state;
        rt_tick_t timestamp;
    } downTrigger;

    downTrigger.state = 0;
    downTrigger.timestamp = 0;

    struct rt_touch_data *read_data;

    read_data = (struct rt_touch_data *)rt_calloc(1, sizeof(struct rt_touch_data));

    while(1)
    {
        rt_device_read(touch, 0, read_data, 1);

        rt_uint16_t touch_x = read_data->x_coordinate;
        rt_uint16_t touch_y = read_data->y_coordinate;

        rt_uint16_t view_x = touch_y;
        rt_uint16_t view_y = 320 - touch_x;

        struct rt_touch_data readDataDown;

        if (read_data->event == RT_TOUCH_EVENT_DOWN) {
            downTrigger.state = 1;
            downTrigger.timestamp = read_data->timestamp;

            readDataDown.event = read_data->event;
            readDataDown.timestamp = read_data->timestamp;
            readDataDown.track_id = read_data->track_id;
            readDataDown.width = read_data->width;
            readDataDown.x_coordinate = read_data->y_coordinate;
            readDataDown.y_coordinate = 320 - read_data->x_coordinate;

            rt_kprintf("down x: %03d y: %03d", view_x, view_y);
            rt_kprintf(" t: %d\n", read_data->timestamp);
        } else if (read_data->event == RT_TOUCH_EVENT_MOVE) {
            rt_kprintf("move x: %03d y: %03d", view_x, view_y);
            rt_kprintf(" t: %d\n", read_data->timestamp);
        } else if (read_data->event == RT_TOUCH_EVENT_UP) {
            if (downTrigger.state == 1) {
                rt_kprintf(" click up \n");
                if (((read_data->timestamp) - downTrigger.timestamp) <= 10) {
                    rt_kprintf(" click in short \n");
                    struct rt_touch_data readDataUp;
                    readDataUp.event = read_data->event;
                    readDataUp.timestamp = read_data->timestamp;
                    readDataUp.track_id = read_data->track_id;
                    readDataUp.width = read_data->width;
                    readDataUp.x_coordinate = read_data->y_coordinate;
                    readDataUp.y_coordinate = 320 - read_data->x_coordinate;
                    dispatchClickEvent(readDataDown, readDataUp);
                }
            }

            downTrigger.state = 0;
            downTrigger.timestamp = 0;

            rt_kprintf("up   x: %03d y: %03d", view_x, view_y);
            rt_kprintf(" t: %d\n\n", read_data->timestamp);
        }
        rt_thread_delay(10);
    }

}

#define REST_PIN GET_PIN(D, 3)

int window_touch_init(void) {
    rt_thread_t ft6236_thread;
    struct rt_touch_config cfg;

    cfg.dev_name = "i2c2";
    rt_hw_ft6236_init("touch", &cfg, REST_PIN);

    touch = rt_device_find("touch");

    rt_device_open(touch, RT_DEVICE_FLAG_RDONLY);

    struct rt_touch_info info;
    rt_device_control(touch, RT_TOUCH_CTRL_GET_INFO, &info);
    LOG_I("type       :%d", info.type);
    LOG_I("vendor     :%d", info.vendor);
    LOG_I("point_num  :%d", info.point_num);
    LOG_I("range_x    :%d", info.range_x);
    LOG_I("range_y    :%d", info.range_y);

    ft6236_thread = rt_thread_create("touch", ft6236_thread_entry, RT_NULL, 800, 10, 20);
    if (ft6236_thread == RT_NULL)
    {
        LOG_D("create ft6236 thread err");

        return -RT_ENOMEM;
    }
    rt_thread_startup(ft6236_thread);

    return RT_EOK;
}
INIT_APP_EXPORT(window_touch_init);


/* --------------------------------------------------------------------------------- */
void drawButton(View view) {
    int x = view.x;
    int y = view.y;
    int width = view.width;
    int height = view.height;
    int radius = view.radius;

    struct drv_lcd_device *lcd;
    lcd = (struct drv_lcd_device *)rt_device_find("lcd");
    struct rt_device_rect_info rect_info = {0, 0, LCD_WIDTH, LCD_HEIGHT};


    rect_info.x = x;
    rect_info.y = y;
    rect_info.width = width;
    rect_info.height = height;

    for (int i = 0; i < height; i++) {
        int startP = (y + i) * LCD_WIDTH * 3 + x * 3;

        int radiusTag = 0;
        if (i < radius || i > (height - radius)) {
            startP += radius * 3;
            radiusTag = 1;
        }

        for (int j = 0; j < width; j++) {
            if ((radiusTag == 1) && j >= (width - radius * 2)) {
                break;
            }

            lcd->lcd_info.framebuffer[startP++] = 0xDD;
            lcd->lcd_info.framebuffer[startP++] = 0xDD;
            lcd->lcd_info.framebuffer[startP++] = 0xDD;
        }
    }
    lcd->parent.control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);

    lcd_set_color(WHITE, 0xDDDDDD);
    int x1 = x + radius - 1;
    int x2 = x + width - radius;
    int y1 = y + radius - 1;
    int y2 = y + height - radius;
    for (int i = 0; i < radius; i++) {
        lcd_draw_circle(x1, y1, i);
        lcd_draw_circle(x1, y2, i);
        lcd_draw_circle(x2, y2, i);
        lcd_draw_circle(x2, y1, i);
    }

    lcd_set_color(0xDDDDDD, 0x0000FF);
    char* text = view.text;
    lcd_show_string(x+10, y + 9, 32, text);
}

void onClickConfirm(View view) {
    rt_kprintf("button click: %s.\n", view.text);
}


void onClickCacncle(View view) {
    rt_kprintf("button click: %s.\n", view.text);
}

void initButton() {
    View btConfirm;
    btConfirm.x = 10;
    btConfirm.y = BUTTON_START;
    btConfirm.width = 100;
    btConfirm.height = 50;
    btConfirm.radius = 10;
    btConfirm.text = " o k ";
    btConfirm.onClick = onClickConfirm;

    drawButton(btConfirm);
    rootView[0] = btConfirm;

    View btCancel;
    btCancel.x = 120;
    btCancel.y = BUTTON_START;
    btCancel.width = 100;
    btCancel.height = 50;
    btCancel.radius = 10;
    btCancel.text = " n o ";
    btCancel.onClick = onClickCacncle;

    drawButton(btCancel);
    rootView[1] = btCancel;
}
/* --------------------------------------------------------------------------------- */
