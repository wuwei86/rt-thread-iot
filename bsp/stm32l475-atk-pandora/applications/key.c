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

#include "multi_button.h"

//使用打印接口的时候必须要先使用宏定义在包含#include <rtdbg.h>，才能打印出来,否则没有打印
#define DBG_TAG "KEY"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>

#define THREAD_KEY_PRIORITY         29  //RT_THREAD_PRIORITY_MAX为32，不能超过32否则运行错误
#define THREAD_KEY_STACK_SIZE       256 //rgbled stack is close to end of stack address 堆栈太小了运行会报错
#define THREAD_KEY_TIMESLICE        50

/* 配置 LED 灯引脚 */
#define KEY_PIN_0              PIN_KEY0
#define KEY_PIN_1              PIN_KEY1
#define KEY_PIN_2              PIN_KEY2
#define KEY_PIN_WKUP           PIN_WK_UP


static rt_thread_t tid_key = RT_NULL;

/*********************************按键0********************************************/
static struct button btn0;

static uint8_t button0_read_pin(void)
{
    return rt_pin_read(KEY_PIN_0);
}

static void button0_callback(void *btn)
{
    uint32_t btn_event_val;

    btn_event_val = get_button_event((struct button *)btn);

    switch(btn_event_val)
    {
        case PRESS_DOWN:
            LOG_D("button0 press down");
        break;

        case PRESS_UP:
            LOG_D("button0 press up");
        break;

        case PRESS_REPEAT:
            LOG_D("button0 press repeat");
        break;

        case SINGLE_CLICK:
            LOG_D("button0 single click");
        break;

        case DOUBLE_CLICK:
            LOG_D("button0 double click");
        break;

        case LONG_PRESS_START:
            LOG_D("button0 long press start");
        break;

        case LONG_PRESS_HOLD:
            LOG_D("button0 long press hold");
        break;
    }
}

static void button0_init(void)
{
    button_init  (&btn0, button0_read_pin, PIN_LOW);
    button_attach(&btn0, PRESS_DOWN,       button0_callback);
    button_attach(&btn0, PRESS_UP,         button0_callback);
    button_attach(&btn0, PRESS_REPEAT,     button0_callback);
    button_attach(&btn0, SINGLE_CLICK,     button0_callback);
    button_attach(&btn0, DOUBLE_CLICK,     button0_callback);
    button_attach(&btn0, LONG_PRESS_START, button0_callback);
    button_attach(&btn0, LONG_PRESS_HOLD,  button0_callback);
    button_start (&btn0); 
}
/**********************************************************************************/

/*********************************按键1********************************************/


/**********************************************************************************/

/* 线程入口函数 */
static void thread_key_entry(void *parameter)
{
    while (1)
    {
        button_ticks();
        //LOG_D("button0 long press hold");
        //延时5ms
        rt_thread_mdelay(10);
    }
}

/* 删除线程示例的初始化 */
int thread_key(void)
{
    rt_pin_mode(KEY_PIN_0,PIN_MODE_INPUT);
    rt_pin_mode(KEY_PIN_1,PIN_MODE_INPUT);
    rt_pin_mode(KEY_PIN_2,PIN_MODE_INPUT);
    rt_pin_mode(KEY_PIN_WKUP,PIN_MODE_INPUT);  

    button0_init();

    /* 创建线程--动态创建*/
    tid_key = rt_thread_create("key",
                            thread_key_entry, RT_NULL,
                            THREAD_KEY_STACK_SIZE,
                            THREAD_KEY_PRIORITY, THREAD_KEY_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid_key != RT_NULL)
    {
        rt_thread_startup(tid_key);
    }

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_key, thread key);
