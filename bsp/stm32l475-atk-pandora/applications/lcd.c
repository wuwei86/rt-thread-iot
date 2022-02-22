/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     armink       first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <drv_lcd.h>
#include <rttlogo.h>

//使用打印接口的时候必须要先使用宏定义在包含#include <rtdbg.h>，才能打印出来,否则没有打印
#define DBG_TAG "LCD"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>

#define THREAD_LCD_PRIORITY         27  //RT_THREAD_PRIORITY_MAX为32，不能超过32否则运行错误
#define THREAD_LCD_STACK_SIZE       1024 //rgbled stack is close to end of stack address 堆栈太小了运行会报错
#define THREAD_LCD_TIMESLICE        5


static rt_thread_t tid_lcd = RT_NULL;

/**********************************************************************************/
/* 线程入口函数 */
static void thread_lcd_entry(void *parameter)
{
    LOG_D("run lcd thread");
    /* 清屏 */
    lcd_clear(WHITE);

    /* 显示 RT-Thread logo */
    lcd_show_image(0, 0, 240, 69, image_rttlogo);
    
    /* 设置背景色和前景色 */
    lcd_set_color(WHITE, BLACK);

    /* 在 LCD 上显示字符 */
    lcd_show_string(10, 69, 16, "Hello, RT-Thread!");
    lcd_show_string(10, 69+16, 24, "RT-Thread");
    //lcd_show_string(10, 69+16+24, 32, "RT-Thread");
    lcd_show_num(10, 69+16+24, 88,2,32);
    
    /* 在 LCD 上画线 */
    lcd_draw_line(0, 69+16+24+32, 240, 69+16+24+32);
    
    /* 在 LCD 上画一个同心圆 */
    lcd_draw_point(120, 194);
    for (int i = 0; i < 46; i += 4)
    {
        lcd_draw_circle(120, 194, i);
    }
    LOG_D("run lcd thread end");
    //系统会自动删除线程不需要调用
    //rt_thread_delete(tid_lcd);
}

/* 删除线程示例的初始化 */
int thread_lcd(void)
{

    /* 创建线程--动态创建*/
    tid_lcd = rt_thread_create("lcd",
                            thread_lcd_entry, RT_NULL,
                            THREAD_LCD_STACK_SIZE,
                            THREAD_LCD_PRIORITY, THREAD_LCD_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid_lcd != RT_NULL)
    {
        rt_thread_startup(tid_lcd);
    }

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_lcd, thread lcd);
