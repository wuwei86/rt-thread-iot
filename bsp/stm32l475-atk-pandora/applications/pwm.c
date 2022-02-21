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
#define DBG_TAG "PWM"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>

#define THREAD_PWM_PRIORITY         28  //RT_THREAD_PRIORITY_MAX为32，不能超过32否则运行错误
#define THREAD_PWM_STACK_SIZE       256 //rgbled stack is close to end of stack address 堆栈太小了运行会报错
#define THREAD_PWM_TIMESLICE        50

/* 配置 KEY 引脚 */
#define KEY_PIN_0              PIN_KEY0
#define KEY_PIN_1              PIN_KEY1
#define KEY_PIN_2              PIN_KEY2
#define KEY_PIN_WKUP           PIN_WK_UP


#define MOTOR_PIN_A              PIN_MOTOR_A
#define MOTOR_PIN_B              PIN_MOTOR_B

#define MOTOR_PIN_BEEP           PIN_BEEP

#define LED_PWM_PIN              PIN_LED_B

static rt_thread_t tid_pwm = RT_NULL;

/*********************************按键2********************************************/
static struct button btn2;

static uint8_t button2_read_pin(void)
{
    return rt_pin_read(KEY_PIN_2);
}

static void button2_callback(void *btn)
{
    uint32_t btn_event_val;
    static rt_uint8_t beepCtrl = 0;

    btn_event_val = get_button_event((struct button *)btn);

    switch(btn_event_val)
    {
        case PRESS_DOWN:
            LOG_D("button2 press down");
        break;

        case PRESS_UP:
            LOG_D("button2 press up");
        break;

        case PRESS_REPEAT:
            LOG_D("button2 press repeat");
        break;

        case SINGLE_CLICK:
            LOG_D("button2 single click");
        break;

        case DOUBLE_CLICK:
            LOG_D("button2 double click");
        break;

        case LONG_PRESS_START:
            LOG_D("button2 long press start");
        break;

        case LONG_PRESS_HOLD:
            LOG_D("button2 long press hold");
        break;
    }
}

static void button2_init(void)
{
    button_init  (&btn2, button2_read_pin, PIN_LOW);
    button_attach(&btn2, PRESS_DOWN,       button2_callback);
    button_attach(&btn2, PRESS_UP,         button2_callback);
    button_attach(&btn2, PRESS_REPEAT,     button2_callback);
    button_attach(&btn2, SINGLE_CLICK,     button2_callback);
    button_attach(&btn2, DOUBLE_CLICK,     button2_callback);
    button_attach(&btn2, LONG_PRESS_START, button2_callback);
    button_attach(&btn2, LONG_PRESS_HOLD,  button2_callback);
    button_start (&btn2); 
}


/**********************************************************************************/

/* 线程入口函数 */
static void thread_pwm_entry(void *parameter)
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
int thread_pwm(void)
{
    rt_pin_mode(KEY_PIN_0,PIN_MODE_INPUT);
    rt_pin_mode(KEY_PIN_1,PIN_MODE_INPUT);
    rt_pin_mode(KEY_PIN_2,PIN_MODE_INPUT);
    rt_pin_mode(KEY_PIN_WKUP,PIN_MODE_INPUT);  

    button2_init();

    /* 创建线程--动态创建*/
    tid_pwm = rt_thread_create("pwm",
                            thread_pwm_entry, RT_NULL,
                            THREAD_PWM_STACK_SIZE,
                            THREAD_PWM_PRIORITY, THREAD_PWM_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid_pwm != RT_NULL)
    {
        rt_thread_startup(tid_pwm);
    }

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_pwm, thread pwm);