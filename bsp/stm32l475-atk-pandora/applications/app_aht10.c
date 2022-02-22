/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     armink       first implementation
 */

//aht10温湿度传感器
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <drv_lcd.h>
#include "sensor_asair_aht10.h"

//使用打印接口的时候必须要先使用宏定义在包含#include <rtdbg.h>，才能打印出来,否则没有打印
#define DBG_TAG "AHT10"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>

#define THREAD_AHT10_PRIORITY         26  //RT_THREAD_PRIORITY_MAX为32，不能超过32否则运行错误
#define THREAD_AHT10_STACK_SIZE       512 //rgbled stack is close to end of stack address 堆栈太小了运行会报错
#define THREAD_AHT10_TIMESLICE        50


#define AHT10_I2C_BUS "i2c2"

static rt_thread_t tid_aht10= RT_NULL;



static int rt_hw_aht10_port(void)
{
    struct rt_sensor_config cfg;

    cfg.intf.dev_name = AHT10_I2C_BUS;
    cfg.intf.user_data = (void *)AHT10_I2C_ADDR;
    
    rt_hw_aht10_init("aht10", &cfg);

    return RT_EOK;
}

/**********************************************************************************/
//采用senor框架方式读取温度和湿度
static void thread_sensor_aht10_entry(void *parameter)
{
    rt_device_t humidity_dev;
    rt_device_t temperature_dev;
    struct rt_sensor_data humidity_data;
    struct rt_sensor_data temperature_data;

    rt_hw_aht10_port();

    /* 查找传感器设备 */
    humidity_dev = rt_device_find("humi_aht");
    if (humidity_dev == RT_NULL)
    {
        LOG_D(" The sensor initializes faisssslure");
        return;
    }

    /* 查找传感器设备 */
    temperature_dev = rt_device_find("temp_aht");
    if (temperature_dev == RT_NULL)
    {
        LOG_D(" The sensor initializes faisssslure");
        return;
    }
   
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(humidity_dev, RT_DEVICE_FLAG_RDONLY);
    rt_device_open(temperature_dev, RT_DEVICE_FLAG_RDONLY);

    while (1)
    {
        if (rt_device_read(humidity_dev, 0, &humidity_data, 1) == 1)
        {
            LOG_D("aht0: humidity:%d, timestamp:%5d\n", humidity_data.data.humi, humidity_data.timestamp);
        }

        if (rt_device_read(temperature_dev, 0, &temperature_data, 1) == 1)
        {
            LOG_D("aht0: temperature:%d, timestamp:%5d\n", temperature_data.data.temp, temperature_data.timestamp);
        }
        //LOG_D(" The sensor read ok!");
        rt_thread_mdelay(1000);
    }
}

/**********************************************************************************/
/* 线程入口函数 */
//普通I2c的方式读取
static void thread_aht10_entry(void *parameter)
{
    aht10_device_t dev;
    float humidity, temperature;

    dev = aht10_init(AHT10_I2C_BUS);
    if (dev == RT_NULL)
    {
        LOG_D(" The sensor initializes failure");
        return;
    }
    else
    {
        LOG_D(" The sensor initializes ok!");
    }

    while (1)
    {
        /* read humidity 采集湿度 */
        humidity = aht10_read_humidity(dev);
        LOG_D("humidity   : %d.%d %%\n", (int)humidity, (int)      (humidity * 10) % 10); /* former is integer and behind is decimal */

        /* read temperature 采集温度 */
        temperature = aht10_read_temperature(dev);
        LOG_D("temperature: %d.%d \n", (int)temperature, (int)(temperature * 10) % 10); /* former is integer and behind is decimal */

        rt_thread_mdelay(1000);
    }
}

/* 删除线程示例的初始化 */
int thread_aht10(void)
{
    /* 创建线程--动态创建*/
    tid_aht10 = rt_thread_create("app_aht10",
                            thread_sensor_aht10_entry, RT_NULL,
                            THREAD_AHT10_STACK_SIZE,
                            THREAD_AHT10_PRIORITY, THREAD_AHT10_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid_aht10 != RT_NULL)
    {
        rt_thread_startup(tid_aht10);
    }

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_aht10, thread aht10);
