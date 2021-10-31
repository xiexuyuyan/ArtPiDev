/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include <wlan_mgnt.h>

#include "drv_spi_ili9488.h"
#include "window.h"
#include "mlog.h"

#define LED_PIN GET_PIN(I, 8)

void wifi_connect(void) {
    // rt_err_t rt_wlan_connect(const char *ssid, const char *password);
    // rt_wlan_connect("Coder OldWang", "22225555");
    // rt_wlan_connect("VPN", "syrj2030");
    rt_wlan_connect("HUAWEI_Sample", "22225555");
}
MSH_CMD_EXPORT(wifi_connect, wifi_connect);

void printHello() {
    char buf[6] = {'h', 'e', 'l', 'l', 'o', '\0'};
    addNewLineLn(buf);
}

int main(void)
{
    rt_uint32_t count = 1;

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    lcd_clear(WHITE);
    lcd_set_color(WHITE, BLACK);
    lcd_draw_line(0, LOG_BORDER, 480, LOG_BORDER);
    printHello();
    initButton();


    while(count++)
    {
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
    }
    return RT_EOK;
}

#include "stm32h7xx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal QSPI_FLASH */
    SCB->VTOR = QSPI_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);


