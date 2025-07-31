
#include "sle_counter_server.h"

static char g_sle_uuid_app_uuid[UUID_LEN_2] = {0x0, 0x0}; // sle server app uuid
static uint8_t g_server_id = 0;                           // sle server id
static uint16_t g_service_handle = 0;                     // sle service handle
static uint16_t g_property_handle = 0;                    // sle ntf property handle

static uint8_t sle_uuid_base[] = {0x37, 0xBE, 0xA8, 0x80, 0xFC, 0x70, 0x11, 0xEA,
                                  0xB7, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* 广播名称 */
static uint8_t g_sle_local_addr[SLE_ADDR_LEN] = {0x04, 0x01, 0x06, 0x08, 0x06, 0x03};
static uint8_t g_sle_local_name[8] = "COUNTER";
static void sle_set_addr(void)
{
    uint8_t *addr = g_sle_local_addr;

    sle_addr_t sle_addr = {0};
    sle_addr.type = 0;
    if (memcpy_s(sle_addr.addr, SLE_ADDR_LEN, addr, SLE_ADDR_LEN) != EOK) {
        osal_printk("sle_set_addr fail \r\n");
    }

    if (sle_set_local_addr(&sle_addr) == ERRCODE_SUCC) {
        osal_printk("sle_set_addr success \r\n");
    }
}

static errcode_t sle_set_default_announce_param(void)
{
    sle_announce_param_t param = {0};
    param.announce_mode = SLE_ANNOUNCE_MODE_CONNECTABLE_SCANABLE;
    param.announce_handle = SLE_ADV_HANDLE_DEFAULT;
    param.announce_gt_role = SLE_ANNOUNCE_ROLE_T_CAN_NEGO;
    param.announce_level = SLE_ANNOUNCE_LEVEL_NORMAL;
    param.announce_channel_map = SLE_ADV_CHANNEL_MAP_DEFAULT;
    param.announce_interval_min = SLE_ADV_INTERVAL_MIN_DEFAULT;
    param.announce_interval_max = SLE_ADV_INTERVAL_MAX_DEFAULT;
    param.conn_interval_min = SLE_CONN_INTV_MIN_DEFAULT;
    param.conn_interval_max = SLE_CONN_INTV_MAX_DEFAULT;
    param.conn_max_latency = SLE_CONN_MAX_LATENCY;
    param.conn_supervision_timeout = SLE_CONN_SUPERVISION_TIMEOUT_DEFAULT;

    if (memcpy_s(param.own_addr.addr, SLE_ADDR_LEN, g_sle_local_addr, SLE_ADDR_LEN) != EOK) {
        osal_printk("[SLE Adv] set sle adv param addr memcpy fail\r\n");
        return ERRCODE_MEMCPY;
    }

    return sle_set_announce_param(param.announce_handle, &param);
}

static errcode_t sle_set_default_announce_data(void)
{
    errcode_t ret = ERRCODE_FAIL;
    sle_announce_data_t data = {0};
    uint8_t adv_handle = SLE_ADV_HANDLE_DEFAULT;
    uint8_t announce_data[SLE_ADV_DATA_LEN_MAX] = {SLE_ADV_DATA_TYPE_DISCOVERY_LEVEL, 1, SLE_ANNOUNCE_LEVEL_NORMAL};
    uint8_t seek_rsp_data[SLE_ADV_DATA_LEN_MAX] = {SLE_ADV_DATA_TYPE_SHORTENED_LOCAL_NAME, sizeof(g_sle_local_name)};

    memcpy_s(&seek_rsp_data[2], SLE_ADV_DATA_LEN_MAX - 2, g_sle_local_name, sizeof(g_sle_local_name));

    data.announce_data = announce_data;

    data.announce_data_len = 3; // 由类型、长度、普通广播flag三位组成

    data.seek_rsp_data = seek_rsp_data;

    data.seek_rsp_data_len = 2 + sizeof(g_sle_local_name); // 由类型、长度、广播名组成

    ret = sle_set_announce_data(adv_handle, &data);

    if (ret == ERRCODE_SUCC) {
        osal_printk("sle_set_default_announce_data success.\r\n");
    } else {
        osal_printk("sle_set_default_announce_data fail.\r\n");
    }
    return ERRCODE_SUCC;
}

void sle_announce_enable_cbk(uint32_t announce_id, errcode_t status)
{
    osal_printk("sle_announce_enable_cbk id:%02x, state:%02x\r\n", announce_id, status);
}

void sle_announce_disable_cbk(uint32_t announce_id, errcode_t status)
{
    osal_printk("sle_announce_disable_cbk id:%02x, state:%02x\r\n", announce_id, status);
}

void sle_announce_terminal_cbk(uint32_t announce_id)
{
    osal_printk("sle_announce_terminal_cbk id:%02x\r\n", announce_id);
}

void sle_enable_cbk(errcode_t status)
{
    osal_printk("sle_enable_cbk status:%02x\r\n", status);
}

void sle_announce_register_cbks(void)
{
    sle_announce_seek_callbacks_t seek_cbks = {0};
    seek_cbks.announce_enable_cb = sle_announce_enable_cbk;
    seek_cbks.announce_disable_cb = sle_announce_disable_cbk;
    seek_cbks.announce_terminal_cb = sle_announce_terminal_cbk;
    sle_announce_seek_register_callbacks(&seek_cbks);
}

errcode_t sle_server_adv_init(void)
{
    osal_printk("sle_server_adv_init\r\n");
    sle_announce_register_cbks();
    sle_set_default_announce_param();
    sle_set_default_announce_data();

    sle_set_addr();

    sle_start_announce(SLE_ADV_HANDLE_DEFAULT);
    osal_printk("sle_start_announce\r\n");
    return ERRCODE_SUCC;
}

static void sle_uuid_set_base(sle_uuid_t *out)
{
    (void)memcpy_s(out->uuid, SLE_UUID_LEN, sle_uuid_base, SLE_UUID_LEN);
    out->len = UUID_LEN_2;
}

static void sle_uuid_setu2(uint16_t u2, sle_uuid_t *out)
{
    sle_uuid_set_base(out);
    out->len = UUID_LEN_2;
    encode2byte_little(&out->uuid[14], u2);
}

static void ssaps_read_request_cbk(uint8_t server_id,
                                   uint16_t conn_id,
                                   ssaps_req_read_cb_t *read_cb_para,
                                   errcode_t status)
{
    osal_printk("ssaps_read_request_cbk server_id:0x%x, conn_id:0x%x, handle:0x%x, type:0x%x, status:0x%x\r\n",
                server_id, conn_id, read_cb_para->handle, read_cb_para->type, status);
}

static void ssaps_write_request_cbk(uint8_t server_id,
                                    uint16_t conn_id,
                                    ssaps_req_write_cb_t *write_cb_para,
                                    errcode_t status)
{
    osal_printk("ssaps_write_request_cbk server_id:0x%x, conn_id:0x%x, handle:0x%x, status:0x%x\r\n", server_id,
                conn_id, write_cb_para->handle, status);

    osal_printk("ssaps_write_request_cbk value: ");
    for (uint16_t idx = 0; idx < write_cb_para->length; idx++) {
        osal_printk("%02x", write_cb_para->value[idx]);
    }
    osal_printk("( %d )\r\n", write_cb_para->length);

    if (status == ERRCODE_SUCC) {
        // example_print_led_state(write_cb_para);
    }
}

errcode_t sle_server_notify_data(const uint8_t *data, uint8_t len)
{
    errcode_t ret;
    ssaps_ntf_ind_by_uuid_t param = {0};
    param.type = SSAP_PROPERTY_TYPE_VALUE;
    param.start_handle = g_service_handle;
    param.end_handle = g_property_handle;
    param.value_len = len;
    param.value = (uint8_t *)osal_vmalloc(len);
    if (param.value == NULL) {
        osal_printk("sle_server_notify_data value malloc fail\r\n");
        return ERRCODE_SLE_FAIL;
    }
    if (memcpy_s(param.value, param.value_len, data, len) != EOK) {
        osal_printk("sle_server_notify_data memcpy fail\r\n");
        osal_vfree(param.value);
        return ERRCODE_SLE_FAIL;
    }
    sle_uuid_setu2(SLE_UUID_SERVER_PROPERTY, &param.uuid);
    ret = ssaps_notify_indicate_by_uuid(g_server_id, 0xffff, &param);
    if (ret != ERRCODE_SLE_SUCCESS) {
        osal_printk("sle_server_notify_data ssaps_notify_indicate_by_uuid fail :%x\r\n", ret);
        osal_vfree(param.value);
        return ret;
    }
    osal_vfree(param.value);
    return ERRCODE_SLE_SUCCESS;
}

static void ssaps_mtu_changed_cbk(uint8_t server_id, uint16_t conn_id, ssap_exchange_info_t *mtu_size, errcode_t status)
{
    osal_printk("ssaps_mtu_changed_cbk server_id:0x%x, conn_id:0x%x, mtu_size:0x%x, status:0x%x\r\n", server_id,
                conn_id, mtu_size->mtu_size, status);
}

static void ssaps_start_service_cbk(uint8_t server_id, uint16_t handle, errcode_t status)
{
    osal_printk("ssaps_start_service_cbk server_id:0x%x, handle:0x%x, status:0x%x\r\n", server_id, handle, status);
}

static errcode_t sle_ssaps_register_cbks(void)
{
    ssaps_callbacks_t ssaps_cbk = {0};
    ssaps_cbk.start_service_cb = ssaps_start_service_cbk;
    ssaps_cbk.mtu_changed_cb = ssaps_mtu_changed_cbk;
    ssaps_cbk.read_request_cb = ssaps_read_request_cbk;
    ssaps_cbk.write_request_cb = ssaps_write_request_cbk;
    return ssaps_register_callbacks(&ssaps_cbk);
}

static errcode_t sle_server_service_add(void)
{
    errcode_t ret = ERRCODE_FAIL;
    sle_uuid_t service_uuid = {0};
    sle_uuid_setu2(SLE_UUID_SERVER_SERVICE, &service_uuid);
    ret = ssaps_add_service_sync(g_server_id, &service_uuid, true, &g_service_handle);
    if (ret != ERRCODE_SUCC) {
        osal_printk(" sle_server_service_add fail, ret:0x%x\r\n", ret);
        return ERRCODE_FAIL;
    }

    osal_printk("sle_server_service_add service_handle: %u\r\n", g_service_handle);

    return ERRCODE_SUCC;
}

static errcode_t sle_server_property_add(void)
{
    errcode_t ret = ERRCODE_FAIL;
    ssaps_property_info_t property = {0};
    ssaps_desc_info_t descriptor = {0};
    uint8_t ntf_value[] = {0x01, 0x0};

    property.permissions = SSAP_PERMISSION_WRITE | SSAP_PERMISSION_READ;
    property.operate_indication =
        SSAP_OPERATE_INDICATION_BIT_WRITE | SSAP_OPERATE_INDICATION_BIT_NOTIFY | SSAP_OPERATE_INDICATION_BIT_READ;
    sle_uuid_setu2(SLE_UUID_SERVER_PROPERTY, &property.uuid);

    ret = ssaps_add_property_sync(g_server_id, g_service_handle, &property, &g_property_handle);
    if (ret != ERRCODE_SUCC) {
        osal_printk("sle_server_property_add ret:0x%x\r\n", ret);
        osal_vfree(property.value);
        return ERRCODE_FAIL;
    }

    osal_printk("sle_server_property_add property_handle: %u\r\n", g_property_handle);

    descriptor.permissions = SSAP_PERMISSION_READ | SSAP_PERMISSION_WRITE;
    descriptor.type = SSAP_DESCRIPTOR_CLIENT_CONFIGURATION;
    descriptor.value = osal_vmalloc(sizeof(ntf_value));
    descriptor.operate_indication = SSAP_OPERATE_INDICATION_BIT_READ | SSAP_OPERATE_INDICATION_BIT_WRITE |
                                    SSAP_OPERATE_INDICATION_BIT_DESCRIPTOR_CLIENT_CONFIGURATION_WRITE;
    descriptor.value_len = sizeof(ntf_value);

    if (descriptor.value == NULL) {
        osal_printk("sle descriptor mem fail\r\n");
        osal_vfree(property.value);
        return ERRCODE_MALLOC;
    }
    if (memcpy_s(descriptor.value, sizeof(ntf_value), ntf_value, sizeof(ntf_value)) != EOK) {
        osal_printk("sle descriptor mem cpy fail\r\n");
        osal_vfree(property.value);
        osal_vfree(descriptor.value);
        return ERRCODE_MEMCPY;
    }
    ret = ssaps_add_descriptor_sync(g_server_id, g_service_handle, g_property_handle, &descriptor);
    if (ret != ERRCODE_SUCC) {
        osal_printk("ssaps_add_descriptor_sync fail, ret:0x%x\r\n", ret);
        osal_vfree(property.value);
        osal_vfree(descriptor.value);
        return ERRCODE_FAIL;
    }
    osal_vfree(property.value);
    osal_vfree(descriptor.value);
    return ERRCODE_SUCC;
}

static errcode_t sle_server_add(void)
{
    errcode_t ret = ERRCODE_FAIL;
    sle_uuid_t app_uuid = {0};

    osal_printk("sle_server_add\r\n");
    app_uuid.len = sizeof(g_sle_uuid_app_uuid);
    if (memcpy_s(app_uuid.uuid, app_uuid.len, g_sle_uuid_app_uuid, sizeof(g_sle_uuid_app_uuid)) != EOK) {
        return ERRCODE_MEMCPY;
    }
    ssaps_register_server(&app_uuid, &g_server_id);

    if (sle_server_service_add() != ERRCODE_SUCC) {
        ssaps_unregister_server(g_server_id);
        return ERRCODE_FAIL;
    }

    if (sle_server_property_add() != ERRCODE_SUCC) {
        ssaps_unregister_server(g_server_id);
        return ERRCODE_FAIL;
    }
    osal_printk("sle add service, server_id:0x%x, service_handle:0x%x, property_handle:0x%x\r\n", g_server_id,
                g_service_handle, g_property_handle);
    ret = ssaps_start_service(g_server_id, g_service_handle);
    if (ret != ERRCODE_SUCC) {
        osal_printk("sle add service fail, ret:0x%x\r\n", ret);
        return ERRCODE_FAIL;
    }
    return ERRCODE_SUCC;
}

static void sle_connect_state_changed_cbk(uint16_t conn_id,
                                          const sle_addr_t *addr,
                                          sle_acb_state_t conn_state,
                                          sle_pair_state_t pair_state,
                                          sle_disc_reason_t disc_reason)
{
    osal_printk(
        "sle_connect_state_changed_cbk conn_id:0x%02x, conn_state:0x%x, pair_state:0x%x, disc_reason:0x%x, "
        "addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
        conn_id, conn_state, pair_state, disc_reason, addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3],
        addr->addr[4], addr->addr[5]);

    if (conn_state == SLE_ACB_STATE_DISCONNECTED) {
        sle_start_announce(SLE_ADV_HANDLE_DEFAULT);
        osal_printk("sle_start_announce after disconnect\r\n");
    }
}

static void sle_pair_complete_cbk(uint16_t conn_id, const sle_addr_t *addr, errcode_t status)
{
    osal_printk("sle_pair_complete_cbk conn_id:0x%02x, status:0x%x, addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n", conn_id,
                status, addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
}

static errcode_t sle_conn_register_cbks(void)
{
    sle_connection_callbacks_t conn_cbks = {0};
    conn_cbks.connect_state_changed_cb = sle_connect_state_changed_cbk;
    conn_cbks.pair_complete_cb = sle_pair_complete_cbk;
    return sle_connection_register_callbacks(&conn_cbks);
}

static int sle_counter_server_task(const char *arg)
{
    unused(arg);

    (void)osal_msleep(5000); /* 延时5000=5s，等待SLE初始化完毕 */

    osal_printk("sle_counter_server_task\r\n");

    /* 使能SLE */
    if (enable_sle() != ERRCODE_SUCC) {
        osal_printk("sle enbale fail !\r\n");
        return -1;
    }

    /* 注册连接管理回调函数 */
    if (sle_conn_register_cbks() != ERRCODE_SUCC) {
        osal_printk("sle conn register cbks fail !\r\n");
        return -1;
    }

    /* 注册 SSAP server 回调函数 */
    if (sle_ssaps_register_cbks() != ERRCODE_SUCC) {
        osal_printk("sle ssaps register cbks fail !\r\n");
        return -1;
    }

    /* 注册Server, 添加Service和property, 启动Service */
    if (sle_server_add() != ERRCODE_SUCC) {
        osal_printk("sle server add fail !\r\n");
        return -1;
    }

    /* 设置设备公开，并公开设备 */
    if (sle_server_adv_init() != ERRCODE_SUCC) {
        osal_printk("sle server adv fail !\r\n");
        return -1;
    }

    uint8_t pressCount = 0;
    while (1) {
        osal_msleep(500); // 延时500毫秒
        if (buttonPressed) {
            uint8_t temp[5] = {0}; // 随便定义一个长度，例如5位
            temp[0] = pressCount;
            pressCount = (pressCount + 1) % 5; // 从0-4轮转
            sle_server_notify_data(temp, 5);   // 这里把5位都发过去
            buttonPressed = false;
        }
    }

    return 0;
}

static void sle_counter_server_entry(void)
{
    osal_task *task_handle = NULL;

    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)sle_counter_server_task, 0, "SLE_COUNTER_SERVER_TASK",
                                      SLE_COUNTER_SERVER_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, SLE_COUNTER_SERVER_TASK_PRIO);
        osal_kfree(task_handle);
    }
    osal_kthread_unlock();
}

/* Run the sle_sudoku_server_entry. */
app_run(sle_counter_server_entry);