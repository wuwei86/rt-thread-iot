/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     armink       first implementation
 */

//这是一款集成光传感器，距离传感器，红外LED的芯片.
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "sensor_lsc_ap3216c.h"
#include "ap3216c.h"

//使用打印接口的时候必须要先使用宏定义在包含#include <rtdbg.h>，才能打印出来,否则没有打印
#define DBG_TAG "AP3216C"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>

#define THREAD_AP3216C_PRIORITY         25  //RT_THREAD_PRIORITY_MAX为32，不能超过32否则运行错误
#define THREAD_AP3216C_STACK_SIZE       1024 //rgbled stack is close to end of stack address 堆栈太小了运行会报错
#define THREAD_AP3216C_TIMESLICE        50


#define AP3216C_I2C_BUS  "i2c1"

static rt_thread_t tid_ap3216c= RT_NULL;




static int rt_hw_ap3216c_port(void)
{
    struct rt_sensor_config cfg;

    cfg.intf.dev_name  = AP3216C_I2C_BUS;
    cfg.intf.user_data = (void *)AP3216C_I2C_ADDR;

    rt_hw_ap3216c_init("ap3216c", &cfg);

    return RT_EOK;
}

/**********************************************************************************/
//采用senor框架方式读取温度和湿度
static void thread_sensor_ap3216c_entry(void *parameter)
{
    rt_device_t ps_dev;
    rt_device_t als_dev;
    struct rt_sensor_data ps_data;
    struct rt_sensor_data als_data;

    rt_hw_ap3216c_port();

    /* 查找传感器设备 */
    ps_dev = rt_device_find("pr_ap3216c");

    //设备名称:pr_ap3216c: pr + ap3216c

    // static char *const sensor_name_str[] =
    // {
    //     "none",
    //     "acce_",     /* Accelerometer     */
    //     "gyro_",     /* Gyroscope         */
    //     "mag_",      /* Magnetometer      */
    //     "temp_",     /* Temperature       */
    //     "humi_",     /* Relative Humidity */
    //     "baro_",     /* Barometer         */
    //     "li_",       /* Ambient light     */
    //     "pr_",       /* Proximity         */
    //     "hr_",       /* Heart Rate        */
    //     "tvoc_",     /* TVOC Level        */
    //     "noi_",      /* Noise Loudness    */
    //     "step_"      /* Step sensor       */
    //     "forc_"      /* Force sensor      */
    // };

    if (ps_dev == RT_NULL)
    {
        LOG_D(" The sensor initializes faisssslure");
        return;
    }

    /* 查找传感器设备 */
    als_dev = rt_device_find("li_ap3216c");
    if (als_dev == RT_NULL)
    {
        LOG_D(" The sensor initializes faisssslure");
        return;
    }
   
    LOG_D(" The sensor initializes success");
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(ps_dev, RT_DEVICE_FLAG_RDONLY);
    rt_device_open(als_dev, RT_DEVICE_FLAG_RDONLY);
    LOG_D(" The sensor open success");

    while (1)
    {
        if (rt_device_read(ps_dev, 0, &ps_data, 1) == 1)
        {
            LOG_D("current ps data: %d", ps_data.data.proximity);
        }

        if (rt_device_read(als_dev, 0, &als_data, 1) == 1)
        {
            //LOG_D("aht0: temperature:%d, timestamp:%5d\n", temperature_data.data.temp, temperature_data.timestamp);
            LOG_D("current brightness: %d.%d(lux)", (int)als_data.data.light, ((int)(10 * als_data.data.light) % 10));
        }
        //LOG_D(" The sensor read ok!");
        rt_thread_mdelay(1000);
    }
}

/* 删除线程示例的初始化 */
int thread_ap3216c(void)
{
    /* 创建线程--动态创建*/
    tid_ap3216c = rt_thread_create("ap3216c",
                            thread_sensor_ap3216c_entry, RT_NULL,
                            THREAD_AP3216C_STACK_SIZE,
                            THREAD_AP3216C_PRIORITY, THREAD_AP3216C_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid_ap3216c != RT_NULL)
    {
        rt_thread_startup(tid_ap3216c);
    }

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_ap3216c, thread ap3216c);

