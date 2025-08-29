/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022. All rights reserved.
 *
 * Description: SLE private service register sample of client.
 */
#include "sle_client.h"

uint8_t sle_client_name[20] = "xwtag1"; 
sle_addr_t local_addr[SLE_ADDR_LEN] = { { .addr = {0x78, 0x77, 0x74, 0x61, 0x67, 0x37} } };
uint8_t own_addr[SLE_ADDR_LEN] = {0x78, 0x77, 0x74, 0x61, 0x67, 0x36}; //ASCII "xstag1"-787774616732
uint8_t white_list_addr[6] = {0x12,0x34,0x56,0x78,0x90,0x11}; // ASCII "lkltag"
uint8_t empty_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 空地址
#ifdef CONFIG_LARGE_THROUGHPUT_CLIENT
#define RECV_PKT_CNT 1000
#else
#define RECV_PKT_CNT 1
#endif
static sle_announce_seek_callbacks_t g_seek_cbk = {0};
uint8_t sle_rx_buf[SLE_RX_BUF_LEN] = {0};

uint8_t hex_to_string(const uint8_t *hex_buf, int hex_len, char *str_buf, int str_buf_size)
{
    // 每个字节转换为两位16进制字符，加上一个终止符'\0'
    if (str_buf_size < (hex_len * XSECOND + XONE)) 
    {
        // 缓冲区太小，无法存储完整的字符串
        return 0;
    }
    for (int i = XZERO; i < hex_len; i++) 
    {
        sprintf(&str_buf[i * XSECOND], "%02x", hex_buf[i]);
    }
    // 添加字符串终止符
    str_buf[hex_len * XSECOND] = '\0';
    return 1;
}

void sle_sample_sle_enable_cbk(errcode_t status)
{   
    osal_printk("sle enable\r\n");
    if (status == 0)
    {
        sle_start_scan();
    }
}

void sle_sample_seek_enable_cbk(errcode_t status)
{
    osal_printk("sle seek enable\r\n");
    if (status == 0) 
    {
        return;
    }
}

void sle_sample_seek_disable_cbk(errcode_t status)
{
    osal_printk("seek disable\r\n");
    if (status == 0) 
    {
        return;
    }
}

void sle_sample_seek_result_info_cbk(sle_seek_result_info_t *seek_result_data)
{
    if (seek_result_data == NULL) 
    {
        osal_printk("seek_result_data is NULL\r\n");
        return;
    }
// 信息过滤
    osal_printk("addr:");
    for (size_t i = XZERO; i < XSIX; i++)
    {
        osal_printk("%02x", seek_result_data->addr.addr[i]);
    }
    osal_printk("\r\n");
    // 如果广播是定向的，direct addr是目标设备的地址
    // 如果广播式非定向的，direct addr可能是无效的
    if(memcmp(seek_result_data->direct_addr.addr, empty_addr, SLE_ADDR_LEN) == 1 ||
        memcmp(seek_result_data->direct_addr.addr, local_addr, SLE_ADDR_LEN) == 1)
    {        
        osal_printk("direct addr:");
        for (size_t i = XZERO; i < XSIX; i++)
        {
            osal_printk("%02x", seek_result_data->direct_addr.addr[i]);
        }
        osal_printk("\r\n");  
    }
    return;
}

void sle_sample_seek_cbk_register(void)
{
    g_seek_cbk.seek_enable_cb = sle_sample_seek_enable_cbk;
    g_seek_cbk.seek_disable_cb = sle_sample_seek_disable_cbk;
    g_seek_cbk.seek_result_cb = sle_sample_seek_result_info_cbk;
}

void sle_client_init(void)
{
    sle_addr_t local_address;
    local_address.type = XZERO;
    (void)memcpy_s(local_address.addr, SLE_ADDR_LEN, local_addr, SLE_ADDR_LEN);

    enable_sle();
    // 设置扫描回调函数的参数 //
    sle_sample_seek_cbk_register();
    // 注册扫描回调函数 //
    sle_announce_seek_register_callbacks(&g_seek_cbk);
    sle_set_local_addr(&local_address);         // 设置本地地址
    sle_set_local_name(sle_client_name,XFIF);  // 设置本地名称

}

void sle_start_scan(void)
{
    sle_seek_param_t param = {0};
    param.own_addr_type = 0;               // 本端地址类型    
    param.filter_duplicates = 1;           // 重复过滤开关
    param.seek_filter_policy = 0;          // 扫描设备使用的过滤类型
    param.seek_phys = SLE_SEEK_PHY_1M;     // 扫描设备所使用的PHY    
    param.seek_type[0] = 1;                // 扫描类型
    param.seek_interval[0] = SLE_SEEK_INTERVAL_DEFAULT; // 扫描间隔，单位125ms
    param.seek_window[0] = SLE_SEEK_WINDOW_DEFAULT;     // 扫描窗口，单位125ms
    sle_set_seek_param(&param);
    sle_start_seek();
}

void sle_seek_param(uint16_t seek_interval, uint16_t seek_window)
{
    sle_seek_param_t param = {0};
    param.own_addr_type = 0;
    param.filter_duplicates = 1;
    param.seek_filter_policy = SLE_SEEK_FILTER_ALLOW_ALL;
    param.seek_phys = SLE_SEEK_PHY_1M;
    param.seek_type[0] = SLE_SEEK_ACTIVE;
    param.seek_interval[0] = seek_interval;// 扫描间隔
    param.seek_window[0] = seek_window;// 扫描窗口，扫描时长  *1.25
    

    errcode_t ret = sle_set_seek_param(&param);
    if (ret != ERRCODE_SUCC)
    {
        osal_printk("sle_set_seek_param failed %d\n",ret); 
    }
    else 
    {
        osal_printk("sle_set_seek_param success, interval: %d, window: %d\n", seek_interval, seek_window);
    }
    osal_msleep(OSL100); // 等待扫描停止完成
    sle_start_seek();

    osal_printk("sle set success\n");
}

void sle_progress_task(void)
{
    while (1) 
    {
        osal_msleep(OSL50);
        uapi_watchdog_kick();
        if(strstr((char *)sle_rx_buf, "998877665544") != NULL)
        {
            osal_printk("RGB_State: Green(01)\r\n");
            led_state = XSECOND;
            uapi_pm_wkup_process(XZERO);
           
         }
        else if(strstr((char *)sle_rx_buf, "0202020202") != NULL)
        {
            osal_printk("RGB_State: Blue(02)\r\n");
            led_state = XTHREE;
        }
        else if(strstr((char *)sle_rx_buf, "0303030303") != NULL)
        {
            osal_printk("RGB_State: Red(03)\r\n");
            led_state = XONE;
        }
        else if(strstr((char *)sle_rx_buf, "0404040404") != NULL)
        {
            osal_printk("RGB_State: All Off(00)\r\n");
            led_state = XZERO;
        }

        memset(sle_rx_buf, XZERO, sizeof(sle_rx_buf));
    }
}
