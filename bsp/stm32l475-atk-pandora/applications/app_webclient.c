/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-04     ChenYong     first implementation
 */
#include <rtdevice.h>
#include <board.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <rtthread.h>
#include <wlan_mgnt.h>
#include <wifi_config.h>
#include <webclient.h>
#include <msh.h>

#define DBG_TAG "app_webclient"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define HTTP_GET_URL "http://www.rt-thread.com/service/rt-thread.txt"
#define HTTP_POST_URL "http://www.rt-thread.com/service/echo"

#define WLAN_SSID "CMCC-g7cN"
#define WLAN_PASSWORD "su7s9cbx"
#define NET_READY_TIME_OUT (rt_tick_from_millisecond(15 * 1000))


#define THREAD_WEB_PRIORITY         23  //RT_THREAD_PRIORITY_MAX为32，不能超过32否则运行错误
#define THREAD_WEB_STACK_SIZE       2048 //rgbled stack is close to end of stack address 堆栈太小了运行会报错
#define THREAD_WEB_TIMESLICE        50

static rt_thread_t tid_webclient= RT_NULL;
static struct rt_semaphore net_ready;
static const char *post_data = "RT-Thread is an open source IoT operating system from China!";

static int webclient_get_data(void);
static int webclient_post_data(void);
static void print_scan_result(struct rt_wlan_scan_result *scan_result);
static void print_wlan_information(struct rt_wlan_info *info);



/* HTTP client download data by GET request */
static int webclient_get_data(void)
{
    unsigned char *buffer = RT_NULL;
    int length = 0;

    length = webclient_request(HTTP_GET_URL, RT_NULL, RT_NULL, &buffer);
    if (length < 0)
    {
        LOG_E("webclient GET request response data error.");
        return -RT_ERROR;
    }

    LOG_D("webclient GET request response data :");
    LOG_D("%s", buffer);

    web_free(buffer);
    return RT_EOK;
}

/* HTTP client upload data to server by POST request */
static int webclient_post_data(void)
{
    unsigned char *buffer = RT_NULL;
    int length = 0;

    length = webclient_request(HTTP_POST_URL, RT_NULL, post_data, &buffer);
    if (length < 0)
    {
        LOG_E("webclient POST request response data error.");
        return -RT_ERROR;
    }

    LOG_D("webclient POST request response data :");
    LOG_D("%s", buffer);

    web_free(buffer);
    return RT_EOK;
}

static void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_sem_release(&net_ready);
}

/* 断开连接回调函数 */
static void wlan_station_disconnect_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    LOG_I("disconnect from the network!");
}

static void print_scan_result(struct rt_wlan_scan_result *scan_result)
{
    char *security;
    int index, num;

    num = scan_result->num;
    /* 有规则的排列扫描到的热点 */
    rt_kprintf("             SSID                      MAC            security    rssi chn Mbps\n");
    rt_kprintf("------------------------------- -----------------  -------------- ---- --- ----\n");
    for (index = 0; index < num; index++)
    {
        rt_kprintf("%-32.32s", &scan_result->info[index].ssid.val[0]);
        rt_kprintf("%02x:%02x:%02x:%02x:%02x:%02x  ",
                   scan_result->info[index].bssid[0],
                   scan_result->info[index].bssid[1],
                   scan_result->info[index].bssid[2],
                   scan_result->info[index].bssid[3],
                   scan_result->info[index].bssid[4],
                   scan_result->info[index].bssid[5]);
        switch (scan_result->info[index].security)
        {
        case SECURITY_OPEN:
            security = "OPEN";
            break;
        case SECURITY_WEP_PSK:
            security = "WEP_PSK";
            break;
        case SECURITY_WEP_SHARED:
            security = "WEP_SHARED";
            break;
        case SECURITY_WPA_TKIP_PSK:
            security = "WPA_TKIP_PSK";
            break;
        case SECURITY_WPA_AES_PSK:
            security = "WPA_AES_PSK";
            break;
        case SECURITY_WPA2_AES_PSK:
            security = "WPA2_AES_PSK";
            break;
        case SECURITY_WPA2_TKIP_PSK:
            security = "WPA2_TKIP_PSK";
            break;
        case SECURITY_WPA2_MIXED_PSK:
            security = "WPA2_MIXED_PSK";
            break;
        case SECURITY_WPS_OPEN:
            security = "WPS_OPEN";
            break;
        case SECURITY_WPS_SECURE:
            security = "WPS_SECURE";
            break;
        default:
            security = "UNKNOWN";
            break;
        }
        rt_kprintf("%-14.14s ", security);
        rt_kprintf("%-4d ", scan_result->info[index].rssi);
        rt_kprintf("%3d ", scan_result->info[index].channel);
        rt_kprintf("%4d\n", scan_result->info[index].datarate / 1000000);
    }
    rt_kprintf("\n");
}

static void print_wlan_information(struct rt_wlan_info *info)
{
    LOG_D("SSID : %-.32s", &info->ssid.val[0]);
    LOG_D("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", info->bssid[0],
          info->bssid[1],
          info->bssid[2],
          info->bssid[3],
          info->bssid[4],
          info->bssid[5]);
    LOG_D("Channel: %d", info->channel);
    LOG_D("DataRate: %dMbps", info->datarate / 1000000);
    LOG_D("RSSI: %d", info->rssi);
}



static void thread_webclient_entry(void *parameter)
{
    int result = RT_EOK;
    struct rt_wlan_info info;
    struct rt_wlan_scan_result *scan_result;

    /* 等待 500 ms 以便 wifi 完成初始化 */
    rt_hw_wlan_wait_init_done(500);

    /* 扫描热点 */
    LOG_D("start to scan ap ...");
    /* 执行同步扫描 */
    scan_result = rt_wlan_scan_sync();
    if (scan_result)
    {
        LOG_D("the scan is complete, results is as follows: ");
        /* 打印扫描结果 */
        print_scan_result(scan_result);
        /* 清除扫描结果 */
        rt_wlan_scan_result_clean();
    }
    else
    {
        LOG_E("not found ap information ");
        return ;
    }

    /* 热点连接 */
    LOG_D("start to connect ap ...");
    rt_sem_init(&net_ready, "net_ready", 0, RT_IPC_FLAG_FIFO);

    /* 注册 wlan ready 回调函数 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    /* 注册 wlan 断开回调函数 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wlan_station_disconnect_handler, RT_NULL);
    result = rt_wlan_connect(WLAN_SSID, WLAN_PASSWORD);
    if (result == RT_EOK)
    {
        rt_memset(&info, 0, sizeof(struct rt_wlan_info));
        /* 获取当前连接热点信息 */
        rt_wlan_get_info(&info);
        LOG_D("station information:");
        print_wlan_information(&info);
        /* 等待成功获取 IP */
        result = rt_sem_take(&net_ready, NET_READY_TIME_OUT);
        if (result == RT_EOK)
        {
            LOG_D("networking ready!");
            msh_exec("ifconfig", rt_strlen("ifconfig"));
        }
        else
        {
            LOG_D("wait ip got timeout!");
        }
        /* 回收资源 */
        rt_wlan_unregister_event_handler(RT_WLAN_EVT_READY);
        rt_sem_detach(&net_ready);
    }
    else
    {
        LOG_E("The AP(%s) is connect failed!", WLAN_SSID);
    }

    rt_thread_mdelay(5000);

     /* HTTP GET 请求发送 */
    webclient_get_data();
    /* HTTP POST 请求发送 */
    webclient_post_data();
    
    //LOG_D("ready to disconect from ap ...");
    //rt_wlan_disconnect();

    /* 自动连接 */
    //LOG_D("start to autoconnect ...");
    /* 初始化自动连接配置 */
    //wlan_autoconnect_init();
    /* 使能 wlan 自动连接 */
    //rt_wlan_config_autoreconnect(RT_TRUE);

   //return 0;

}

/* 删除线程示例的初始化 */
int thread_webclient(void)
{
    /* 创建线程--动态创建*/
    tid_webclient = rt_thread_create("app_webclient",
                            thread_webclient_entry, RT_NULL,
                            THREAD_WEB_STACK_SIZE,
                            THREAD_WEB_PRIORITY, THREAD_WEB_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid_webclient != RT_NULL)
    {
        rt_thread_startup(tid_webclient);
    }

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_webclient, thread web client);
