#include "surface_dial_hid.h"
#include "securec.h"
#include "osal_debug.h"

// USB 设备 ID 定义
#define SURFACE_DIAL_VENDOR_ID   0x1D50    // OpenMoko 开源硬件 VID
#define SURFACE_DIAL_PRODUCT_ID  0x6D1A    // 产品 ID
#define SURFACE_DIAL_RELEASE_NUM 0x0110    // 版本号 1.1.0

// HID 报告相关定义
#define HID_REPORT_ID            0x05      // 报告 ID
#define HID_REPORT_SIZE          3         // 报告大小（字节）
#define ROTATION_VALUE_MASK      0x7FFF    // 旋转值掩码（15位）
#define ROTATION_LOW_7BITS_MASK  0x7F      // 低7位掩码
#define ROTATION_HIGH_8BITS_SHIFT 7        // 高8位移位数
#define ROTATION_MIN_VALUE       -3600     // 最小旋转值
#define ROTATION_MAX_VALUE       3600      // 最大旋转值
#define BUTTON_PRESSED_VALUE     0x01      // 按键按下值
#define BUTTON_RELEASED_VALUE    0x00      // 按键释放值

// 报告数据索引
#define REPORT_ID_INDEX          0         // 报告 ID 索引
#define REPORT_DATA_LOW_INDEX    1         // 数据低字节索引
#define REPORT_DATA_HIGH_INDEX   2         // 数据高字节索引

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
int surface_dial_send_rotation(int16_t rotation)
{
    if (!g_surface_dial_inited || g_surface_dial_hid_index < 0)
    {
        return -1;
    }

    // 限制旋转值范围
    if (rotation < ROTATION_MIN_VALUE)
        rotation = ROTATION_MIN_VALUE;
    if (rotation > ROTATION_MAX_VALUE)
        rotation = ROTATION_MAX_VALUE;

    uint8_t report[HID_REPORT_SIZE];
    report[REPORT_ID_INDEX] = HID_REPORT_ID;

    // 按钮状态为0，旋转值为15位有符号数
    uint16_t rotation_bits = (uint16_t)(rotation & ROTATION_VALUE_MASK);
    report[REPORT_DATA_LOW_INDEX] = rotation_bits & ROTATION_LOW_7BITS_MASK;
    report[REPORT_DATA_HIGH_INDEX] = (rotation_bits >> ROTATION_HIGH_8BITS_SHIFT) & 0xFF;

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

    uint8_t report[HID_REPORT_SIZE];
    report[REPORT_ID_INDEX] = HID_REPORT_ID;
    report[REPORT_DATA_LOW_INDEX] = pressed ? BUTTON_PRESSED_VALUE : BUTTON_RELEASED_VALUE;
    report[REPORT_DATA_HIGH_INDEX] = BUTTON_RELEASED_VALUE;

    int32_t ret = fhid_send_data(g_surface_dial_hid_index, (char *)&report, sizeof(report));
    if (ret == -1)
    {
        osal_printk("Failed to send button data\n");
        return -1;
    }

    return 0;
}