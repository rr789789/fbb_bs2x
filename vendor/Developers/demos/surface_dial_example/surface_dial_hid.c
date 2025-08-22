#include "surface_dial_hid.h"
#include "securec.h"
#include "osal_debug.h"

#define SURFACE_DIAL_VENDOR_ID 0x1D50 //OpenMoko开源硬件VID
#define SURFACE_DIAL_PRODUCT_ID 0x6D1A // 随便
#define SURFACE_DIAL_RELEASE_NUM 0x0110 // 版本号

static bool g_surface_dial_inited = false;
static int g_surface_dial_hid_index = -1;

int surface_dial_init(void)
{
    if (g_surface_dial_inited)
    {
        return g_surface_dial_hid_index;
    }

    // 设备字符串
    const char manufacturer[] = {'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 's', 0, 'o', 0, 'f', 0, 't', 0};
    struct device_string str_manufacturer = {
        .str = manufacturer,
        .len = sizeof(manufacturer)};

    const char product[] = {'S', 0, 'u', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0, ' ', 0, 'D', 0, 'i', 0, 'a', 0, 'l', 0};
    struct device_string str_product = {
        .str = product,
        .len = sizeof(product)};

    const char serial[] = {'1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0};
    struct device_string str_serial_number = {
        .str = serial,
        .len = sizeof(serial)};
    /*
        这些字符串用于描述USB设备的信息，分别为：厂商，产品，序列号，上述描述为：
        - 厂商：Microsoft
        - 产品：Surface Dial
        - 序列号：12345678
        可以自定义。
    */

    struct device_id dev_id = {
        .vendor_id = SURFACE_DIAL_VENDOR_ID,
        .product_id = SURFACE_DIAL_PRODUCT_ID,
        .release_num = SURFACE_DIAL_RELEASE_NUM};

    // 设置设备信息，使得windows可以识别为Surface Dial
    if (usbd_set_device_info(DEV_HID, &str_manufacturer, &str_product, &str_serial_number, dev_id) != 0)
    {
        osal_printk("Failed to set device info\n");
        return -1;
    }

    // 添加HID报告描述符,注册为HID设备
    g_surface_dial_hid_index = hid_add_report_descriptor(
        g_surface_dial_report_desc,
        sizeof(g_surface_dial_report_desc),
        0);

    if (g_surface_dial_hid_index < 0)
    {
        osal_printk("Failed to add HID report descriptor\n");
        return -1;
    }
    
    

    // 初始化USB
    if (usb_init(DEVICE, DEV_HID) != 0)
    {
        osal_printk("Failed to init USB\n");
        return -1;
    }

    g_surface_dial_inited = true;
    osal_printk("Surface Dial initialized successfully\n");
    return g_surface_dial_hid_index;
}

void surface_dial_deinit(void)
{
    if (!g_surface_dial_inited)
    {
        return;
    }

    usb_deinit();
    g_surface_dial_inited = false;
    g_surface_dial_hid_index = -1;
}

/*
    surface_dial_send_rotation 函数用于向 Surface Dial 设备发送旋转数据。
    它会检查设备是否已初始化并获取有效的 HID 索引，然后通过 fhid_send_data 函数发送包含旋转值的报告，若发送失败则返回 -1，否则返回 0。
*/
int surface_dial_send_rotation(int16_t rotation)//旋转
{
    if (!g_surface_dial_inited || g_surface_dial_hid_index < 0)
    {
        return -1;
    }

    // 根据CompositeHID.c的Dial报告格式，需要发送2字节数据
    // 第一个字节包含按钮状态（1位）+ 旋转值的低7位
    // 第二个字节包含旋转值的高8位

    // 限制旋转值范围到-3600到3600
    if (rotation < -3600)
        rotation = -3600;
    if (rotation > 3600)
        rotation = 3600;

    uint8_t report[3];
    report[0] = 0x05; // Report ID

    // 按钮状态为0，旋转值为15位有符号数
    uint16_t rotation_bits = (uint16_t)(rotation & 0x7FFF);
    report[1] = rotation_bits & 0x7F;        // 低7位，最高位为按钮位（设为0）
    report[2] = (rotation_bits >> 7) & 0xFF; // 高8位

    int32_t ret = fhid_send_data(g_surface_dial_hid_index, (char *)&report, sizeof(report));
    if (ret == -1)
    {
        osal_printk("Failed to send rotation data\n");
        return -1;
    }

    return 0;
}

int surface_dial_send_button(uint8_t pressed)
{
    if (!g_surface_dial_inited || g_surface_dial_hid_index < 0)
    {
        return -1;
    }

    uint8_t report[3];
    report[0] = 0x05; // Report ID

    // 按钮状态在最低位，旋转值为0
    report[1] = pressed ? 0x01 : 0x00; // 按钮状态在bit 0
    report[2] = 0x00;                  // 旋转值高8位为0

    int32_t ret = fhid_send_data(g_surface_dial_hid_index, (char *)&report, sizeof(report));
    if (ret == -1)
    {
        osal_printk("Failed to send button data\n");
        return -1;
    }

    return 0;
}