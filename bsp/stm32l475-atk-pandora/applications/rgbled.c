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

#define DBG_TAG "RGBLED"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>

#define THREAD_RGBLED_PRIORITY         30  //RT_THREAD_PRIORITY_MAX为32，不能超过32否则运行错误
#define THREAD_RGBLED_STACK_SIZE       256 //rgbled stack is close to end of stack address 堆栈太小了运行会报错
#define THREAD_RGBLED_TIMESLICE        50

/* 配置 LED 灯引脚 */
#define LED_PIN_R              PIN_LED_R
#define LED_PIN_G              PIN_LED_G
#define LED_PIN_B              PIN_LED_B

/* 定义 LED 亮灭电平 */
#define LED_ON  (0)
#define LED_OFF (1)

/* 定义 8 组 LED 闪灯表，其顺序为 R G B */
//c语言定义动态长度数据
static const rt_uint8_t _blink_tab[][3] =
{
    {LED_ON, LED_ON, LED_ON},
    {LED_OFF, LED_ON, LED_ON},
    {LED_ON, LED_OFF, LED_ON},
    {LED_ON, LED_ON, LED_OFF},
    {LED_OFF, LED_OFF, LED_ON},
    {LED_ON, LED_OFF, LED_OFF},
    {LED_OFF, LED_ON, LED_OFF},
    {LED_OFF, LED_OFF, LED_OFF},
};

static rt_thread_t tid_rgbled = RT_NULL;

/* 线程1的入口函数 */
static void thread_regled_entry(void *parameter)
{
    rt_uint32_t count = 1;
    rt_uint32_t group_num = sizeof(_blink_tab)/sizeof(_blink_tab[0]);
    rt_uint32_t group_current;

    rt_pin_mode(LED_PIN_R,PIN_MODE_OUTPUT);
    rt_pin_mode(LED_PIN_G,PIN_MODE_OUTPUT);
    rt_pin_mode(LED_PIN_B,PIN_MODE_OUTPUT);
    while (1)
    {
        //  /* 获得组编号 */
        group_current = count % group_num;

        /* 控制 RGB 灯*/
        rt_pin_write(PIN_LED_R, _blink_tab[group_current][0]);
        rt_pin_write(PIN_LED_G, _blink_tab[group_current][1]);
        rt_pin_write(PIN_LED_B, _blink_tab[group_current][2]);

        /* 输出 LOG 信息 */
        LOG_D("group: %d | red led [%-3.3s] | green led [%-3.3s] | blue led [%-3.3s]",
            group_current,
            _blink_tab[group_current][0] == LED_ON ? "ON" : "OFF",
            _blink_tab[group_current][1] == LED_ON ? "ON" : "OFF",
            _blink_tab[group_current][2] == LED_ON ? "ON" : "OFF");

        // /* 延时一段时间 */
         rt_thread_mdelay(500);
         count++;
    }
}

/* 删除线程示例的初始化 */
int thread_rgbled(void)
{
    /* 创建线程--动态创建*/
    tid_rgbled = rt_thread_create("rgbled",
                            thread_regled_entry, RT_NULL,
                            THREAD_RGBLED_STACK_SIZE,
                            THREAD_RGBLED_PRIORITY, THREAD_RGBLED_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid_rgbled != RT_NULL)
    {
        rt_thread_startup(tid_rgbled);
    }
        

    // /* 初始化线程--静态创建 */
    // rt_thread_init(&thread2,
    //                "thread2",
    //                thread2_entry,
    //                RT_NULL,
    //                &thread2_stack[0],
    //                sizeof(thread2_stack),
    //                THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    // rt_thread_startup(&thread2);

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_rgbled, thread rgbled);
