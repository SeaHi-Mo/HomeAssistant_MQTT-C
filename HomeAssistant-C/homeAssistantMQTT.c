
/**
 * @file homeAssistantMQTT.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-03
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include <task.h>
#include <assert.h>
#include <stdarg.h>
#include "time.h"
#include "blog.h"
// #include "aiio_wifi.h"
#include "homeAssistantPort.h"
#include "cJSON.h"
#define DBG_TAG "homeAssistantMQTT"

static homeAssisatnt_device_t *ha_device;

static uint8_t STA_MAC[6] = {0};
char *unique_id = NULL;

static cJSON *homeAssistant_device_create(void)
{
    if (ha_device == NULL)
    {
        HA_LOG_E("device is NULL\r\n");
        return NULL;
    }
    cJSON *root = cJSON_CreateObject();
    if (ha_device->name != NULL)
        cJSON_AddStringToObject(root, "name", ha_device->name);
    if (ha_device->hw_version != NULL)
        cJSON_AddStringToObject(root, "hw_version", ha_device->hw_version);
    if (ha_device->identifiers != NULL)
    {
        cJSON *identifiers = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "identifiers", identifiers);
        cJSON_AddItemToArray(identifiers, cJSON_CreateString(ha_device->identifiers));
    }
    if (ha_device->manufacturer != NULL)
        cJSON_AddStringToObject(root, "manufacturer", ha_device->manufacturer);
    if (ha_device->model != NULL)
        cJSON_AddStringToObject(root, "model", ha_device->model);
    if (ha_device->suggest_area != NULL)
        cJSON_AddStringToObject(root, "suggest_area", ha_device->suggest_area);
    if (ha_device->sw_version != NULL)
        cJSON_AddStringToObject(root, "sw_version", ha_device->sw_version);
    if (ha_device->via_device != NULL)
        cJSON_AddStringToObject(root, "via_device", ha_device->via_device);

    cJSON *connencts = cJSON_CreateArray();
    cJSON *MAC = cJSON_CreateArray();
    cJSON_AddItemToArray(connencts, MAC);
    char *mac_str = pvPortMalloc(strlen(":::::") + 12);
    memset(mac_str, 0, strlen(":::::") + 12);
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
    cJSON_AddItemToArray(MAC, cJSON_CreateString("mac"));
    cJSON_AddItemToArray(MAC, cJSON_CreateString(mac_str));
    cJSON_AddItemToObject(root, "connections", connencts);
    vPortFree(mac_str);
    return root;
}
/**
 * @brief homeAssistant_create_switch_data 创建开关实体配置数据
 *
 * @param switch_entity
 * @param device_json
 */
#if CONFIG_ENTITY_ENABLE_SWITCH
static void homeAssistant_create_switch_data(ha_sw_entity_t *switch_entity, cJSON *device_json)
{
    if (switch_entity == NULL)
    {
        HA_LOG_E("entity switch buff is NULL\r\n");
        return;
    }
    cJSON *root = cJSON_CreateObject();
    if (switch_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", switch_entity->name);
    if (switch_entity->device_class != NULL)
        cJSON_AddStringToObject(root, "device_class", switch_entity->device_class);

    if (unique_id == NULL && switch_entity->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(switch_entity->unique_id) + 5);
        memset(unique_id, 0, strlen(switch_entity->unique_id) + 5);
        sprintf(unique_id, "%s-%02x%02x", switch_entity->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    cJSON_AddStringToObject(root, "unique_id", unique_id);

    if (switch_entity->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", switch_entity->object_id);

    if (switch_entity->command_topic == NULL)
    {
        switch_entity->command_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        memset(switch_entity->command_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        sprintf(switch_entity->command_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/set", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    cJSON_AddStringToObject(root, "command_topic", switch_entity->command_topic);
    if (switch_entity->state_topic == NULL)
    {
        switch_entity->state_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///state"));
        memset(switch_entity->state_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///state"));
        sprintf(switch_entity->state_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/state", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    cJSON_AddStringToObject(root, "state_topic", switch_entity->state_topic);
    if (switch_entity->payload_off == NULL)
    {
        switch_entity->payload_off = pvPortMalloc(3);
        memset(switch_entity->payload_off, 0, 3);
        sprintf(switch_entity->payload_off, "%s", "OFF");
    }

    if (switch_entity->payload_on == NULL)
    {
        switch_entity->payload_on = pvPortMalloc(2);
        memset(switch_entity->payload_on, 0, 2);
        sprintf(switch_entity->payload_on, "%s", "ON");
    }

    cJSON_AddStringToObject(root, "payload_off", switch_entity->payload_off);
    cJSON_AddStringToObject(root, "payload_on", switch_entity->payload_on);

    if (switch_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", switch_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (switch_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", switch_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (switch_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", switch_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);
    if (switch_entity->device_class != NULL)
        cJSON_AddStringToObject(root, "device_class", switch_entity->device_class);
    if (switch_entity->qos)
        cJSON_AddNumberToObject(root, "qos", switch_entity->qos);
    if (switch_entity->retain)
        cJSON_AddTrueToObject(root, "retain");
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    if (switch_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", switch_entity->icon);

    switch_entity->config_data = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);
}

/**
 * @brief entity_swith_add_node
 *         添加一个开关实体
 * @param switch_new_node
 */
static void entity_swith_add_node(ha_sw_entity_t *switch_new_node)
{
    assert(ha_device->entity_switch->switch_list);

    // 添加新的节点
    ha_sw_entity_t *switch_list_handle = ha_device->entity_switch->switch_list->prev;

    // 开始创建新的实体数据
    homeAssistant_create_switch_data(switch_new_node, homeAssistant_device_create());
    if (switch_new_node->entity_config_topic == NULL)
    {
        switch_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SWITCH) + strlen(unique_id) + strlen("///config"));
        memset(switch_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SWITCH) + strlen(unique_id) + strlen("///config"));
        sprintf(switch_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SWITCH, unique_id);
    }

    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(switch_new_node->entity_config_topic, switch_new_node->config_data, 0, 1);
        if (switch_new_node->command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(switch_new_node->command_topic, 1);
        // 发送初始化状态
        if (switch_new_node->state_topic != NULL)
            homeAssistant_mqtt_port_public(switch_new_node->state_topic, switch_new_node->switch_state ? switch_new_node->payload_on : switch_new_node->payload_off, 0, 1);
        if (switch_new_node->availability_topic != NULL)
            homeAssistant_mqtt_port_public(switch_new_node->availability_topic, switch_new_node->payload_available, 0, 0);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }
    ha_sw_entity_t *switch_node_buff = pvPortMalloc(sizeof(ha_sw_entity_t));
    memset(switch_node_buff, 0, sizeof(ha_sw_entity_t));
    memcpy(switch_node_buff, switch_new_node, sizeof(ha_sw_entity_t));
    // 插入节点
    switch_list_handle->next = switch_node_buff;
    switch_node_buff->prev = switch_list_handle;
    switch_node_buff->next = ha_device->entity_switch->switch_list;
    ha_device->entity_switch->switch_list->prev = switch_node_buff;
    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(switch_node_buff->config_data);
}

static bool entity_switch_delete_node(char *unique_id)
{
    ha_sw_entity_t *node_to_delete = ha_device->entity_switch->switch_list->next;

    while (node_to_delete != ha_device->entity_switch->switch_list)
    {
        if (node_to_delete->unique_id == NULL)
        {
            HA_LOG_E("%s unique_id is NULL\r\n", CONFIG_HA_ENTITY_SWITCH);
            return false;
        }

        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_switch->switch_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_switch->switch_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }
        if (node_to_delete->command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->command_topic);
            if (isPvPortMallocAllocated(node_to_delete->command_topic))
                vPortFree(node_to_delete->command_topic);
        }
        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);
        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);

        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);

        if (node_to_delete->payload_off != NULL && isPvPortMallocAllocated(node_to_delete->payload_off))
            vPortFree(node_to_delete->payload_off);
        if (node_to_delete->payload_on != NULL && isPvPortMallocAllocated(node_to_delete->payload_on))
            vPortFree(node_to_delete->payload_on);
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}
#endif
/**
 * @brief 创建Light 的config 数据
 *
 * @param light_entity
 * @param device_json
 */
#if CONFIG_ENTITY_ENABLE_LIGHT
static void homeAssistant_create_light_data(ha_lh_entity_t *light_entity, cJSON *device_json)
{
    if (light_entity == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }
    cJSON *root = cJSON_CreateObject();
    if (light_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", light_entity->name);

    if (unique_id == NULL && light_entity->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(light_entity->unique_id) + 5);
        memset(unique_id, 0, strlen(light_entity->unique_id) + 5);
        sprintf(unique_id, "%s-%02x%02x", light_entity->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    cJSON_AddStringToObject(root, "unique_id", unique_id);
    if (light_entity->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", light_entity->object_id);
    if (light_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", light_entity->icon);
    if (light_entity->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", light_entity->availability_template);
    if (light_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", light_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (light_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", light_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (light_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", light_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);

    if (light_entity->command_topic == NULL)
    {
        light_entity->command_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        memset(light_entity->command_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        sprintf(light_entity->command_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/set", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    if (light_entity->state_topic == NULL)
    {
        light_entity->state_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///state"));
        memset(light_entity->state_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///state"));
        sprintf(light_entity->state_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/state", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }

    cJSON_AddStringToObject(root, "command_topic", light_entity->command_topic);
    cJSON_AddStringToObject(root, "state_topic", light_entity->state_topic);
    if (light_entity->payload_off == NULL)
    {
        light_entity->payload_off = pvPortMalloc(3);
        memset(light_entity->payload_off, 0, 3);
        sprintf(light_entity->payload_off, "OFF");
    }
    cJSON_AddStringToObject(root, "payload_off", light_entity->payload_off);

    if (light_entity->payload_on == NULL)
    {
        light_entity->payload_on = pvPortMalloc(2);
        memset(light_entity->payload_on, 0, 2);
        sprintf(light_entity->payload_on, "ON");
    }
    cJSON_AddStringToObject(root, "payload_on", "ON");
    // brightness
    if (light_entity->brightness.brightness_command_template != NULL)
        cJSON_AddStringToObject(root, "brightness_command_template", light_entity->brightness.brightness_command_template);
    if (light_entity->brightness.brightness_command_topic != NULL)
        cJSON_AddStringToObject(root, "brightness_command_topic", light_entity->brightness.brightness_command_topic);
    if (light_entity->brightness.brightness_state_topic != NULL)
        cJSON_AddStringToObject(root, "brightness_state_topic", light_entity->brightness.brightness_state_topic);
    if (light_entity->brightness.brightness_scale != NULL)
        cJSON_AddStringToObject(root, "brightness_scale", light_entity->brightness.brightness_scale);
    if (light_entity->brightness.brightness_value_template != NULL)
        cJSON_AddStringToObject(root, "brightness_value_template", light_entity->brightness.brightness_value_template);
    // rbgw
    if (light_entity->rgbw.rgbw_command_template != NULL)
        cJSON_AddStringToObject(root, "rgbw_command_template", light_entity->rgbw.rgbw_command_template);
    if (light_entity->rgbw.rgbw_command_topic != NULL)
        cJSON_AddStringToObject(root, "rgbw_command_topic", light_entity->rgbw.rgbw_command_topic);
    if (light_entity->rgbw.rgbw_state_topic != NULL)
        cJSON_AddStringToObject(root, "rgbw_state_topic", light_entity->rgbw.rgbw_state_topic);
    if (light_entity->rgbw.rgbw_value_template != NULL)
        cJSON_AddStringToObject(root, "rgbw_value_template", light_entity->rgbw.rgbw_value_template);
    // rbg
    if (light_entity->rgb.rgb_command_template != NULL)
        cJSON_AddStringToObject(root, "rgb_command_template", light_entity->rgb.rgb_command_template);
    if (light_entity->rgb.rgb_command_topic != NULL)
        cJSON_AddStringToObject(root, "rgb_command_topic", light_entity->rgb.rgb_command_topic);
    if (light_entity->rgb.rgb_state_topic != NULL)
        cJSON_AddStringToObject(root, "rgb_state_topic", light_entity->rgb.rgb_state_topic);
    if (light_entity->rgb.rgb_value_template != NULL)
        cJSON_AddStringToObject(root, "rgb_value_template", light_entity->rgb.rgb_value_template);
    // hs
    if (light_entity->hs.hs_command_template != NULL)
        cJSON_AddStringToObject(root, "hs_command_template", light_entity->hs.hs_command_template);
    if (light_entity->hs.hs_command_topic != NULL)
        cJSON_AddStringToObject(root, "hs_command_topic", light_entity->hs.hs_command_topic);
    if (light_entity->hs.hs_state_topic != NULL)
        cJSON_AddStringToObject(root, "hs_state_topic", light_entity->hs.hs_state_topic);
    if (light_entity->hs.hs_value_template != NULL)
        cJSON_AddStringToObject(root, "hs_value_template", light_entity->hs.hs_value_template);
    // rgbww
    if (light_entity->rgbww.rgbww_command_template != NULL)
        cJSON_AddStringToObject(root, "rgbww_command_template", light_entity->rgbww.rgbww_command_template);
    if (light_entity->rgbww.rgbww_command_topic != NULL)
        cJSON_AddStringToObject(root, "rgbww_command_topic", light_entity->rgbww.rgbww_command_topic);
    if (light_entity->rgbww.rgbww_state_topic != NULL)
        cJSON_AddStringToObject(root, "rgbww_state_topic", light_entity->rgbww.rgbww_state_topic);
    if (light_entity->rgbww.rgbww_value_template != NULL)
        cJSON_AddStringToObject(root, "rgbww_value_template", light_entity->rgbww.rgbww_value_template);
    // white
    if (light_entity->white.white_command_topic != NULL)
        cJSON_AddStringToObject(root, "white_command_topic", light_entity->white.white_command_topic);
    if (light_entity->white.white_scale != NULL)
        cJSON_AddStringToObject(root, "white_scale", light_entity->white.white_scale);

    if (light_entity->qos)
        cJSON_AddNumberToObject(root, "qos", light_entity->qos);
    if (light_entity->retain)
        cJSON_AddTrueToObject(root, "retain");
    // 添加设备信息
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);

    light_entity->config_data = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);
}

static void entity_light_add_node(ha_lh_entity_t *light_new_node)
{
    // assert(ha_device->entity_light->light_list);
    ha_lh_entity_t *light_list_handle = ha_device->entity_light->light_list->prev;

    // 开始创建新的实体数据
    homeAssistant_create_light_data(light_new_node, homeAssistant_device_create());
    if (light_new_node->entity_config_topic == NULL)
    {
        light_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_LIGHT) + strlen(unique_id) + strlen("///config"));
        memset(light_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_LIGHT) + strlen(unique_id) + strlen("///config"));
        sprintf(light_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_LIGHT, unique_id);
    }
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(light_new_node->entity_config_topic, light_new_node->config_data, 0, 1);
        if (light_new_node->command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(light_new_node->command_topic, 1);
        // 发送初始化开关状态
        if (light_new_node->state_topic != NULL)
        {
            homeAssistant_mqtt_port_public(light_new_node->state_topic, light_new_node->light_state ? light_new_node->payload_on : light_new_node->payload_off, 0, 1);
        }
        // 发送初始化RGB的值
        if (light_new_node->rgb.rgb_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(light_new_node->rgb.rgb_command_topic, 1);
        if (light_new_node->light_state && (light_new_node->rgb.red != 0 || light_new_node->rgb.green != 0 || light_new_node->rgb.blue != 0))
        {
            char *rgb_data = pvPortMalloc(16);
            sprintf(rgb_data, "%d,%d,%d", light_new_node->rgb.red, light_new_node->rgb.green, light_new_node->rgb.blue);
            homeAssistant_mqtt_port_public(light_new_node->rgb.rgb_state_topic, rgb_data, 0, 1);
            vPortFree(rgb_data);
        }
        // 发送初始化亮度的值
        if (light_new_node->brightness.brightness_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(light_new_node->brightness.brightness_command_topic, 1);
        if (light_new_node->light_state && light_new_node->brightness.brightness != 0)
            homeAssistant_mqtt_port_public(light_new_node->brightness.brightness_state_topic, light_new_node->brightness.brightness, 0, 1);
        // 发送初始化 HS 的值
        if (light_new_node->hs.hs_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(light_new_node->hs.hs_command_topic, 1);
        if (light_new_node->light_state && (light_new_node->hs.hue != 0 || light_new_node->hs.saturation != 0))
        {
            char *hs_data = pvPortMalloc(16);
            sprintf(hs_data, "%0.3f,%0.3f", light_new_node->hs.hue, light_new_node->hs.saturation);
            homeAssistant_mqtt_port_public(light_new_node->hs.hs_state_topic, hs_data, 0, 1);
            vPortFree(hs_data);
        }
        if (light_new_node->availability_topic != NULL)
            homeAssistant_mqtt_port_public(light_new_node->availability_topic, light_new_node->payload_available, 0, 0);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }

    ha_lh_entity_t *light_node_buff = pvPortMalloc(sizeof(ha_lh_entity_t));
    memset(light_node_buff, 0, sizeof(ha_lh_entity_t));
    memcpy(light_node_buff, light_new_node, sizeof(ha_lh_entity_t));
    // 插入节点
    light_list_handle->next = light_node_buff;
    light_node_buff->prev = light_list_handle;
    light_node_buff->next = ha_device->entity_light->light_list;
    ha_device->entity_light->light_list->prev = light_node_buff;
    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(light_new_node->config_data);
}

static bool entity_light_delete_node(char *unique_id)
{
    /* 找到被删除的实体 */
    ha_lh_entity_t *node_to_delete = ha_device->entity_light->light_list->next;

    while (node_to_delete != ha_device->entity_light->light_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_light->light_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_light->light_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        // HA_LOG_D("delete entity %s from homeAssistant...\r\n", node_to_delete->unique_id);
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }
        // HA_LOG_D("delete entity %s from homeAssistant OK\r\n", node_to_delete->unique_id);
        if (node_to_delete->command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->command_topic);
            if (isPvPortMallocAllocated(node_to_delete->command_topic))
                vPortFree(node_to_delete->command_topic);
        }

        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);

        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);

        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);

        if (node_to_delete->payload_off != NULL && isPvPortMallocAllocated(node_to_delete->payload_off))
            vPortFree(node_to_delete->payload_off);

        if (node_to_delete->payload_on != NULL && isPvPortMallocAllocated(node_to_delete->payload_on))
            vPortFree(node_to_delete->payload_on);
        /**
         * @brief RGB 控制信息
         *
         */
        if (node_to_delete->rgb.rgb_command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->rgb.rgb_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->rgb.rgb_command_topic))
                vPortFree(node_to_delete->rgb.rgb_command_topic);
        }

        if (node_to_delete->rgb.rgb_value_template != NULL && isPvPortMallocAllocated(node_to_delete->rgb.rgb_value_template))
            vPortFree(node_to_delete->rgb.rgb_value_template);

        if (node_to_delete->rgb.rgb_command_template != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->rgb.rgb_command_template);
            if (isPvPortMallocAllocated(node_to_delete->rgb.rgb_command_template))
                vPortFree(node_to_delete->rgb.rgb_command_template);
        }
        if (node_to_delete->rgb.rgb_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->rgb.rgb_state_topic))
            vPortFree(node_to_delete->rgb.rgb_state_topic);
        /**
         * @brief HS 控制信息
         *
         */
        if (node_to_delete->hs.hs_command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->hs.hs_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->hs.hs_command_topic))
                vPortFree(node_to_delete->hs.hs_command_topic);
        }
        if (node_to_delete->hs.hs_command_template != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->hs.hs_command_template);
            if (isPvPortMallocAllocated(node_to_delete->hs.hs_command_template))
                vPortFree(node_to_delete->hs.hs_command_template);
        }
        if (node_to_delete->hs.hs_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->hs.hs_state_topic))
            vPortFree(node_to_delete->hs.hs_state_topic);
        if (node_to_delete->hs.hs_value_template != NULL && isPvPortMallocAllocated(node_to_delete->hs.hs_value_template))
            vPortFree(node_to_delete->hs.hs_value_template);

        /**
         * @brief 亮度控制信息
         *
         */
        if (node_to_delete->brightness.brightness_command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->brightness.brightness_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->brightness.brightness_command_topic))
                vPortFree(node_to_delete->brightness.brightness_command_topic);
        }
        if (node_to_delete->brightness.brightness_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->brightness.brightness_state_topic))
            vPortFree(node_to_delete->brightness.brightness_state_topic);
        if (node_to_delete->brightness.brightness_value_template != NULL && isPvPortMallocAllocated(node_to_delete->brightness.brightness_value_template))
            vPortFree(node_to_delete->brightness.brightness_value_template);
        if (node_to_delete->brightness.brightness_command_template != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->brightness.brightness_command_template);
            if (isPvPortMallocAllocated(node_to_delete->brightness.brightness_command_template))
                vPortFree(node_to_delete->brightness.brightness_command_template);
        }
        /**
         * @brief 色温控制信息
         *
         */
        if (node_to_delete->color_temp.color_temp_command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->color_temp.color_temp_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->color_temp.color_temp_command_topic))
                vPortFree(node_to_delete->color_temp.color_temp_command_topic);
        }
        if (node_to_delete->color_temp.color_temp_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->color_temp.color_temp_state_topic))
            vPortFree(node_to_delete->color_temp.color_temp_state_topic);
        if (node_to_delete->color_temp.color_temp_value_template != NULL && isPvPortMallocAllocated(node_to_delete->color_temp.color_temp_value_template))
            vPortFree(node_to_delete->color_temp.color_temp_value_template);
        if (node_to_delete->color_temp.color_temp_command_template != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->color_temp.color_temp_command_template);
            if (isPvPortMallocAllocated(node_to_delete->color_temp.color_temp_command_template))
                vPortFree(node_to_delete->color_temp.color_temp_command_template);
        }
        /**
         * @brief RGB+白光控制信息
         *
         */

        if (node_to_delete->rgbw.rgbw_command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->rgbw.rgbw_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->rgbw.rgbw_command_topic))
                vPortFree(node_to_delete->rgbw.rgbw_command_topic);
        }
        if (node_to_delete->rgbw.rgbw_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->rgbw.rgbw_state_topic))
            vPortFree(node_to_delete->rgbw.rgbw_state_topic);
        if (node_to_delete->rgbw.rgbw_value_template != NULL && isPvPortMallocAllocated(node_to_delete->rgbw.rgbw_value_template))
            vPortFree(node_to_delete->rgbw.rgbw_value_template);
        if (node_to_delete->rgbw.rgbw_command_template != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->rgbw.rgbw_command_template);
            if (isPvPortMallocAllocated(node_to_delete->rgbw.rgbw_command_template))
                vPortFree(node_to_delete->rgbw.rgbw_command_template);
        }
        /**
         * @brief RGB+双白光控制信息
         *
         */
        if (node_to_delete->rgbww.rgbww_command_topic != NULL)
        {

            homeAssistant_mqtt_port_unsubscribe(node_to_delete->rgbww.rgbww_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->rgbww.rgbww_command_topic))
                vPortFree(node_to_delete->rgbww.rgbww_command_topic);
        }
        if (node_to_delete->rgbww.rgbww_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->rgbww.rgbww_state_topic))
            vPortFree(node_to_delete->rgbww.rgbww_state_topic);
        if (node_to_delete->rgbww.rgbww_value_template != NULL && isPvPortMallocAllocated(node_to_delete->rgbww.rgbww_value_template))
            vPortFree(node_to_delete->rgbww.rgbww_value_template);
        if (node_to_delete->rgbww.rgbww_command_template != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->rgbww.rgbww_command_template);
            if (isPvPortMallocAllocated(node_to_delete->rgbww.rgbww_command_template))
                vPortFree(node_to_delete->rgbww.rgbww_command_template);
        }

        /**
         * @brief 白光状态信息
         *
         */
        if (node_to_delete->white.white_command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->white.white_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->white.white_command_topic))
                vPortFree(node_to_delete->white.white_command_topic);
        }
        if (node_to_delete->white.white_scale != NULL && isPvPortMallocAllocated(node_to_delete->white.white_scale))
            vPortFree(node_to_delete->white.white_scale);
    }
    // HA_LOG_D("delete entity %s is OK\r\n", node_to_delete->unique_id);
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);
    node_to_delete = NULL;
    return true;
}
/**
 * @brief homeAssistant_get_light_rgb
 *        获取RGB 值，适用于没有使用格式要求的情况，简单解析出RGB三基色值
 * @param light_entity
 * @param rgb_data
 */
static void homeAssistant_get_light_rgb(ha_lh_entity_t *light_entity, const char *rgb_data, unsigned short data_len)
{
    if (light_entity == NULL || rgb_data == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }
    char *data_str = pvPortMalloc(data_len);
    memset(data_str, 0, data_len);
    sprintf(data_str, "%.*s", data_len, rgb_data);
    HA_LOG_F("rgb_data %s\r\n", data_str);
    if (light_entity->rgb.rgb_command_template == NULL)
    {
        char *rgb = strtok(data_str, ",");
        if (rgb)
            light_entity->rgb.red = atoi(rgb);
        rgb = strtok(NULL, ",");
        if (rgb)
            light_entity->rgb.green = atoi(rgb);
        rgb = strtok(NULL, ",");
        if (rgb)
            light_entity->rgb.blue = atoi(rgb);
    }
    vPortFree(data_str);
}
/**
 * @brief homeAssistant_get_light_hs
 *        获取HS 值，适用于没有使用格式要求的情况，简单解析出HS值
 * @param light_entity
 * @param hs_data
 * @param data_len
 */
static void homeAssistant_get_light_hs(ha_lh_entity_t *light_entity, const char *hs_data, unsigned short data_len)
{
    if (light_entity == NULL || hs_data == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }
    char *data_str = pvPortMalloc(data_len);
    memset(data_str, 0, data_len);
    sprintf(data_str, "%.*s", data_len, hs_data);
    HA_LOG_F("hs_data %s\r\n", data_str);

    if (light_entity->hs.hs_command_template == NULL)
    {
        char *hs = strtok(data_str, ",");
        if (hs)
            light_entity->hs.hue = atof(hs);
        hs = strtok(NULL, ",");
        if (hs)
            light_entity->hs.saturation = atof(hs);
    }
    vPortFree(data_str);
}

static void homeAssistant_get_light_brightness(ha_lh_entity_t *light_entity, const char *brightness_data, unsigned short data_len)
{
    if (light_entity == NULL || brightness_data == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }
    light_entity->brightness.brightness = atoi(brightness_data);
}

#endif
/**
 * @brief 传感器开关实体
 *
 */
#if CONFIG_ENTITY_ENABLE_SENSOR
static char *sensor_class_type[] = {"None", "apparent_power", "aqi", "area", "atmospheric_pressure", "battery", "blood_glucose_concentration",
                                    "carbon_dioxide", "carbon_monoxide", "current", "data_rate", "data_size", "date", "distance", "duration",
                                    "energy", "energy_storage", "enum", "frequency", "gas", "humidity", "illuminance", "irradiance", "moisture",
                                    "monetary", "nitrogen_dioxide", "nitrogen_monoxide", "nitrous_oxide", "ozone", "ph", "pm1", "pm25", "pm10",
                                    "power_factor", "power", "precipitation", "precipitation_intensity", "pressure", "reactive_power", "signal_strength",
                                    "sound_pressure", "speed", "sulphur_dioxide", "temperature", "timestamp", "volatile_organic_compounds", "volatile_organic_compounds_parts",
                                    "voltage", "volume", "volume_flow_rate", "volume_storage", "water", "weight", "wind_speed"};

static void homeAssistant_create_sensor_data(ha_sensor_entity_t *sensor_entity, cJSON *device_json)
{
    if (sensor_entity == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }

    cJSON *root = cJSON_CreateObject();
    if (sensor_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", sensor_entity->name);
    if (unique_id == NULL && sensor_entity->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(sensor_entity->unique_id) + 5);
        memset(unique_id, 0, strlen(sensor_entity->unique_id) + 5);
        sprintf(unique_id, "%s-%02x%02x", sensor_entity->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    cJSON_AddStringToObject(root, "unique_id", unique_id);

    if (sensor_entity->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", sensor_entity->object_id);
    if (sensor_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", sensor_entity->icon);
    if (sensor_entity->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", sensor_entity->availability_template);
    if (sensor_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", sensor_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (sensor_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", sensor_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (sensor_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", sensor_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);
    if (sensor_entity->device_class)
        cJSON_AddStringToObject(root, "device_class", sensor_class_type[sensor_entity->device_class]);
    if (sensor_entity->suggested_display_precision)
        cJSON_AddNumberToObject(root, "suggested_display_precision", sensor_entity->suggested_display_precision);
    if (sensor_entity->json_attributes_template != NULL)
        cJSON_AddStringToObject(root, "json_attributes_template", sensor_entity->json_attributes_template);
    if (sensor_entity->json_attributes_topic != NULL)
        cJSON_AddStringToObject(root, "json_attributes_topic", sensor_entity->json_attributes_topic);
    if (sensor_entity->state_class != NULL)
        cJSON_AddStringToObject(root, "state_class", sensor_entity->state_class);
    if (sensor_entity->state_topic == NULL)
    {
        sensor_entity->state_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        memset(sensor_entity->state_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        sprintf(sensor_entity->state_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/state", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    cJSON_AddStringToObject(root, "state_topic", sensor_entity->state_topic);

    if (sensor_entity->unit_of_measurement != NULL)
        cJSON_AddStringToObject(root, "unit_of_measurement", sensor_entity->unit_of_measurement);
    if (sensor_entity->value_template != NULL)
        cJSON_AddStringToObject(root, "value_template", sensor_entity->value_template);
    if (sensor_entity->expire_after)
        cJSON_AddNumberToObject(root, "expire_after", sensor_entity->expire_after);
    if (sensor_entity->force_update)
        cJSON_AddTrueToObject(root, "force_update");
    else
        cJSON_AddFalseToObject(root, "force_update");

    if (sensor_entity->qos)
        cJSON_AddNumberToObject(root, "qos", sensor_entity->qos);
    if (sensor_entity->retain)
        cJSON_AddTrueToObject(root, "retain");
    // 添加设备信息
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    sensor_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void entity_sensor_add_node(ha_sensor_entity_t *sensor_new_node)
{
    if (sensor_new_node == NULL)
        return;
    ha_sensor_entity_t *sensor_list_handle = ha_device->entity_sensor->sensor_list->prev;
    homeAssistant_create_sensor_data(sensor_new_node, homeAssistant_device_create());

    if (sensor_new_node->entity_config_topic == NULL)
    {
        sensor_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SENSOR) + strlen(unique_id) + strlen("///config"));
        memset(sensor_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SENSOR) + strlen(unique_id) + strlen("///config"));
        sprintf(sensor_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SENSOR, unique_id);
    }
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(sensor_new_node->entity_config_topic, sensor_new_node->config_data, 0, 1);
        if (sensor_new_node->availability_topic != NULL)
            homeAssistant_mqtt_port_public(sensor_new_node->availability_topic, sensor_new_node->payload_available, 0, 0);
        if (sensor_new_node->sensor_data != NULL)
            homeAssistant_mqtt_port_public(sensor_new_node->state_topic, sensor_new_node->sensor_data, 0, 1);
        // HA_LOG_D("Topic:%s\r\n", sensor_new_node->entity_config_topic);
        // HA_LOG_D("config data:%s\r\n", sensor_new_node->config_data);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }

    ha_sensor_entity_t *sensor_new_node_temp = pvPortMalloc(sizeof(ha_sensor_entity_t));
    memset(sensor_new_node_temp, 0, sizeof(ha_sensor_entity_t));
    memcpy(sensor_new_node_temp, sensor_new_node, sizeof(ha_sensor_entity_t));

    sensor_list_handle->next = sensor_new_node_temp;
    sensor_new_node_temp->prev = sensor_list_handle;
    sensor_new_node_temp->next = ha_device->entity_sensor->sensor_list;
    ha_device->entity_sensor->sensor_list->prev = sensor_new_node_temp;
    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(sensor_new_node->config_data);
}

static bool entity_sensor_delete_node(char *unique_id)
{
    /* 找到被删除的实体 */
    ha_sensor_entity_t *node_to_delete = ha_device->entity_sensor->sensor_list->next;

    while (node_to_delete != ha_device->entity_sensor->sensor_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_sensor->sensor_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_sensor->sensor_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }
        else
        {
            HA_LOG_E("entity_config_topic is NULL\r\n");
        }

        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);

        if (node_to_delete->availability_template != NULL && isPvPortMallocAllocated(node_to_delete->availability_template))
            vPortFree(node_to_delete->availability_template);

        if (node_to_delete->value_template != NULL && isPvPortMallocAllocated(node_to_delete->value_template))
            vPortFree(node_to_delete->value_template);

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);

        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);

        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);
    }
    else
    {
        HA_LOG_W("MQTT server is diconnenct\r\n");
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}
#endif
/**
 * @brief 二进制传感器
 *
 */
#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
static char *Bsensor_class_type[] = {"None", "battery", "battery_charging", "carbon_monoxide", "cold", "connectivity", "door", "garage_door",
                                     "gas", "heat", "light", "lock", "moisture", "motion", "moving", "occupancy", "opening", "plug", "power", "presence", "problem", "running", "safety", "smoke", "sound", "tamper", "update", "vibration", "window"};

static void homeAssistant_create_binary_sensor_data(ha_Bsensor_entity_t *binary_sensor_entity, cJSON *device_json)
{
    if (binary_sensor_entity == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }

    cJSON *root = cJSON_CreateObject();
    if (binary_sensor_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", binary_sensor_entity->name);
    if (unique_id == NULL && binary_sensor_entity->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(binary_sensor_entity->unique_id) + 5);
        memset(unique_id, 0, strlen(binary_sensor_entity->unique_id) + 5);
        sprintf(unique_id, "%s-%02x%02x", binary_sensor_entity->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    cJSON_AddStringToObject(root, "unique_id", unique_id);

    if (binary_sensor_entity->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", binary_sensor_entity->object_id);
    if (binary_sensor_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", binary_sensor_entity->icon);
    if (binary_sensor_entity->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", binary_sensor_entity->availability_template);

    if (binary_sensor_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", binary_sensor_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (binary_sensor_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", binary_sensor_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (binary_sensor_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", binary_sensor_entity->payload_not_available);

    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);
    if (binary_sensor_entity->device_class)
        cJSON_AddStringToObject(root, "device_class", Bsensor_class_type[binary_sensor_entity->device_class]);
    if (binary_sensor_entity->json_attributes_template != NULL)
        cJSON_AddStringToObject(root, "json_attributes_template", binary_sensor_entity->json_attributes_template);
    if (binary_sensor_entity->json_attributes_topic != NULL)
        cJSON_AddStringToObject(root, "json_attributes_topic", binary_sensor_entity->json_attributes_topic);
    if (binary_sensor_entity->state_class != NULL)
        cJSON_AddStringToObject(root, "state_class", binary_sensor_entity->state_class);

    if (binary_sensor_entity->entity_category != NULL)
        cJSON_AddStringToObject(root, "entity_category", binary_sensor_entity->entity_category);
    if (binary_sensor_entity->state_topic == NULL)
    {
        binary_sensor_entity->state_topic = pvPortMalloc(strlen(ha_device->name) + 12 + strlen(unique_id) + strlen("///state"));
        memset(binary_sensor_entity->state_topic, 0, strlen(ha_device->name) + 12 + strlen(unique_id) + strlen("///state"));
        sprintf(binary_sensor_entity->state_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/state", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    cJSON_AddStringToObject(root, "state_topic", binary_sensor_entity->state_topic);
    if (binary_sensor_entity->payload_on == NULL)
        binary_sensor_entity->payload_on = "ON";
    cJSON_AddStringToObject(root, "payload_on", binary_sensor_entity->payload_on);
    if (binary_sensor_entity->payload_off == NULL)
        binary_sensor_entity->payload_off = "OFF";
    cJSON_AddStringToObject(root, "payload_off", binary_sensor_entity->payload_off);

    if (binary_sensor_entity->value_template != NULL)
        cJSON_AddStringToObject(root, "value_template", binary_sensor_entity->value_template);
    if (binary_sensor_entity->expire_after)
        cJSON_AddNumberToObject(root, "expire_after", binary_sensor_entity->expire_after);
    if (binary_sensor_entity->force_update)
        cJSON_AddTrueToObject(root, "force_update");
    else
        cJSON_AddFalseToObject(root, "force_update");

    if (binary_sensor_entity->qos)
        cJSON_AddNumberToObject(root, "qos", binary_sensor_entity->qos);
    if (binary_sensor_entity->retain)
        cJSON_AddTrueToObject(root, "retain");
    // 添加设备信息
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    binary_sensor_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void entity_binary_sensor_add_node(ha_Bsensor_entity_t *binary_sensor_new_node)
{
    if (binary_sensor_new_node == NULL)
        return;
    ha_Bsensor_entity_t *binary_sensor_list_handle = ha_device->entity_binary_sensor->binary_sensor_list->prev;

    homeAssistant_create_binary_sensor_data(binary_sensor_new_node, homeAssistant_device_create());
    if (binary_sensor_new_node->entity_config_topic == NULL)
    {
        binary_sensor_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_BINARY_SENSOR) + strlen(unique_id) + strlen("///config"));
        memset(binary_sensor_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_BINARY_SENSOR) + strlen(unique_id) + strlen("///config"));
        sprintf(binary_sensor_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_BINARY_SENSOR, unique_id);
    }
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(binary_sensor_new_node->entity_config_topic, binary_sensor_new_node->config_data, 0, 1);
        if (binary_sensor_new_node->availability_topic != NULL)
            homeAssistant_mqtt_port_public(binary_sensor_new_node->availability_topic, binary_sensor_new_node->payload_available, 0, 0);
        // HA_LOG_D("Topic:%s\r\n", binary_sensor_new_node->entity_config_topic);
        // HA_LOG_D("config data:%s\r\n", binary_sensor_new_node->config_data);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }
    ha_Bsensor_entity_t *binary_sensor_new_node_temp = pvPortMalloc(sizeof(ha_Bsensor_entity_t));
    memset(binary_sensor_new_node_temp, 0, sizeof(ha_Bsensor_entity_t));
    memcpy(binary_sensor_new_node_temp, binary_sensor_new_node, sizeof(ha_Bsensor_entity_t));

    binary_sensor_list_handle->next = binary_sensor_new_node_temp;
    binary_sensor_new_node_temp->prev = binary_sensor_list_handle;
    binary_sensor_new_node_temp->next = ha_device->entity_binary_sensor->binary_sensor_list;
    ha_device->entity_binary_sensor->binary_sensor_list->prev = binary_sensor_new_node_temp;
    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(binary_sensor_new_node->config_data);
}

static bool entity_binary_sensor_delete_node(char *unique_id)
{
    ha_Bsensor_entity_t *node_to_delete = ha_device->entity_binary_sensor->binary_sensor_list->next;

    while (node_to_delete != ha_device->entity_binary_sensor->binary_sensor_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_binary_sensor->binary_sensor_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_binary_sensor->binary_sensor_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }

        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);
        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic && isPvPortMallocAllocated(node_to_delete->availability_topic));

        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);
        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);
        if (node_to_delete->payload_off != NULL && isPvPortMallocAllocated(node_to_delete->payload_off))
            vPortFree(node_to_delete->payload_off);
        if (node_to_delete->payload_on != NULL && isPvPortMallocAllocated(node_to_delete->payload_on))
            vPortFree(node_to_delete->payload_on);
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}

#endif

#if CONFIG_ENTITY_ENABLE_TEXT
static void homeAssistant_create_text_data(ha_text_entity_t *text_entity, cJSON *device_json)
{
    if (text_entity == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }

    cJSON *root = cJSON_CreateObject();
    if (text_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", text_entity->name);
    if (text_entity->unique_id != NULL)
    {
        if (unique_id == NULL)
            unique_id = pvPortMalloc(strlen(text_entity->unique_id) + 4);
        memset(unique_id, 0, strlen(text_entity->unique_id) + 4);
        sprintf(unique_id, "%s-%02x%02x", text_entity->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    else
        HA_LOG_E("unique id is null for entity:%s \r\n", text_entity->name);
    cJSON_AddStringToObject(root, "unique_id", unique_id);
    if (text_entity->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", text_entity->object_id);
    if (text_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", text_entity->icon);
    if (text_entity->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", text_entity->availability_template);

    if (text_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", text_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (text_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", text_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (text_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", text_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);

    if (text_entity->json_attributes_template != NULL)
        cJSON_AddStringToObject(root, "json_attributes_template", text_entity->json_attributes_template);
    if (text_entity->json_attributes_topic != NULL)
        cJSON_AddStringToObject(root, "json_attributes_topic", text_entity->json_attributes_topic);
    if (text_entity->enabled_by_default)
        cJSON_AddTrueToObject(root, "enabled_by_default");
    if (text_entity->entity_category != NULL)
        cJSON_AddStringToObject(root, "entity_category", text_entity->entity_category);
    if (text_entity->encoding != NULL)
        cJSON_AddStringToObject(root, "encoding", text_entity->encoding);

    if (text_entity->max)
        cJSON_AddNumberToObject(root, "max", text_entity->max);
    if (text_entity->min)
        cJSON_AddNumberToObject(root, "min", text_entity->min);
    if (text_entity->mode != NULL)
        cJSON_AddStringToObject(root, "mode", text_entity->mode);
    else
        cJSON_AddStringToObject(root, "mode", "text");
    if (text_entity->command_template != NULL)
        cJSON_AddStringToObject(root, "command_template", text_entity->command_template);
    if (text_entity->command_topic == NULL)
    {
        text_entity->command_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        memset(text_entity->command_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        sprintf(text_entity->command_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/set", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], text_entity->unique_id);
    }
    cJSON_AddStringToObject(root, "command_topic", text_entity->command_topic);

    if (text_entity->state_topic == NULL)
    {
        text_entity->state_topic = pvPortMalloc(128);
        memset(text_entity->state_topic, 0, 128);
        sprintf(text_entity->state_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/state", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], text_entity->unique_id);
    }
    cJSON_AddStringToObject(root, "state_topic", text_entity->state_topic);

    if (text_entity->value_template != NULL)
        cJSON_AddStringToObject(root, "value_template", text_entity->value_template);
    if (text_entity->pattern != NULL)
        cJSON_AddStringToObject(root, "pattern", text_entity->pattern);

    if (text_entity->qos)
        cJSON_AddNumberToObject(root, "qos", text_entity->qos);
    if (text_entity->retain)
        cJSON_AddTrueToObject(root, "retain");

    // 添加设备信息
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    text_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}
/**
 * @brief 插入 text 实体节点
 *
 * @param text_new_node
 */
static void entity_text_add_node(ha_text_entity_t *text_new_node)
{

    ha_text_entity_t *text_list_handle = ha_device->entity_text->text_list->prev;
    // 开始创建新的实体数据
    homeAssistant_create_text_data(text_new_node, homeAssistant_device_create());
    if (text_new_node->entity_config_topic == NULL)
    {
        text_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_TEXT) + strlen(unique_id) + strlen("///config"));
        memset(text_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_TEXT) + strlen(unique_id) + strlen("///config"));
        sprintf(text_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_TEXT, text_new_node->unique_id);
    }
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(text_new_node->entity_config_topic, text_new_node->config_data, 0, 1);
        if (text_new_node->command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(text_new_node->command_topic, 1);
        if (text_new_node->availability_topic != NULL)
            homeAssistant_mqtt_port_public(text_new_node->availability_topic, text_new_node->payload_available, 0, 0);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }
    // 创建时检查默认内容是否存在
    if (text_new_node->text_value != NULL)
    {
        char *value_buff = pvPortMalloc(strlen(text_new_node->text_value));
        memset(value_buff, 0, strlen(text_new_node->text_value));
        sprintf(value_buff, "%s", text_new_node->text_value);

        memset(text_new_node->text_value, 0, strlen(text_new_node->text_value));
        text_new_node->text_value = pvPortMalloc(strlen(value_buff));
        memset(text_new_node->text_value, 0, strlen(value_buff));
        sprintf(text_new_node->text_value, "%s", value_buff);
        vPortFree(value_buff);
    }
    ha_text_entity_t *text_new_node_temp = pvPortMalloc(sizeof(ha_text_entity_t));
    memset(text_new_node_temp, 0, sizeof(ha_text_entity_t));
    memcpy(text_new_node_temp, text_new_node, sizeof(ha_text_entity_t));
    // 插入节点
    text_list_handle->next = text_new_node_temp;
    text_new_node_temp->prev = text_list_handle;
    text_new_node_temp->next = ha_device->entity_text->text_list;
    ha_device->entity_text->text_list->prev = text_new_node_temp;
    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(text_new_node->config_data);
}

static bool entity_text_delete_node(char *unique_id)
{
    ha_text_entity_t *node_to_delete = ha_device->entity_text->text_list->next;

    while (node_to_delete != ha_device->entity_text->text_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_text->text_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_text->text_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }

        if (node_to_delete->command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->command_topic);
            if (isPvPortMallocAllocated(node_to_delete->command_topic))
                vPortFree(node_to_delete->command_topic);
        }
        if (node_to_delete->command_template != NULL && isPvPortMallocAllocated(node_to_delete->command_template))
            vPortFree(node_to_delete->command_template);
        if (node_to_delete->value_template != NULL && isPvPortMallocAllocated(node_to_delete->value_template))
            vPortFree(node_to_delete->value_template);
        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);
        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);
        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available && isPvPortMallocAllocated(node_to_delete->payload_available));
        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}
#endif

#if CONFIG_ENTITY_ENABLE_NUMBER
/**
 * @brief 创建number实体的config 数据
 *
 * @param number_entity
 * @param device_json
 */
static void homeAssistant_create_number_data(ha_number_entity_t *number_entity, cJSON *device_json)
{
    if (number_entity == NULL)
    {
        HA_LOG_E("entity light buff is NULL\r\n");
        return;
    }

    cJSON *root = cJSON_CreateObject();
    if (number_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", number_entity->name);
    if (number_entity->unique_id != NULL)
    {
        if (unique_id == NULL)
            unique_id = pvPortMalloc(strlen(number_entity->unique_id) + 4);
        memset(unique_id, 0, strlen(number_entity->unique_id) + 4);
        sprintf(unique_id, "%s-%02x%02x", number_entity->unique_id, STA_MAC[4], STA_MAC[5]);
    }
    else
        HA_LOG_E("unique id is null for entity:%s \r\n", number_entity->name);
    cJSON_AddStringToObject(root, "unique_id", unique_id);
    if (number_entity->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", number_entity->object_id);
    if (number_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", number_entity->icon);
    if (number_entity->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", number_entity->availability_template);

    if (number_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", number_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (number_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", number_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (number_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", number_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);

    if (number_entity->json_attributes_template != NULL)
        cJSON_AddStringToObject(root, "json_attributes_template", number_entity->json_attributes_template);
    if (number_entity->json_attributes_topic != NULL)
        cJSON_AddStringToObject(root, "json_attributes_topic", number_entity->json_attributes_topic);
    if (number_entity->enabled_by_default)
        cJSON_AddTrueToObject(root, "enabled_by_default");
    if (number_entity->entity_category != NULL)
        cJSON_AddStringToObject(root, "entity_category", number_entity->entity_category);
    if (number_entity->encoding != NULL)
        cJSON_AddStringToObject(root, "encoding", number_entity->encoding);

    if (number_entity->unit_of_measurement != NULL)
        cJSON_AddStringToObject(root, "unit_of_measurement", number_entity->unit_of_measurement);
    if (number_entity->optimistic)
        cJSON_AddTrueToObject(root, "optimistic");

    if (number_entity->step)
        cJSON_AddNumberToObject(root, "step", 1.0 / (10.0 * number_entity->step));
    if (number_entity->max)
        cJSON_AddNumberToObject(root, "max", number_entity->max);
    if (number_entity->min)
        cJSON_AddNumberToObject(root, "min", number_entity->min);
    if (number_entity->mode != NULL)
        cJSON_AddStringToObject(root, "mode", number_entity->mode);
    else
        cJSON_AddStringToObject(root, "mode", "text");
    if (number_entity->command_template != NULL)
        cJSON_AddStringToObject(root, "command_template", number_entity->command_template);
    if (number_entity->command_topic == NULL)
    {
        number_entity->command_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        memset(number_entity->command_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        sprintf(number_entity->command_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/set", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    cJSON_AddStringToObject(root, "command_topic", number_entity->command_topic);

    if (number_entity->state_topic == NULL)
    {
        number_entity->state_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///state"));
        memset(number_entity->state_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///state"));
        sprintf(number_entity->state_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/state", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    cJSON_AddStringToObject(root, "state_topic", number_entity->state_topic);

    if (number_entity->value_template != NULL)
        cJSON_AddStringToObject(root, "value_template", number_entity->value_template);
    if (number_entity->pattern != NULL)
        cJSON_AddStringToObject(root, "pattern", number_entity->pattern);

    if (number_entity->qos)
        cJSON_AddNumberToObject(root, "qos", number_entity->qos);
    if (number_entity->retain)
        cJSON_AddTrueToObject(root, "retain");

    // 添加设备信息
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    number_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void entity_number_add_node(ha_number_entity_t *number_new_node)
{

    ha_number_entity_t *number_list_handle = ha_device->entity_number->number_list->prev;
    // 开始创建新的实体数据
    homeAssistant_create_number_data(number_new_node, homeAssistant_device_create());
    if (number_new_node->entity_config_topic == NULL)
    {
        number_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_NUMBER) + strlen(unique_id) + strlen("///config"));
        memset(number_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_NUMBER) + strlen(unique_id) + strlen("///config"));
        sprintf(number_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_NUMBER, unique_id);
    }
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (number_new_node->entity_config_topic != NULL && number_new_node->config_data != NULL)
            homeAssistant_mqtt_port_public(number_new_node->entity_config_topic, number_new_node->config_data, 0, 1);
        if (number_new_node->command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(number_new_node->command_topic, 1);
        if (number_new_node->availability_topic != NULL && number_new_node->payload_available != NULL)
            homeAssistant_mqtt_port_public(number_new_node->availability_topic, number_new_node->payload_available, 0, 0);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }
    // 创建时检查默认内容是否存在
    if (number_new_node->number_value != NULL)
    {
        // 缓存内容
        char *value_buff = pvPortMalloc(strlen(number_new_node->number_value));
        memset(value_buff, 0, strlen(number_new_node->number_value));
        sprintf(value_buff, "%s", number_new_node->number_value);
        memset(number_new_node->number_value, 0, strlen(number_new_node->number_value));
        number_new_node->number_value = NULL;
        // 重新申请空间
        number_new_node->number_value = pvPortMalloc(strlen(value_buff));
        memset(number_new_node->number_value, 0, strlen(value_buff));
        sprintf(number_new_node->number_value, "%s", value_buff);
        vPortFree(value_buff);
    }
    ha_number_entity_t *number_new_node_temp = pvPortMalloc(sizeof(ha_number_entity_t));
    memset(number_new_node_temp, 0, sizeof(ha_number_entity_t));
    memcpy(number_new_node_temp, number_new_node, sizeof(ha_number_entity_t));

    // 插入节点
    number_list_handle->next = number_new_node_temp;
    number_new_node_temp->prev = number_list_handle;
    number_new_node_temp->next = ha_device->entity_number->number_list;
    ha_device->entity_number->number_list->prev = number_new_node_temp;
    vPortFree(number_new_node->config_data);
    vPortFree(unique_id);
    unique_id = NULL;
}

static bool entity_number_delete_node(char *unique_id)
{
    ha_number_entity_t *node_to_delete = ha_device->entity_number->number_list->next;

    while (node_to_delete != ha_device->entity_number->number_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_number->number_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_number->number_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }

        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);
        if (node_to_delete->command_topic != NULL && isPvPortMallocAllocated(node_to_delete->command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->command_topic);
            if (isPvPortMallocAllocated(node_to_delete->command_topic))
                vPortFree(node_to_delete->command_topic);
        }
        if (node_to_delete->command_template != NULL && isPvPortMallocAllocated(node_to_delete->command_template))
            vPortFree(node_to_delete->command_template);

        if (node_to_delete->value_template != NULL && isPvPortMallocAllocated(node_to_delete->value_template))
            vPortFree(node_to_delete->value_template);

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);
        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);
        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);
        ;
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}

#endif

/**
 * @brief 空调设备
 *
 */
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC

static char *modes_def[] = {"auto", "cool", "heat", "dry", "fan_only"};
static char *fan_modes_def[] = {"auto", "low", "medium", "high"};
static char *preset_modes_def[] = {"eco", "away", "boost", "comfort", "home", "sleep", "activity"};

static void homeAssistant_create_climate_HVAC_data(ha_climateHVAC_t *climateHVAC_entity, cJSON *device_json)
{
    if (climateHVAC_entity == NULL)
    {
        HA_LOG_E("entity climateHVAC buff is NULL\r\n");
        return;
    }
    cJSON *root = cJSON_CreateObject();

    if (climateHVAC_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", climateHVAC_entity->name);

    cJSON_AddStringToObject(root, "unique_id", unique_id);
    if (climateHVAC_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", climateHVAC_entity->icon);
    if (climateHVAC_entity->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", climateHVAC_entity->availability_template);

    if (climateHVAC_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", climateHVAC_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (climateHVAC_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", climateHVAC_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (climateHVAC_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", climateHVAC_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);

    if (climateHVAC_entity->payload_off != NULL)
        cJSON_AddStringToObject(root, "payload_off", climateHVAC_entity->payload_off);
    if (climateHVAC_entity->payload_on != NULL)
        cJSON_AddStringToObject(root, "payload_on", climateHVAC_entity->payload_on);
    // 定义监听湿度的TOPIC
    if (climateHVAC_entity->current_humidity_template != NULL)
        cJSON_AddStringToObject(root, "current_humidity_template", climateHVAC_entity->current_humidity_template);
    if (climateHVAC_entity->current_humidity_topic != NULL)
        cJSON_AddStringToObject(root, "current_humidity_topic", climateHVAC_entity->current_humidity_topic);
    // 定义监听温度的topic
    if (climateHVAC_entity->current_temperature_template != NULL)
        cJSON_AddStringToObject(root, "current_temperature_template", climateHVAC_entity->current_temperature_template);
    if (climateHVAC_entity->current_temperature_topic != NULL)
        cJSON_AddStringToObject(root, "current_temperature_topic", climateHVAC_entity->current_temperature_topic);
    //
    if (!climateHVAC_entity->enabled_by_default)
        cJSON_AddFalseToObject(root, "enabled_by_default");
    if (climateHVAC_entity->encoding != NULL)
        cJSON_AddStringToObject(root, "encoding", climateHVAC_entity->encoding);
    if (climateHVAC_entity->entity_category != NULL)
        cJSON_AddStringToObject(root, "entity_category", climateHVAC_entity->entity_category);

    // 风力模式控制
    if (climateHVAC_entity->fan_mode_command_template != NULL)
        cJSON_AddStringToObject(root, "fan_mode_command_template", climateHVAC_entity->fan_mode_command_template);
    if (climateHVAC_entity->fan_mode_command_topic != NULL)
        cJSON_AddStringToObject(root, "fan_mode_command_topic", climateHVAC_entity->fan_mode_command_topic);
    if (climateHVAC_entity->fan_mode_state_template != NULL)
        cJSON_AddStringToObject(root, "fan_mode_state_template", climateHVAC_entity->fan_mode_state_template);
    if (climateHVAC_entity->fan_modes[0] != NULL)
    {
        cJSON *fan_modes = cJSON_CreateArray();
        if (climateHVAC_entity->fan_modes[0] != NULL)
            cJSON_AddItemToArray(fan_modes, cJSON_CreateString(climateHVAC_entity->fan_modes[0]));
        if (climateHVAC_entity->fan_modes[1] != NULL)
            cJSON_AddItemToArray(fan_modes, cJSON_CreateString(climateHVAC_entity->fan_modes[1]));
        if (climateHVAC_entity->fan_modes[2] != NULL)
            cJSON_AddItemToArray(fan_modes, cJSON_CreateString(climateHVAC_entity->fan_modes[2]));
        if (climateHVAC_entity->fan_modes[3] != NULL)
            cJSON_AddItemToArray(fan_modes, cJSON_CreateString(climateHVAC_entity->fan_modes[3]));
        cJSON_AddItemToObject(root, "fan_modes", fan_modes);
    }
    if (climateHVAC_entity->initial != 0.00)
        cJSON_AddNumberToObject(root, "initial", climateHVAC_entity->initial);
    if (climateHVAC_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", climateHVAC_entity->icon);
    if (climateHVAC_entity->json_attributes_template != NULL)
        cJSON_AddStringToObject(root, "json_attributes_template", climateHVAC_entity->json_attributes_template);
    if (climateHVAC_entity->json_attributes_topic != NULL)
        cJSON_AddStringToObject(root, "json_attributes_topic", climateHVAC_entity->json_attributes_topic);

    if (climateHVAC_entity->max_humidity != 0.00)
        cJSON_AddNumberToObject(root, "max_humidity", climateHVAC_entity->max_humidity);
    if (climateHVAC_entity->max_temp != 0.00)
        cJSON_AddNumberToObject(root, "max_temp", climateHVAC_entity->max_temp);
    if (climateHVAC_entity->min_humidity != 0.0)
        cJSON_AddNumberToObject(root, "min_humidity", climateHVAC_entity->min_humidity);
    if (climateHVAC_entity->min_temp != 0.0)
        cJSON_AddNumberToObject(root, "min_temp", climateHVAC_entity->min_temp);
    // 定义模式
    if (climateHVAC_entity->mode_command_template != NULL)
        cJSON_AddStringToObject(root, "mode_command_template", climateHVAC_entity->mode_command_template);
    if (climateHVAC_entity->mode_command_topic != NULL)
        cJSON_AddStringToObject(root, "mode_command_topic", climateHVAC_entity->mode_command_topic);
    if (climateHVAC_entity->mode_state_template != NULL)
        cJSON_AddStringToObject(root, "mode_state_template", climateHVAC_entity->mode_state_template);
    if (climateHVAC_entity->mode_state_topic != NULL)
        cJSON_AddStringToObject(root, "mode_state_topic", climateHVAC_entity->mode_state_topic);
    if (climateHVAC_entity->modes[0] != NULL)
    {
        cJSON *modes = cJSON_CreateArray();
        if (climateHVAC_entity->modes[0] != NULL)
            cJSON_AddItemToArray(modes, cJSON_CreateString(climateHVAC_entity->modes[0]));
        if (climateHVAC_entity->modes[1] != NULL)
            cJSON_AddItemToArray(modes, cJSON_CreateString(climateHVAC_entity->modes[1]));
        if (climateHVAC_entity->modes[2] != NULL)
            cJSON_AddItemToArray(modes, cJSON_CreateString(climateHVAC_entity->modes[2]));
        if (climateHVAC_entity->modes[3] != NULL)
            cJSON_AddItemToArray(modes, cJSON_CreateString(climateHVAC_entity->modes[3]));
        if (climateHVAC_entity->modes[4] != NULL)
            cJSON_AddItemToArray(modes, cJSON_CreateString(climateHVAC_entity->modes[4]));
        if (climateHVAC_entity->modes[5] != NULL)
            cJSON_AddItemToArray(modes, cJSON_CreateString(climateHVAC_entity->modes[5]));
        cJSON_AddItemToObject(root, "modes", modes);
    }
    // 定义开关
    if (climateHVAC_entity->power_command_template != NULL)
        cJSON_AddStringToObject(root, "power_command_template", climateHVAC_entity->power_command_template);
    if (climateHVAC_entity->power_command_topic != NULL)
        cJSON_AddStringToObject(root, "power_command_topic", climateHVAC_entity->power_command_topic);
    // 控制精度
    if (climateHVAC_entity->precision != 0.0)
        cJSON_AddNumberToObject(root, "precision", climateHVAC_entity->precision);
    // 预设模式配置
    if (climateHVAC_entity->preset_mode_command_template != NULL)
        cJSON_AddStringToObject(root, "preset_mode_command_template", climateHVAC_entity->preset_mode_command_template);
    if (climateHVAC_entity->preset_mode_command_topic != NULL)
        cJSON_AddStringToObject(root, "preset_mode_command_topic", climateHVAC_entity->preset_mode_command_topic);
    if (climateHVAC_entity->preset_mode_state_topic != NULL)
        cJSON_AddStringToObject(root, "preset_mode_state_topic", climateHVAC_entity->preset_mode_state_topic);
    if (climateHVAC_entity->preset_mode_value_template != NULL)
        cJSON_AddStringToObject(root, "preset_mode_value_template", climateHVAC_entity->preset_mode_value_template);
    if (climateHVAC_entity->preset_modes[0] != NULL)
    {
        cJSON *preset_modes = cJSON_CreateArray();
        if (climateHVAC_entity->preset_modes[0] != NULL)
            cJSON_AddItemToArray(preset_modes, cJSON_CreateString(climateHVAC_entity->preset_modes[0]));
        if (climateHVAC_entity->preset_modes[1] != NULL)
            cJSON_AddItemToArray(preset_modes, cJSON_CreateString(climateHVAC_entity->preset_modes[1]));
        if (climateHVAC_entity->preset_modes[2] != NULL)
            cJSON_AddItemToArray(preset_modes, cJSON_CreateString(climateHVAC_entity->preset_modes[2]));
        if (climateHVAC_entity->preset_modes[3] != NULL)
            cJSON_AddItemToArray(preset_modes, cJSON_CreateString(climateHVAC_entity->preset_modes[3]));
        if (climateHVAC_entity->preset_modes[4] != NULL)
            cJSON_AddItemToArray(preset_modes, cJSON_CreateString(climateHVAC_entity->preset_modes[4]));
        if (climateHVAC_entity->preset_modes[5] != NULL)
            cJSON_AddItemToArray(preset_modes, cJSON_CreateString(climateHVAC_entity->preset_modes[5]));
        if (climateHVAC_entity->preset_modes[6] != NULL)
            cJSON_AddItemToArray(preset_modes, cJSON_CreateString(climateHVAC_entity->preset_modes[6]));
        cJSON_AddItemToObject(root, "preset_modes", preset_modes);
    }
    if (climateHVAC_entity->qos != 0)
        cJSON_AddNumberToObject(root, "qos", climateHVAC_entity->qos);
    if (climateHVAC_entity->retain)
        cJSON_AddTrueToObject(root, "retain");
    // 摆风模式
    if (climateHVAC_entity->swing_mode_command_template != NULL)
        cJSON_AddStringToObject(root, "swing_mode_command_template", climateHVAC_entity->swing_mode_command_template);
    if (climateHVAC_entity->swing_mode_command_topic != NULL)
        cJSON_AddStringToObject(root, "swing_mode_command_topic", climateHVAC_entity->swing_mode_command_topic);
    if (climateHVAC_entity->swing_mode_state_template != NULL)
        cJSON_AddStringToObject(root, "swing_mode_state_template", climateHVAC_entity->swing_mode_state_template);
    if (climateHVAC_entity->swing_mode_state_topic != NULL)
        cJSON_AddStringToObject(root, "swing_mode_state_topic", climateHVAC_entity->swing_mode_state_topic);
    if (climateHVAC_entity->swing_modes[0] != NULL)
    {
        cJSON *swing_modes = cJSON_CreateArray();
        if (climateHVAC_entity->swing_modes[0] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[0]));
        if (climateHVAC_entity->swing_modes[1] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[1]));
        if (climateHVAC_entity->swing_modes[2] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[2]));
        if (climateHVAC_entity->swing_modes[3] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[3]));
        if (climateHVAC_entity->swing_modes[4] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[4]));
        if (climateHVAC_entity->swing_modes[5] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[5]));
        if (climateHVAC_entity->swing_modes[6] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[6]));
        if (climateHVAC_entity->swing_modes[7] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[7]));
        if (climateHVAC_entity->swing_modes[8] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[8]));
        if (climateHVAC_entity->swing_modes[9] != NULL)
            cJSON_AddItemToArray(swing_modes, cJSON_CreateString(climateHVAC_entity->swing_modes[9]));
        cJSON_AddItemToObject(root, "swing_modes", swing_modes);
    }
    // 目标湿度
    if (climateHVAC_entity->target_humidity_command_template != NULL)
        cJSON_AddStringToObject(root, "target_humidity_command_template", climateHVAC_entity->target_humidity_command_template);
    if (climateHVAC_entity->target_humidity_command_topic != NULL)
        cJSON_AddStringToObject(root, "target_humidity_command_topic", climateHVAC_entity->target_humidity_command_topic);

    if (climateHVAC_entity->target_humidity_state_template != NULL)
        cJSON_AddStringToObject(root, "target_humidity_state_template", climateHVAC_entity->target_humidity_state_template);
    if (climateHVAC_entity->target_humidity_state_topic != NULL)
        cJSON_AddStringToObject(root, "target_humidity_state_topic", climateHVAC_entity->target_humidity_state_topic);
    // 目标温度配置
    if (climateHVAC_entity->temperature_command_template != NULL)
        cJSON_AddStringToObject(root, "temperature_command_template", climateHVAC_entity->temperature_command_template);
    if (climateHVAC_entity->temperature_command_topic != NULL)
        cJSON_AddStringToObject(root, "temperature_command_topic", climateHVAC_entity->temperature_command_topic);

    if (climateHVAC_entity->temperature_high_command_template != NULL)
        cJSON_AddStringToObject(root, "temperature_high_command_template", climateHVAC_entity->temperature_high_command_template);
    if (climateHVAC_entity->temperature_high_command_topic != NULL)
        cJSON_AddStringToObject(root, "temperature_high_command_topic", climateHVAC_entity->temperature_high_command_topic);

    if (climateHVAC_entity->temperature_high_state_template != NULL)
        cJSON_AddStringToObject(root, "temperature_high_state_template", climateHVAC_entity->temperature_high_state_template);
    if (climateHVAC_entity->temperature_high_state_topic != NULL)
        cJSON_AddStringToObject(root, "temperature_high_state_topic", climateHVAC_entity->temperature_high_state_topic);

    if (climateHVAC_entity->temperature_low_command_template != NULL)
        cJSON_AddStringToObject(root, "temperature_low_command_template", climateHVAC_entity->temperature_low_command_template);
    if (climateHVAC_entity->temperature_low_command_topic != NULL)
        cJSON_AddStringToObject(root, "temperature_low_command_topic", climateHVAC_entity->temperature_low_command_topic);

    if (climateHVAC_entity->temperature_low_state_template != NULL)
        cJSON_AddStringToObject(root, "temperature_low_state_template", climateHVAC_entity->temperature_low_state_template);
    if (climateHVAC_entity->temperature_low_state_topic != NULL)
        cJSON_AddStringToObject(root, "temperature_low_state_topic", climateHVAC_entity->temperature_low_state_topic);

    if (climateHVAC_entity->temperature_state_template != NULL)
        cJSON_AddStringToObject(root, "temperature_state_template", climateHVAC_entity->temperature_state_template);
    if (climateHVAC_entity->temperature_state_topic != NULL)
        cJSON_AddStringToObject(root, "temperature_state_topic", climateHVAC_entity->temperature_state_topic);

    if (climateHVAC_entity->temperature_unit != NULL)
        cJSON_AddStringToObject(root, "temperature_unit", climateHVAC_entity->temperature_unit);
    if (climateHVAC_entity->temp_step != 0.0)
        cJSON_AddNumberToObject(root, "temp_step", climateHVAC_entity->temp_step);

    if (climateHVAC_entity->value_template != NULL)
        cJSON_AddStringToObject(root, "value_template", climateHVAC_entity->value_template);
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    climateHVAC_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void entity_climate_HVAC_add_node(ha_climateHVAC_t *climateHVAC_new_node)
{
    ha_climateHVAC_t *climateHVAC_list_handle = ha_device->entity_climateHVAC->climateHVAC_list->prev;
    // 初始化相关的参数
    climateHVAC_new_node->enabled_by_default = true;
    climateHVAC_new_node->modes_type = AC_MODES_AUTO;
    if (unique_id == NULL && climateHVAC_new_node->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(climateHVAC_new_node->unique_id) + 5);
        memset(unique_id, 0, strlen(climateHVAC_new_node->unique_id) + 5);
        sprintf(unique_id, "%s-%02x%02x", climateHVAC_new_node->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    // if (climateHVAC_new_node->action_topic==NULL) {
    //     climateHVAC_new_node->action_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->action_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->action_topic, "%s/%s/%s/action/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }
    // 开关
    if (climateHVAC_new_node->power_command_topic == NULL)
    {
        climateHVAC_new_node->power_command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///power/set"));
        memset(climateHVAC_new_node->power_command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///power/set"));
        sprintf(climateHVAC_new_node->power_command_topic, "%s/%s/%s/power/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
        if (climateHVAC_new_node->payload_on == NULL)
            climateHVAC_new_node->payload_on = "ON";
        if (climateHVAC_new_node->payload_off == NULL)
            climateHVAC_new_node->payload_off = "OFF";
    }
    // 模式
    if (climateHVAC_new_node->mode_command_topic == NULL)
    {
        climateHVAC_new_node->mode_command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///modes/set"));
        memset(climateHVAC_new_node->mode_command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///modes/set"));
        sprintf(climateHVAC_new_node->mode_command_topic, "%s/%s/%s/modes/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    }
    if (climateHVAC_new_node->mode_command_template == NULL)
    {
        climateHVAC_new_node->mode_command_template = "{\"mode\":\"{{value}}\"}";
    }

    if (climateHVAC_new_node->mode_state_topic == NULL)
    {
        climateHVAC_new_node->mode_state_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///modes/state"));
        memset(climateHVAC_new_node->mode_state_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///modes/state"));
        sprintf(climateHVAC_new_node->mode_state_topic, "%s/%s/%s/modes/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    }
    // 温度
    //  if (climateHVAC_new_node->current_temperature_topic==NULL) {
    //      climateHVAC_new_node->current_temperature_topic = pvPortMalloc(256);
    //      memset(climateHVAC_new_node->current_temperature_topic, 0, 256);
    //      sprintf(climateHVAC_new_node->current_temperature_topic, "%s/%s/%s/current_temperature/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    //  }
    if (climateHVAC_new_node->temperature_command_template == NULL)
        climateHVAC_new_node->temperature_command_template = "{\"temperature\":{{value}}}";

    if (climateHVAC_new_node->temperature_command_topic == NULL)
    {
        climateHVAC_new_node->temperature_command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///temperature/set"));
        memset(climateHVAC_new_node->temperature_command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///temperature/set"));
        sprintf(climateHVAC_new_node->temperature_command_topic, "%s/%s/%s/temperature/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    }
    // if (climateHVAC_new_node->temperature_state_topic==NULL) {
    //     climateHVAC_new_node->temperature_state_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->temperature_state_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->temperature_state_topic, "%s/%s/%s/temperature/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }
    // if (climateHVAC_new_node->max_temp==0.0) climateHVAC_new_node->max_temp = 30.0;
    // if (climateHVAC_new_node->min_temp==0.0) climateHVAC_new_node->min_temp = 17.0;

    // if (climateHVAC_new_node->temperature_high_command_template==NULL)climateHVAC_new_node->temperature_high_command_template = "{\"high_temperature\":{{value}}}";
    // if (climateHVAC_new_node->temperature_high_command_topic==NULL) {
    //     climateHVAC_new_node->temperature_high_command_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->temperature_high_command_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->temperature_high_command_topic, "%s/%s/%s/high_temperature/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }
    // if (climateHVAC_new_node->temperature_high_state_topic==NULL) {
    //     climateHVAC_new_node->temperature_high_state_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->temperature_high_state_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->temperature_high_state_topic, "%s/%s/%s/high_temperature/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }

    if (climateHVAC_new_node->temperature_low_command_template == NULL)
        climateHVAC_new_node->temperature_low_command_template = "{\"low_temperature\":{{value}}}";
    if (climateHVAC_new_node->temperature_low_command_topic == NULL)
    {
        climateHVAC_new_node->temperature_low_command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///low_temperature/set"));
        memset(climateHVAC_new_node->temperature_low_command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///low_temperature/set"));
        sprintf(climateHVAC_new_node->temperature_low_command_topic, "%s/%s/%s/low_temperature/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    }
    // if (climateHVAC_new_node->temperature_low_state_topic==NULL) {
    //     climateHVAC_new_node->temperature_low_state_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->temperature_low_state_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->temperature_low_state_topic, "%s/%s/%s/low_temperature/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }

    // 预设值
    // if (climateHVAC_new_node->preset_mode_command_topic==NULL) {
    //     climateHVAC_new_node->preset_mode_command_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->preset_mode_command_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->preset_mode_command_topic, "%s/%s/%s/preset_mode/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }
    // if (climateHVAC_new_node->preset_mode_state_topic==NULL) {
    //     climateHVAC_new_node->preset_mode_state_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->preset_mode_state_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->preset_mode_state_topic, "%s/%s/%s/preset_mode/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }
    // //风扇模式
    if (climateHVAC_new_node->fan_mode_command_topic == NULL)
    {
        climateHVAC_new_node->fan_mode_command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///fan_mode/set"));
        memset(climateHVAC_new_node->fan_mode_command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///fan_mode/set"));
        sprintf(climateHVAC_new_node->fan_mode_command_topic, "%s/%s/%s/fan_mode/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    }
    // if (climateHVAC_new_node->fan_mode_state_topic==NULL) {
    //     climateHVAC_new_node->fan_mode_state_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->fan_mode_state_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->fan_mode_state_topic, "%s/%s/%s/fan_mode/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }
    // 摆风模式
    if (climateHVAC_new_node->swing_mode_command_topic == NULL)
    {
        climateHVAC_new_node->swing_mode_command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///swing_mode/set"));
        memset(climateHVAC_new_node->swing_mode_command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///swing_mode/set"));
        sprintf(climateHVAC_new_node->swing_mode_command_topic, "%s/%s/%s/swing_mode/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    }
    // if (climateHVAC_new_node->swing_mode_state_topic==NULL) {
    //     climateHVAC_new_node->swing_mode_state_topic = pvPortMalloc(256);
    //     memset(climateHVAC_new_node->swing_mode_state_topic, 0, 256);
    //     sprintf(climateHVAC_new_node->swing_mode_state_topic, "%s/%s/%s/swing_mode/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
    // }

    homeAssistant_create_climate_HVAC_data(climateHVAC_new_node, homeAssistant_device_create());

    if (climateHVAC_new_node->entity_config_topic == NULL)
    {
        climateHVAC_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///config"));
        memset(climateHVAC_new_node->entity_config_topic, 0, (strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_CLIMATE_HVAC) + strlen(unique_id) + strlen("///config")));

        sprintf(climateHVAC_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_CLIMATE_HVAC, unique_id);
        // HA_LOG_I("config topic=%s\r\n", climateHVAC_new_node->entity_config_topic);
        // HA_LOG_D("config data=%s\r\n", climateHVAC_new_node->config_data);
    }
    if (ha_device->mqtt_info.mqtt_connect_status)
    {

        if (climateHVAC_new_node->entity_config_topic != NULL && climateHVAC_new_node->config_data != NULL)
            homeAssistant_mqtt_port_public(climateHVAC_new_node->entity_config_topic, climateHVAC_new_node->config_data, 0, 1);

        if (climateHVAC_new_node->availability_topic != NULL && climateHVAC_new_node->payload_available != NULL)
            homeAssistant_mqtt_port_public(climateHVAC_new_node->availability_topic, climateHVAC_new_node->payload_available, 0, 0);

        if (climateHVAC_new_node->power_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->power_command_topic, 1);

        if (climateHVAC_new_node->mode_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->mode_command_topic, 1);
        if (climateHVAC_new_node->temperature_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->temperature_command_topic, 1);
        if (climateHVAC_new_node->temperature_low_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->temperature_low_command_topic, 1);
        if (climateHVAC_new_node->temperature_high_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->temperature_high_command_topic, 1);
        if (climateHVAC_new_node->current_temperature_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->current_temperature_topic, 1);
        if (climateHVAC_new_node->preset_mode_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->preset_mode_command_topic, 1);
        if (climateHVAC_new_node->fan_mode_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->fan_mode_command_topic, 1);
        if (climateHVAC_new_node->swing_mode_command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->swing_mode_command_topic, 1);
        if (climateHVAC_new_node->action_topic != NULL)
            homeAssistant_mqtt_port_subscribe(climateHVAC_new_node->action_topic, 1);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }

    ha_climateHVAC_t *climateHVAC_new_node_temp = pvPortMalloc(sizeof(ha_climateHVAC_t));
    memset(climateHVAC_new_node_temp, 0, sizeof(ha_climateHVAC_t));
    memcpy(climateHVAC_new_node_temp, climateHVAC_new_node, sizeof(ha_climateHVAC_t));

    climateHVAC_list_handle->next = climateHVAC_new_node_temp;
    climateHVAC_new_node_temp->prev = climateHVAC_new_node_temp;
    climateHVAC_new_node_temp->next = ha_device->entity_climateHVAC->climateHVAC_list;
    ha_device->entity_climateHVAC->climateHVAC_list->prev = climateHVAC_new_node_temp;

    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(climateHVAC_new_node->config_data);
}

static bool entity_climate_HVAC_delete_node(char *unique_id)
{
    ha_climateHVAC_t *node_to_delete = ha_device->entity_climateHVAC->climateHVAC_list->next;

    while (node_to_delete != ha_device->entity_climateHVAC->climateHVAC_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_climateHVAC->climateHVAC_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_climateHVAC->climateHVAC_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
            node_to_delete->entity_config_topic = NULL;
        }

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);
        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);
        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);
        if (node_to_delete->availability_template != NULL && isPvPortMallocAllocated(node_to_delete->availability_template))
            vPortFree(node_to_delete->availability_template);
        if (node_to_delete->availability_mode != NULL && isPvPortMallocAllocated(node_to_delete->availability_mode))
            vPortFree(node_to_delete->availability_mode);
        /**
         * @brief 删除模式配置信息
         *
         */
        if (node_to_delete->mode_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->mode_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->mode_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->mode_command_topic))
                vPortFree(node_to_delete->mode_command_topic);
        }
        if (node_to_delete->mode_command_template != NULL && isPvPortMallocAllocated(node_to_delete->mode_command_template))
            vPortFree(node_to_delete->mode_command_template);
        if (node_to_delete->mode_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->mode_state_topic))
            vPortFree(node_to_delete->mode_state_topic);
        if (node_to_delete->mode_state_template != NULL && isPvPortMallocAllocated(node_to_delete->mode_state_template))
            vPortFree(node_to_delete->mode_state_template);
        if (node_to_delete->modes[0] != NULL && isPvPortMallocAllocated(node_to_delete->modes[0]))
            vPortFree(node_to_delete->modes[0]);
        if (node_to_delete->modes[1] != NULL && isPvPortMallocAllocated(node_to_delete->modes[1]))
            vPortFree(node_to_delete->modes[1]);
        if (node_to_delete->modes[2] != NULL && isPvPortMallocAllocated(node_to_delete->modes[2]))
            vPortFree(node_to_delete->modes[2]);
        if (node_to_delete->modes[3] != NULL && isPvPortMallocAllocated(node_to_delete->modes[3]))
            vPortFree(node_to_delete->modes[3]);
        if (node_to_delete->modes[4] != NULL && isPvPortMallocAllocated(node_to_delete->modes[4]))
            vPortFree(node_to_delete->modes[4]);
        if (node_to_delete->modes[5] != NULL && isPvPortMallocAllocated(node_to_delete->modes[5]))
            vPortFree(node_to_delete->modes[5]);
        /**
         * @brief 删除温湿度配置信息
         *
         */
        if (node_to_delete->current_humidity_topic != NULL && isPvPortMallocAllocated(node_to_delete->current_humidity_topic))
            vPortFree(node_to_delete->current_humidity_topic);
        if (node_to_delete->current_humidity_template != NULL && isPvPortMallocAllocated(node_to_delete->current_humidity_template))
            vPortFree(node_to_delete->current_humidity_template);
        if (node_to_delete->current_temperature_topic != NULL && isPvPortMallocAllocated(node_to_delete->current_temperature_topic))
            vPortFree(node_to_delete->current_temperature_topic);
        if (node_to_delete->current_temperature_template != NULL && isPvPortMallocAllocated(node_to_delete->current_temperature_template))
            vPortFree(node_to_delete->current_temperature_template);
        /**
         * @brief 删除扫风配置信息
         *
         */
        if (node_to_delete->fan_mode_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->fan_mode_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->fan_mode_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->fan_mode_command_topic))
                vPortFree(node_to_delete->fan_mode_command_topic);
        }
        if (node_to_delete->fan_mode_command_template != NULL && isPvPortMallocAllocated(node_to_delete->fan_mode_command_template))
            vPortFree(node_to_delete->fan_mode_command_template);
        if (node_to_delete->fan_mode_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->fan_mode_state_topic))
            vPortFree(node_to_delete->fan_mode_state_topic);
        if (node_to_delete->fan_mode_state_template != NULL && isPvPortMallocAllocated(node_to_delete->fan_mode_state_template))
            vPortFree(node_to_delete->fan_mode_state_template);
        if (node_to_delete->fan_modes[0] != NULL && isPvPortMallocAllocated(node_to_delete->fan_modes[0]))
            vPortFree(node_to_delete->fan_modes[0]);
        if (node_to_delete->fan_modes[1] != NULL && isPvPortMallocAllocated(node_to_delete->fan_modes[1]))
            vPortFree(node_to_delete->fan_modes[1]);
        if (node_to_delete->fan_modes[2] != NULL && isPvPortMallocAllocated(node_to_delete->fan_modes[2]))
            vPortFree(node_to_delete->fan_modes[2]);
        if (node_to_delete->fan_modes[3] != NULL && isPvPortMallocAllocated(node_to_delete->fan_modes[3]))
            vPortFree(node_to_delete->fan_modes[3]);
        /**
         * @brief 删除开关配置信息
         *
         */
        if (node_to_delete->power_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->power_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->power_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->power_command_topic))
                vPortFree(node_to_delete->power_command_topic);
        }
        if (node_to_delete->power_command_template != NULL && isPvPortMallocAllocated(node_to_delete->power_command_template))
            vPortFree(node_to_delete->power_command_template);
        /**
         * @brief 删除预置信息配置
         *
         */
        if (node_to_delete->preset_mode_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->preset_mode_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->preset_mode_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->preset_mode_command_topic))
                vPortFree(node_to_delete->preset_mode_command_topic);
        }
        if (node_to_delete->preset_mode_command_template != NULL && isPvPortMallocAllocated(node_to_delete->preset_mode_command_template))
            vPortFree(node_to_delete->preset_mode_command_template);
        if (node_to_delete->preset_mode_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->preset_mode_state_topic))
            vPortFree(node_to_delete->preset_mode_state_topic);
        if (node_to_delete->preset_mode_value_template != NULL && isPvPortMallocAllocated(node_to_delete->preset_mode_value_template))
            vPortFree(node_to_delete->preset_mode_value_template);
        /**
         * @brief 删除摆风信息
         *
         */
        if (node_to_delete->swing_mode_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->swing_mode_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->swing_mode_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->swing_mode_command_topic))
                vPortFree(node_to_delete->swing_mode_command_topic);
        }
        if (node_to_delete->swing_mode_command_template != NULL && isPvPortMallocAllocated(node_to_delete->swing_mode_command_template))
            vPortFree(node_to_delete->swing_mode_command_template);
        if (node_to_delete->swing_mode_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->swing_mode_state_topic))
            vPortFree(node_to_delete->swing_mode_state_topic);
        if (node_to_delete->swing_mode_state_template != NULL && isPvPortMallocAllocated(node_to_delete->swing_mode_state_template))
            vPortFree(node_to_delete->swing_mode_state_template);
        if (node_to_delete->swing_modes[0] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[0]))
            vPortFree(node_to_delete->swing_modes[0]);
        if (node_to_delete->swing_modes[1] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[1]))
            vPortFree(node_to_delete->swing_modes[1]);
        if (node_to_delete->swing_modes[2] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[2]))
            vPortFree(node_to_delete->swing_modes[2]);
        if (node_to_delete->swing_modes[3] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[3]))
            vPortFree(node_to_delete->swing_modes[3]);
        if (node_to_delete->swing_modes[4] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[4]))
            vPortFree(node_to_delete->swing_modes[4]);
        if (node_to_delete->swing_modes[5] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[5]))
            vPortFree(node_to_delete->swing_modes[5]);
        if (node_to_delete->swing_modes[6] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[6]))
            vPortFree(node_to_delete->swing_modes[6]);
        if (node_to_delete->swing_modes[7] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[7]))
            vPortFree(node_to_delete->swing_modes[7]);
        if (node_to_delete->swing_modes[8] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[8]))
            vPortFree(node_to_delete->swing_modes[8]);
        if (node_to_delete->swing_modes[9] != NULL && isPvPortMallocAllocated(node_to_delete->swing_modes[9]))
            vPortFree(node_to_delete->swing_modes[9]);
        /**
         * @brief 删除目标湿度信息
         *
         */
        if (node_to_delete->target_humidity_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->target_humidity_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->target_humidity_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->target_humidity_command_topic))
                vPortFree(node_to_delete->target_humidity_command_topic);
        }
        if (node_to_delete->target_humidity_command_template != NULL && isPvPortMallocAllocated(node_to_delete->target_humidity_command_template))
            vPortFree(node_to_delete->target_humidity_command_template);
        if (node_to_delete->target_humidity_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->target_humidity_state_topic))
            vPortFree(node_to_delete->target_humidity_state_topic);
        if (node_to_delete->target_humidity_state_template != NULL && isPvPortMallocAllocated(node_to_delete->target_humidity_state_template))
            vPortFree(node_to_delete->target_humidity_state_template);
        /**
         * @brief 删除动作信息
         *
         */

        if (node_to_delete->action_topic != NULL && isPvPortMallocAllocated(node_to_delete->action_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->action_topic);
            if (isPvPortMallocAllocated(node_to_delete->action_topic))
                vPortFree(node_to_delete->action_topic);
        }
        if (node_to_delete->action_template != NULL && isPvPortMallocAllocated(node_to_delete->action_template))
            vPortFree(node_to_delete->action_template);
        /**
         * @brief 删除温度信息
         *
         */
        if (node_to_delete->temperature_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->temperature_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->temperature_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->temperature_command_topic))
                vPortFree(node_to_delete->temperature_command_topic);
        }
        if (node_to_delete->temperature_command_template != NULL && isPvPortMallocAllocated(node_to_delete->temperature_command_template))
            vPortFree(node_to_delete->temperature_command_template);
        if (node_to_delete->temperature_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->temperature_state_topic))
            vPortFree(node_to_delete->temperature_state_topic);
        if (node_to_delete->temperature_state_template != NULL && isPvPortMallocAllocated(node_to_delete->temperature_state_template))
            vPortFree(node_to_delete->temperature_state_template);
        if (node_to_delete->temperature_unit != NULL && isPvPortMallocAllocated(node_to_delete->temperature_unit))
            vPortFree(node_to_delete->temperature_unit);
        if (node_to_delete->temperature_high_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->temperature_high_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->temperature_high_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->temperature_high_command_topic))
                vPortFree(node_to_delete->temperature_high_command_topic);
        }
        if (node_to_delete->temperature_high_command_template != NULL && isPvPortMallocAllocated(node_to_delete->temperature_high_command_template))
            vPortFree(node_to_delete->temperature_high_command_template);
        if (node_to_delete->temperature_high_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->temperature_high_state_topic))
            vPortFree(node_to_delete->temperature_high_state_topic);
        if (node_to_delete->temperature_high_state_template != NULL && isPvPortMallocAllocated(node_to_delete->temperature_high_state_template))
            vPortFree(node_to_delete->temperature_high_state_template);
        if (node_to_delete->temperature_low_command_topic != NULL && isPvPortMallocAllocated(node_to_delete->temperature_low_command_topic))
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->temperature_low_command_topic);
            if (isPvPortMallocAllocated(node_to_delete->temperature_low_command_topic))
                vPortFree(node_to_delete->temperature_low_command_topic);
        }
        if (node_to_delete->temperature_low_command_template != NULL && isPvPortMallocAllocated(node_to_delete->temperature_low_command_template))
            vPortFree(node_to_delete->temperature_low_command_template);
        if (node_to_delete->temperature_low_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->temperature_low_state_topic))
            vPortFree(node_to_delete->temperature_low_state_topic);
        if (node_to_delete->temperature_low_state_template != NULL && isPvPortMallocAllocated(node_to_delete->temperature_low_state_template))
            vPortFree(node_to_delete->temperature_low_state_template);
        if (node_to_delete->temperature_state_topic != NULL && isPvPortMallocAllocated(node_to_delete->temperature_state_topic))
            vPortFree(node_to_delete->temperature_state_topic);
        if (node_to_delete->temperature_state_template != NULL && isPvPortMallocAllocated(node_to_delete->temperature_state_template))
            vPortFree(node_to_delete->temperature_state_template);
        if (node_to_delete->temperature_unit != NULL && isPvPortMallocAllocated(node_to_delete->temperature_unit))
            vPortFree(node_to_delete->temperature_unit);
        if (node_to_delete->value_template != NULL && isPvPortMallocAllocated(node_to_delete->value_template))
            vPortFree(node_to_delete->value_template);
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}
#endif

#if CONFIG_ENTITY_ENABLE_SELECT

char *default_options[] = {"options 1", "options 2"};

static void homeAssistant_create_select_data(ha_select_t *select_new_node, cJSON *device_json)
{
    if (select_new_node == NULL)
    {
        HA_LOG_E("entity select buff is NULL\r\n");
        return;
    }
    cJSON *root = cJSON_CreateObject();

    if (select_new_node->name != NULL)
        cJSON_AddStringToObject(root, "name", select_new_node->name);
    if (unique_id == NULL && select_new_node->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(select_new_node->unique_id) + 4);
        memset(unique_id, 0, strlen(select_new_node->unique_id) + 4);
        sprintf(unique_id, "%s-%02x%02x", select_new_node->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    cJSON_AddStringToObject(root, "unique_id", unique_id);

    if (select_new_node->availability_mode != NULL)
        cJSON_AddStringToObject(root, "availability_mode", select_new_node->availability_mode);
    if (select_new_node->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", select_new_node->availability_template);
    if (select_new_node->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", select_new_node->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    // 开始创建新的实体数据

    if (select_new_node->command_template != NULL)
        cJSON_AddStringToObject(root, "command_template", select_new_node->command_template);

    if (select_new_node->command_topic == NULL)
    {
        select_new_node->command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SELECT) + strlen(unique_id) + strlen("///set"));
        memset(select_new_node->command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SELECT) + strlen(unique_id) + strlen("///set"));
        sprintf(select_new_node->command_topic, "%s/%s/%s/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SELECT, unique_id);
    }
    if (select_new_node->command_topic != NULL)
        cJSON_AddStringToObject(root, "command_topic", select_new_node->command_topic);

    if (select_new_node->state_topic == NULL)
    {
        select_new_node->state_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SELECT) + strlen(unique_id) + strlen("///state"));
        memset(select_new_node->state_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SELECT) + strlen(unique_id) + strlen("///state"));
        sprintf(select_new_node->state_topic, "%s/%s/%s/state", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SELECT, unique_id);
    }
    if (select_new_node->state_topic != NULL)
        cJSON_AddStringToObject(root, "state_topic", select_new_node->state_topic);
    if (!select_new_node->enabled_by_default)
        cJSON_AddFalseToObject(root, "enabled_by_default");
    if (select_new_node->encoding != NULL)
        cJSON_AddStringToObject(root, "encoding", select_new_node->encoding);
    if (select_new_node->entity_category != NULL)
        cJSON_AddStringToObject(root, "entity_category", select_new_node->entity_category);
    if (select_new_node->icon != NULL)
        cJSON_AddStringToObject(root, "icon", select_new_node->icon);
    if (select_new_node->json_attributes_template != NULL)
        cJSON_AddStringToObject(root, "json_attributes_template", select_new_node->json_attributes_template);
    if (select_new_node->json_attributes_topic != NULL)
        cJSON_AddStringToObject(root, "json_attributes_topic", select_new_node->json_attributes_topic);
    if (select_new_node->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", select_new_node->object_id);
    if (!select_new_node->optimistic)
        cJSON_AddFalseToObject(root, "optimistic");

    if (select_new_node->options == NULL)
    {
        select_new_node->options = default_options;
        select_new_node->options_numble = 2;
    }
    cJSON *options = cJSON_CreateArray();
    for (size_t i = 0; i < select_new_node->options_numble; i++)
    {
        cJSON_AddItemToArray(options, cJSON_CreateString(select_new_node->options[i]));
    }
    cJSON_AddItemToObject(root, "options", options);
    if (select_new_node->qos > 0)
        cJSON_AddNumberToObject(root, "qos", select_new_node->qos);
    if (select_new_node->retain > 0)
        cJSON_AddNumberToObject(root, "retain", select_new_node->retain);
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    select_new_node->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void entity_select_add_node(ha_select_t *select_new_node)
{
    ha_select_t *select_list_handle = ha_device->entity_select->select_list->prev;

    homeAssistant_create_select_data(select_new_node, homeAssistant_device_create());

    if (select_new_node->entity_config_topic == NULL)
    {
        select_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SELECT) + strlen(unique_id) + strlen("///config"));
        memset(select_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SELECT) + strlen(unique_id) + strlen("///config"));
        sprintf(select_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SELECT, unique_id);
    }

    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(select_new_node->entity_config_topic, select_new_node->config_data, 0, 1);
        if (select_new_node->command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(select_new_node->command_topic, 1);
        homeAssistant_mqtt_port_public(select_new_node->state_topic, select_new_node->options[select_new_node->option], 1, 1);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }
    ha_select_t *select_new_node_temp = pvPortMalloc(sizeof(ha_select_t));
    memset(select_new_node_temp, 0, sizeof(ha_select_t));
    memcpy(select_new_node_temp, select_new_node, sizeof(ha_select_t));

    select_list_handle->next = select_new_node_temp;
    select_new_node_temp->prev = select_list_handle;
    select_new_node_temp->next = ha_device->entity_select->select_list;
    ha_device->entity_select->select_list->prev = select_new_node_temp;
    vPortFree(select_new_node_temp->config_data);
    vPortFree(unique_id);
    unique_id = NULL;
}

static bool entity_select_delete_node(char *unique_id)
{
    ha_select_t *node_to_delete = ha_device->entity_select->select_list->next;

    while (node_to_delete != ha_device->entity_select->select_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_select->select_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_select->select_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }

        if (node_to_delete->command_topic != NULL && isPvPortMallocAllocated(node_to_delete->command_topic))
        {

            homeAssistant_mqtt_port_unsubscribe(node_to_delete->command_topic);
            if (isPvPortMallocAllocated(node_to_delete->command_topic))
                vPortFree(node_to_delete->command_topic);
        }
        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);
        if (node_to_delete->availability_mode != NULL && isPvPortMallocAllocated(node_to_delete->availability_mode))
            vPortFree(node_to_delete->availability_mode);
        if (node_to_delete->availability_template != NULL && isPvPortMallocAllocated(node_to_delete->availability_template))
            vPortFree(node_to_delete->availability_template);

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);

        if (node_to_delete->command_template != NULL && isPvPortMallocAllocated(node_to_delete->command_template))
            vPortFree(node_to_delete->command_template);

        if (node_to_delete->state_topic != NULL && isPvPortMallocAllocated(node_to_delete->state_topic))
            vPortFree(node_to_delete->state_topic);

        if (node_to_delete->value_template != NULL && isPvPortMallocAllocated(node_to_delete->value_template))
            vPortFree(node_to_delete->value_template);
    }

    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}
static int get_ac_type_enum(ha_select_t *select, char *ac_type, int len)
{
    if (select == NULL || ac_type == NULL)
    {
        HA_LOG_E("params is NULL\r\n");
        return -1;
    }
    uint8_t i = 0;
    for (i = 0; i < select->options_numble; i++)
    {
        if (!strncmp(select->options[i], ac_type, len))
        {

            return i;
        }
    }
    return -1;
}
#endif

#if CONFIG_ENTITY_ENABLE_BUTTON

static void homeAssistant_create_button_data(ha_btn_entity_t *button_entity, cJSON *device_json)
{
    if (button_entity == NULL)
    {
        HA_LOG_E("entity button buff is NULL\r\n");
        return;
    }
    cJSON *root = cJSON_CreateObject();
    if (button_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", button_entity->name);
    if (button_entity->device_class != NULL)
        cJSON_AddStringToObject(root, "device_class", button_entity->device_class);
    if (unique_id == NULL && button_entity->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(button_entity->unique_id) + 4);
        memset(unique_id, 0, strlen(button_entity->unique_id));
        sprintf(unique_id, "%s-%02x%02x", button_entity->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    cJSON_AddStringToObject(root, "unique_id", unique_id);
    if (button_entity->object_id != NULL)
        cJSON_AddStringToObject(root, "object_id", button_entity->object_id);

    if (button_entity->command_topic == NULL)
    {
        button_entity->command_topic = pvPortMalloc(strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        memset(button_entity->command_topic, 0, strlen(ha_device->name) + 6 * 2 + strlen(unique_id) + strlen("///set"));
        sprintf(button_entity->command_topic, "%s/%02x%02x%02x%02x%02x%02x/%s/set", ha_device->name, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], unique_id);
    }
    cJSON_AddStringToObject(root, "command_topic", button_entity->command_topic);
    if (button_entity->command_template != NULL)
        cJSON_AddStringToObject(root, "command_template", button_entity->command_template);
    if (button_entity->payload_press != NULL)
        cJSON_AddStringToObject(root, "payload_press", button_entity->payload_press);

    if (button_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", button_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (button_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", button_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (button_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", button_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);
    if (button_entity->qos)
        cJSON_AddNumberToObject(root, "qos", button_entity->qos);
    if (button_entity->retain)
        cJSON_AddTrueToObject(root, "retain");
    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    if (button_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", button_entity->icon);

    button_entity->config_data = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);
}

/**
 * @brief entity_button_add_node
 *         添加一个按钮实体
 * @param button_new_node
 */
static void entity_button_add_node(ha_btn_entity_t *button_new_node)
{
    assert(ha_device->entity_button->button_list);

    // 添加新的节点
    ha_btn_entity_t *button_list_handle = ha_device->entity_button->button_list->prev;

    // 开始创建新的实体数据
    homeAssistant_create_button_data(button_new_node, homeAssistant_device_create());

    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (button_new_node->entity_config_topic == NULL)
        {
            button_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_BUTTON) + strlen(unique_id) + strlen("///config"));
            memset(button_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_BUTTON) + strlen(unique_id) + strlen("///config"));
            sprintf(button_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_BUTTON, unique_id);
            homeAssistant_mqtt_port_public(button_new_node->entity_config_topic, button_new_node->config_data, 0, 1);
            vPortFree(button_new_node->entity_config_topic);
        }
        else
            homeAssistant_mqtt_port_public(button_new_node->entity_config_topic, button_new_node->config_data, 0, 1);
        if (button_new_node->command_topic != NULL)
            homeAssistant_mqtt_port_subscribe(button_new_node->command_topic, 1);
        if (button_new_node->availability_topic != NULL)
            homeAssistant_mqtt_port_public(button_new_node->availability_topic, button_new_node->payload_available, 0, 0);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }
    ha_btn_entity_t *button_new_node_temp = pvPortMalloc(sizeof(ha_btn_entity_t));
    memset(button_new_node_temp, 0, sizeof(ha_btn_entity_t));
    memcpy(button_new_node_temp, button_new_node, sizeof(ha_btn_entity_t));
    // 插入节点
    button_list_handle->next = button_new_node_temp;
    button_new_node_temp->prev = button_list_handle;
    button_new_node_temp->next = ha_device->entity_button->button_list;
    ha_device->entity_button->button_list->prev = button_new_node_temp;

    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(button_new_node->config_data);
}

static bool entity_button_delete_node(char *unique_id)
{
    ha_btn_entity_t *node_to_delete = ha_device->entity_button->button_list->next;

    while (node_to_delete != ha_device->entity_button->button_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_button->button_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_button->button_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);
        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);
        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);
        if (node_to_delete->command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->command_topic);
            if (isPvPortMallocAllocated(node_to_delete->command_topic))
                vPortFree(node_to_delete->command_topic);
        }

        if (node_to_delete->command_template != NULL && isPvPortMallocAllocated(node_to_delete->command_template))
            vPortFree(node_to_delete->command_template);
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}
#endif

#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
/**
 * @brief homeAssistant_create_device_trigger_data 创建触发器实体
 *
 * @param devTrig_entity
 * @param device_json
 */
static char *devTrig_type_str[] = {"button_short_press", "button_short_release", "button_long_press", "button_long_release", "button_double_press", "button_triple_press", "button_quadruple_press", "button_quintuple_press"};
static char *devTrig_subtype_strp[] = {"turn_on", "turn_off", "button_1", "button_2", "button_3", "button_4", "button_5", "button_6"};

static void homeAssistant_create_device_trigger_data(ha_devTrig_entity_t *devTrig_entity, cJSON *device_json)
{
    if (devTrig_entity == NULL)
    {
        HA_LOG_E("entity button buff is NULL\r\n");
        return;
    }
    cJSON *root = cJSON_CreateObject();

    if (devTrig_entity->automation_type == NULL)
        devTrig_entity->automation_type = "trigger";
    cJSON_AddStringToObject(root, "automation_type", devTrig_entity->automation_type);
    if (devTrig_entity->platform == NULL)
        devTrig_entity->platform = "device_automation";
    cJSON_AddStringToObject(root, "platform", devTrig_entity->platform);
    // 判断type 是否是支持的类型
    for (size_t i = 0; i < sizeof(devTrig_type_str) / sizeof(devTrig_type_str[0]); i++)
    {
        if (!strcmp(devTrig_entity->type, devTrig_type_str[i]))
        {
            cJSON_AddStringToObject(root, "type", devTrig_entity->type);
            devTrig_entity->type = devTrig_type_str[i];
            break;
        }
        else
        {
            cJSON_AddStringToObject(root, "type", "spammed");
        }
    }
    // 判断 subtype 是否是支持的类型
    for (size_t i = 0; i < sizeof(devTrig_subtype_strp) / sizeof(devTrig_subtype_strp[0]); i++)
    {
        if (!strcmp(devTrig_entity->subtype, devTrig_subtype_strp[i]))
        {
            cJSON_AddStringToObject(root, "subtype", devTrig_entity->subtype);
            devTrig_entity->subtype = devTrig_subtype_strp[i];
            break;
        }
        else
        {
            cJSON_AddStringToObject(root, "subtype", "button_1");
        }
    }
    if (devTrig_entity->topic == NULL)
    {
        devTrig_entity->topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + 6 * 2 + strlen(devTrig_entity->type) + strlen("//"));
        memset(devTrig_entity->topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + 6 * 2 + strlen(devTrig_entity->type) + strlen("//"));
        sprintf(devTrig_entity->topic, "%s/%02x%02x%02x%02x%02x%02x/%s", CONFIG_HA_AUTOMATIC_DISCOVERY, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], devTrig_entity->type);
    }
    cJSON_AddStringToObject(root, "topic", devTrig_entity->topic);

    if (devTrig_entity->qos > 0)
        cJSON_AddNumberToObject(root, "qos", devTrig_entity->qos);

    if (devTrig_entity->payload == NULL)
        devTrig_entity->payload = devTrig_entity->type;
    cJSON_AddStringToObject(root, "payload", devTrig_entity->payload);

    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    devTrig_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void entity_device_trigger_add_node(ha_devTrig_entity_t *devTrig_new_node)
{
    assert(ha_device->entity_devTrig->devTrig_list);

    // 添加新的节点
    ha_devTrig_entity_t *devTrig_list_handle = ha_device->entity_devTrig->devTrig_list->prev;
    // 开始创建新的实体数据
    homeAssistant_create_device_trigger_data(devTrig_new_node, homeAssistant_device_create());
    if (devTrig_new_node->entity_config_topic == NULL)
    {
        devTrig_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_DEVICE_TRIGGER) + 12 + strlen(devTrig_new_node->type) + strlen("////config"));
        memset(devTrig_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_DEVICE_TRIGGER) + 12 + strlen(devTrig_new_node->type) + strlen("////config"));
        sprintf(devTrig_new_node->entity_config_topic, "%s/%s/%02x%02x%02x%02x%02x%02x/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_DEVICE_TRIGGER, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], devTrig_new_node->type);
    }

    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(devTrig_new_node->entity_config_topic, devTrig_new_node->config_data, 0, 1);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }
    ha_devTrig_entity_t *devTrig_new_node_temp = pvPortMalloc(sizeof(ha_devTrig_entity_t));
    memset(devTrig_new_node_temp, 0, sizeof(ha_devTrig_entity_t));
    memcpy(devTrig_new_node_temp, devTrig_new_node, sizeof(ha_devTrig_entity_t));

    // 插入节点
    devTrig_list_handle->next = devTrig_new_node_temp;
    devTrig_new_node_temp->prev = devTrig_list_handle;
    devTrig_new_node_temp->next = ha_device->entity_devTrig->devTrig_list;
    ha_device->entity_devTrig->devTrig_list->prev = devTrig_new_node_temp;

    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(devTrig_new_node->config_data);
}

static bool entity_devTrig_delete_node(char *unique_id)
{
    ha_devTrig_entity_t *node_to_delete = ha_device->entity_devTrig->devTrig_list->next;

    while (node_to_delete != ha_device->entity_devTrig->devTrig_list)
    {
        if (!strncmp(node_to_delete->subtype, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_devTrig->devTrig_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_devTrig->devTrig_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }

        if (node_to_delete->topic != NULL && isPvPortMallocAllocated(node_to_delete->topic))
        {
            vPortFree(node_to_delete->topic);
        }
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）
    vPortFree(node_to_delete);

    return true;
}
#endif

#if CONFIG_ENTITY_ENABLE_SCENE
static void homeAssistant_create_scene_data(ha_scene_entity_t *scene_entity, cJSON *device_json)
{
    if (scene_entity == NULL)
    {
        HA_LOG_E("entity button buff is NULL\r\n");
        return;
    }
    cJSON *root = cJSON_CreateObject();
    if (scene_entity->name != NULL)
        cJSON_AddStringToObject(root, "name", scene_entity->name);
    if (unique_id == NULL && scene_entity->unique_id != NULL)
    {
        unique_id = pvPortMalloc(strlen(scene_entity->unique_id) + 4);
        memset(unique_id, 0, strlen(scene_entity->unique_id) + 4);
        sprintf(unique_id, "%s-%02x%02x", scene_entity->unique_id, STA_MAC[4], STA_MAC[5]);
        HA_LOG_F("unique_id =%s\r\n", unique_id);
    }
    if (unique_id != NULL)
        cJSON_AddStringToObject(root, "unique_id", unique_id);

    if (scene_entity->availability_mode != NULL)
        cJSON_AddStringToObject(root, "availability_mode", scene_entity->availability_mode);
    if (scene_entity->availability_template != NULL)
        cJSON_AddStringToObject(root, "availability_template", scene_entity->availability_template);
    if (scene_entity->availability_topic != NULL)
        cJSON_AddStringToObject(root, "availability_topic", scene_entity->availability_topic);
    else
        cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);

    if (scene_entity->command_topic == NULL)
    {
        scene_entity->command_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SCENE) + strlen(unique_id));
        memset(scene_entity->command_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SCENE) + strlen(unique_id));
        sprintf(scene_entity->command_topic, "%s/%s/%s/set", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SCENE, unique_id);
    }
    if (scene_entity->command_topic != NULL)
        cJSON_AddStringToObject(root, "command_topic", scene_entity->command_topic);
    if (scene_entity->entity_category != NULL)
        cJSON_AddStringToObject(root, "entity_category", scene_entity->entity_category);
    if (scene_entity->icon != NULL)
        cJSON_AddStringToObject(root, "icon", scene_entity->icon);
    if (scene_entity->entity_picture != NULL)
        cJSON_AddStringToObject(root, "entity_picture", scene_entity->entity_picture);
    if (scene_entity->json_attributes_template != NULL)
        cJSON_AddStringToObject(root, "json_attributes_template", scene_entity->json_attributes_template);
    if (scene_entity->json_attributes_topic != NULL)
        cJSON_AddStringToObject(root, "json_attributes_topic", scene_entity->json_attributes_topic);

    if (scene_entity->payload_available != NULL)
        cJSON_AddStringToObject(root, "payload_available", scene_entity->payload_available);
    else
        cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);

    if (scene_entity->payload_not_available != NULL)
        cJSON_AddStringToObject(root, "payload_not_available", scene_entity->payload_not_available);
    else
        cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);

    if (scene_entity->payload_on != NULL)
        cJSON_AddStringToObject(root, "payload_on", scene_entity->payload_on);
    else
        cJSON_AddStringToObject(root, "payload_on", "ON");

    if (scene_entity->platform != NULL)
        cJSON_AddStringToObject(root, "platform", scene_entity->platform);
    else
        cJSON_AddStringToObject(root, "platform", "scene");

    if (scene_entity->qos > 0)
        cJSON_AddNumberToObject(root, "qos", scene_entity->qos);
    if (scene_entity->retain)
        cJSON_AddTrueToObject(root, "retain");

    if (device_json != NULL)
        cJSON_AddItemToObject(root, "device", device_json);
    scene_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void entity_scene_add_node(ha_scene_entity_t *scene_new_node)
{
    assert(ha_device->entity_scene->scene_list);

    // 添加新的节点
    ha_scene_entity_t *scene_list_handle = ha_device->entity_scene->scene_list->prev;

    // 开始创建新的实体数据
    homeAssistant_create_scene_data(scene_new_node, homeAssistant_device_create());
    if (scene_new_node->entity_config_topic == NULL)
    {
        scene_new_node->entity_config_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SCENE) + strlen(unique_id) + strlen("///config"));
        memset(scene_new_node->entity_config_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + strlen(CONFIG_HA_ENTITY_SCENE) + strlen(unique_id) + strlen("///config"));
        sprintf(scene_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SCENE, unique_id);
    }

    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        homeAssistant_mqtt_port_public(scene_new_node->entity_config_topic, scene_new_node->config_data, 1, 1);
        homeAssistant_mqtt_port_subscribe(scene_new_node->command_topic, 1);
    }
    else
    {
        HA_LOG_E("MQTT server is diconnenct\r\n");
    }

    ha_scene_entity_t *scene_new_node_temp = pvPortMalloc(sizeof(ha_scene_entity_t));
    memset(scene_new_node_temp, 0, sizeof(ha_scene_entity_t));
    memcpy(scene_new_node_temp, scene_new_node, sizeof(ha_scene_entity_t));
    // 插入节点

    scene_list_handle->next = scene_new_node_temp;
    scene_new_node_temp->prev = scene_list_handle;
    scene_new_node_temp->next = ha_device->entity_scene->scene_list;
    ha_device->entity_scene->scene_list->prev = scene_new_node_temp;
    vPortFree(unique_id);
    unique_id = NULL;
    vPortFree(scene_new_node->config_data);
}
/**
 * @brief
 *
 * @param unique_id
 * @return true
 * @return false
 */
static bool entity_scene_delete_node(char *unique_id)
{
    ha_scene_entity_t *node_to_delete = ha_device->entity_scene->scene_list->next;

    while (node_to_delete != ha_device->entity_scene->scene_list)
    {
        if (!strncmp(node_to_delete->unique_id, unique_id, strlen(unique_id)))
        {
            break;
        }
        node_to_delete = node_to_delete->next;
    }
    // 找不到的话就返回0
    if (node_to_delete == ha_device->entity_scene->scene_list)
    {
        return false;
    }
    /*开始删除实体*/

    // 从链表删除实体
    if (node_to_delete->prev != NULL)
    {
        node_to_delete->prev->next = node_to_delete->next;
    }
    else
    {
        // 更新头指针（如果删除的是头节点）
        ha_device->entity_scene->scene_list = node_to_delete->next;
    }
    // 处理后继节点
    if (node_to_delete->next != NULL)
    {
        node_to_delete->next->prev = node_to_delete->prev;
    }
    // 从平台删除实体
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (node_to_delete->entity_config_topic != NULL)
        {
            homeAssistant_mqtt_port_public(node_to_delete->entity_config_topic, "", 1, 1);
            if (isPvPortMallocAllocated(node_to_delete->entity_config_topic))
                vPortFree(node_to_delete->entity_config_topic);
        }

        if (node_to_delete->command_topic != NULL)
        {
            homeAssistant_mqtt_port_unsubscribe(node_to_delete->command_topic);
            if (isPvPortMallocAllocated(node_to_delete->command_topic))
                vPortFree(node_to_delete->command_topic);
        }

        if (node_to_delete->availability_topic != NULL && isPvPortMallocAllocated(node_to_delete->availability_topic))
            vPortFree(node_to_delete->availability_topic);

        if (node_to_delete->json_attributes_topic != NULL && isPvPortMallocAllocated(node_to_delete->json_attributes_topic))
            vPortFree(node_to_delete->json_attributes_topic);

        if (node_to_delete->payload_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_available))
            vPortFree(node_to_delete->payload_available);

        if (node_to_delete->payload_not_available != NULL && isPvPortMallocAllocated(node_to_delete->payload_not_available))
            vPortFree(node_to_delete->payload_not_available);
    }
    // 释放节点内存（假设使用FreeRTOS内存管理）

    vPortFree(node_to_delete);

    return true;
}
#endif
/**
 * @brief homeAssistant_get_command
 *      获取 服务器指令事件，并把相应实体指针指向对应的实体
 * @param topic
 * @param topic_len
 * @param data
 * @param data_len
 * @return ha_event_t
 */
ha_event_t homeAssistant_get_command(const char *topic, unsigned short topic_len, const char *data, unsigned short data_len)
{
    if (topic == NULL || data == NULL)
    {
        HA_LOG_E("params is NULL\r\n");
        return HA_EVENT_NONE;
    }
    ha_event_t event = HA_EVENT_NONE;
    // 识别homeassistant 平台的出生消息
    if (!strncmp(topic, CONFIG_HA_STATUS_TOPIC, topic_len))
    {
        if (!strncmp(data, CONFIG_HA_STATUS_MESSAGE_ON, data_len))
        {
            event = HA_EVENT_HOMEASSISTANT_STATUS_ONLINE;
            ha_device->homeassistant_online = true;
        }
        else
        {
            event = HA_EVENT_HOMEASSISTANT_STATUS_OFFLINE;
            ha_device->homeassistant_online = false;
        }
        return event;
    }
    // 查找出switch 实体
#if CONFIG_ENTITY_ENABLE_SWITCH
    ha_sw_entity_t *switch_cur = ha_device->entity_switch->switch_list->next;

    while (switch_cur != ha_device->entity_switch->switch_list)
    {
        if (!strncmp(topic, switch_cur->command_topic, topic_len))
        {
            if (switch_cur->payload_on != NULL)
                switch_cur->switch_state = strncmp(data, switch_cur->payload_on, data_len) ? false : true;
            else
                switch_cur->switch_state = strncmp(data, "ON", data_len) ? false : true;
            event = HA_EVENT_MQTT_COMMAND_SWITCH;
            // 找出该实体之后，方便后面操作实体
            ha_device->entity_switch->command_switch = switch_cur;
            return event;
        }
        // 指针指向下一个节点
        if (switch_cur->next == ha_device->entity_switch->switch_list)
            break;
        else
            switch_cur = switch_cur->next;
    }
#endif
    // 查找相应的Light 实体
#if CONFIG_ENTITY_ENABLE_LIGHT
    ha_lh_entity_t *light_cur = ha_device->entity_light->light_list->next;
    while (light_cur != ha_device->entity_light->light_list)
    {
        // 识别开关信息
        if (light_cur->command_topic != NULL && !strncmp(topic, light_cur->command_topic, topic_len))
        {

            if (light_cur->payload_on != NULL)
                light_cur->light_state = strncmp(data, light_cur->payload_on, data_len) == 0 ? true : false;
            else
                light_cur->light_state = strncmp(data, "ON", data_len) == 0 ? true : false;

            event = HA_EVENT_MQTT_COMMAND_LIGHT_SWITCH;
            ha_device->entity_light->command_light = light_cur;
            return event;
        }
        // 识别RGB 信息
        if (light_cur->rgb.rgb_command_topic != NULL && !strncmp(topic, light_cur->rgb.rgb_command_topic, topic_len))
        {
            homeAssistant_get_light_rgb(light_cur, data, data_len);
            event = HA_EVENT_MQTT_COMMAND_LIGHT_RGB_UPDATE;
            light_cur->light_state = true;
            ha_device->entity_light->command_light = light_cur;
            return event;
        }
        // 识别HS 信息
        if (light_cur->hs.hs_command_topic != NULL && !strncmp(topic, light_cur->hs.hs_command_topic, topic_len))
        {
            homeAssistant_get_light_hs(light_cur, data, data_len);
            light_cur->light_state = true;
            event = HA_EVENT_MQTT_COMMAND_LIGHT_HS_UPDATE;
            ha_device->entity_light->command_light = light_cur;
            return event;
        }

        if (light_cur->next == ha_device->entity_light->light_list)
            break;
        else
            light_cur = light_cur->next;
    }
#endif

    // 查找相应的tet 实体
#if CONFIG_ENTITY_ENABLE_TEXT
    ha_text_entity_t *text_cur = ha_device->entity_text->text_list->next;

    while (text_cur != ha_device->entity_text->text_list)
    {
        if (!strncmp(topic, text_cur->command_topic, topic_len))
        {
            if (text_cur->text_value == NULL)
                text_cur->text_value = pvPortMalloc(data_len);
            memset(text_cur->text_value, 0, data_len);
            strncpy(text_cur->text_value, data, data_len);
            event = HA_EVENT_MQTT_COMMAND_TEXT_VALUE;
            ha_device->entity_text->command_text = text_cur;
            return event;
        }
        if (text_cur->next == ha_device->entity_text->text_list)
            break;
        else
            text_cur = text_cur->next;
    }
#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    ha_number_entity_t *number_cur = ha_device->entity_number->number_list->next;

    while (number_cur != ha_device->entity_number->number_list)
    {
        if (!strncmp(topic, number_cur->command_topic, topic_len))
        {
            if (number_cur->number_value == NULL)
            {
                number_cur->number_value = pvPortMalloc(data_len);
            }
            memset(number_cur->number_value, 0, data_len);
            strncpy(number_cur->number_value, data, data_len);
            if (number_cur->step)
            {
                char *number_str = number_cur->number_value;
                while (*number_str != '\0')
                {
                    if (*number_str == '.')
                    {
                        number_str += number_cur->step;
                        number_str += 1;
                        *number_str = '\0';
                        break;
                    }
                    number_str++;
                }
            }
            number_cur->number = (double)(number_cur->step ? atof(number_cur->number_value) : atoi(number_cur->number_value));

            event = HA_EVENT_MQTT_COMMAND_NUMBER_VALUE;
            ha_device->entity_number->command_number = number_cur;
            return event;
        }
        if (number_cur->next == ha_device->entity_number->number_list)
            break;
        else
            number_cur = number_cur->next;
    }
#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    ha_climateHVAC_t *climateHVAC_cur = ha_device->entity_climateHVAC->climateHVAC_list->next;
    while (climateHVAC_cur != ha_device->entity_climateHVAC->climateHVAC_list)
    {
        // 通过开关
        if (climateHVAC_cur->power_command_topic != NULL)
        {
            if (!strncmp(topic, climateHVAC_cur->power_command_topic, topic_len))
            {
                if (!strncmp(data, climateHVAC_cur->payload_on, data_len))
                {
                    climateHVAC_cur->power_state = true;
                }
                else
                {
                    climateHVAC_cur->power_state = false;
                }
                event = HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_POWER;
                ha_device->entity_climateHVAC->command_climateHVAC = climateHVAC_cur;
                return event;
            }
        }
        event = HA_EVENT_NONE;
        // 识别出模式设置事件
        if (climateHVAC_cur->mode_command_topic != NULL)
        {
            if (!strncmp(topic, climateHVAC_cur->mode_command_topic, topic_len))
            {
                cJSON *root = cJSON_Parse(data);
                if (root == NULL)
                {
                    HA_LOG_E("%.*s is NO json\r\n", data_len, data);
                    cJSON_Delete(root);
                }
                cJSON *modes = cJSON_GetObjectItem(root, "mode");

                if (!memcmp(modes->valuestring, "off", strlen(modes->valuestring)))
                {
                    climateHVAC_cur->power_state = false;
                    event = HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_POWER;
                    ha_device->entity_climateHVAC->command_climateHVAC = climateHVAC_cur;
                    cJSON_Delete(root);
                    return event;
                }
                if (climateHVAC_cur->modes[0] == NULL)
                    memcpy(climateHVAC_cur->modes, modes_def, sizeof(modes_def));
                for (size_t i = 0; climateHVAC_cur->modes[i] != NULL; i++)
                {
                    if (!memcmp(modes->valuestring, climateHVAC_cur->modes[i], strlen(modes->valuestring)))
                    {
                        climateHVAC_cur->modes_type = i;
                        goto _exit;
                    }
                }

            _exit:
                cJSON_Delete(root);
                event = HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_MODES;
                ha_device->entity_climateHVAC->command_climateHVAC = climateHVAC_cur;
                return event;
            }
        }
        event = HA_EVENT_NONE;
        // 识别温度命令
        if (climateHVAC_cur->temperature_command_topic != NULL)
        {
            if (!strncmp(topic, climateHVAC_cur->temperature_command_topic, topic_len))
            {
                cJSON *root = cJSON_Parse(data);
                if (root == NULL)
                {
                    HA_LOG_E("%.*s is NO json\r\n", data_len, data);
                    cJSON_Delete(root);
                }
                cJSON *temperature = cJSON_GetObjectItem(root, "temperature");
                climateHVAC_cur->temperature_value = temperature->valuedouble;
                cJSON_Delete(root);
                event = HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_TEMP;
                ha_device->entity_climateHVAC->command_climateHVAC = climateHVAC_cur;
                return event;
            }
            else
            {
                event = HA_EVENT_NONE;
            }
        }
        // 识别送风强度控制
        if (climateHVAC_cur->fan_mode_command_topic != NULL)
        {

            if (!strncmp(topic, climateHVAC_cur->fan_mode_command_topic, topic_len))
            {
                if (climateHVAC_cur->fan_modes[0] == NULL)
                    memcpy(climateHVAC_cur->fan_modes, fan_modes_def, sizeof(fan_modes_def));
                for (size_t i = 0; climateHVAC_cur->fan_modes[i] != NULL; i++)
                {
                    if (!strncmp(data, climateHVAC_cur->fan_modes[i], data_len))
                    {
                        climateHVAC_cur->fan_modes_type = i;
                        goto _fammode_exit;
                    }
                }
            _fammode_exit:
                event = HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_FAN_MODES;
                ha_device->entity_climateHVAC->command_climateHVAC = climateHVAC_cur;
                return event;
            }
            else
            {
                event = HA_EVENT_NONE;
            }
        }

        if (climateHVAC_cur->next == ha_device->entity_climateHVAC->climateHVAC_list)
            break;
        else
            climateHVAC_cur = climateHVAC_cur->next;
    }

#endif

#if CONFIG_ENTITY_ENABLE_SELECT
    ha_select_t *select_cur = ha_device->entity_select->select_list->next;
    if (!strncmp(topic, select_cur->command_topic, topic_len))
    {
        while (select_cur != ha_device->entity_select->select_list)
        {

            // if (select_cur->options_value==NULL)select_cur->options_value = pvPortMalloc(256);
            // memset(select_cur->options_value, 0, 256);
            // memcpy(select_cur->options_value, data, data_len);
            select_cur->option = get_ac_type_enum(select_cur, data, data_len);
            // HA_LOG_D("select_cur->options_value=%s\r\n", select_cur->options[select_cur->option]);
            event = HA_EVENT_MQTT_COMMAND_SELECT_VALUE;
            ha_device->entity_select->command_select = select_cur;
            return event;

            if (select_cur->next == ha_device->entity_select->select_list)
                break;
            else
                select_cur = select_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_BUTTON
    ha_btn_entity_t *button_cur = ha_device->entity_button->button_list->next;

    while (button_cur != ha_device->entity_button->button_list)
    {
        if (!strncmp(topic, button_cur->command_topic, topic_len))
        {

            event = HA_EVENT_MQTT_COMMAND_BUTTON;

            // 找出该实体之后，方便后面操作实体
            ha_device->entity_button->command_button = button_cur;
            return event;
        }
        // 指针指向下一个节点
        if (button_cur->next == ha_device->entity_button->button_list)
            break;
        else
            button_cur = button_cur->next;
    }
#endif
#if CONFIG_ENTITY_ENABLE_SCENE
    ha_scene_entity_t *scene_cur = ha_device->entity_scene->scene_list->next;

    while (scene_cur != ha_device->entity_scene->scene_list)
    {
        if (!strncmp(topic, scene_cur->command_topic, topic_len))
        {

            event = HA_EVENT_MQTT_COMMAND_SCENE_VALUE;
            if (scene_cur->payload_on == NULL)
            {
                scene_cur->payload_on = pvPortMalloc(data_len);
                memset(scene_cur->payload_on, 0, data_len);
            }
            strncpy(scene_cur->payload_on, data, data_len);
            scene_cur->scene_state = strcmp(scene_cur->payload_on, "ON") == 0 ? 1 : 0;
            // 找出该实体之后，方便后面操作实体
            ha_device->entity_scene->command_scene = scene_cur;
            return event;
        }
        // 指针指向下一个节点
        if (scene_cur->next == ha_device->entity_scene->scene_list)
            break;
        else
            scene_cur = scene_cur->next;
    }
#endif
    return event;
}

// 单独更新所有实体的配置，当homeAssistant重启时更新实体上线
void update_all_entity_to_homeassistant(void)
{
    // 更新所有switch 实体
    int ret = 0;
    homeAssistant_device_send_status(HOMEASSISTANT_STATUS_OFFLINE);
#if CONFIG_ENTITY_ENABLE_SWITCH

    ha_sw_entity_t *switch_cur = ha_device->entity_switch->switch_list->next;
    if (switch_cur != ha_device->entity_switch->switch_list)
    {
        while (switch_cur != ha_device->entity_switch->switch_list)
        {
            if (switch_cur->entity_config_topic != NULL)
            {
                homeAssistant_create_switch_data(switch_cur, homeAssistant_device_create());
                homeAssistant_mqtt_port_public(switch_cur->entity_config_topic, switch_cur->config_data, 0, 0);
                vPortFree(switch_cur->config_data);
            }
            // 实体上线
            if (switch_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(switch_cur->availability_topic, switch_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 实体上报当前状态
            if (ha_device->entity_switch->command_switch != NULL)
                homeAssistant_device_send_entity_state(CONFIG_HA_ENTITY_SWITCH, ha_device->entity_switch->command_switch, ha_device->entity_switch->command_switch->switch_state);
            else
                homeAssistant_device_send_entity_state(CONFIG_HA_ENTITY_SWITCH, switch_cur, switch_cur->switch_state);

            switch_cur = switch_cur->next;
        }
    }
#endif
    // 更新所有light 实体
#if CONFIG_ENTITY_ENABLE_LIGHT
    ha_lh_entity_t *light_cur = ha_device->entity_light->light_list->next;
    if (light_cur != ha_device->entity_light->light_list)
    {
        while (light_cur != ha_device->entity_light->light_list)
        {
            homeAssistant_create_light_data(light_cur, homeAssistant_device_create());
            ret = homeAssistant_mqtt_port_public(light_cur->entity_config_topic, light_cur->config_data, 0, 0);
            vPortFree(light_cur->config_data);
            // 实体上线
            if (light_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(light_cur->availability_topic, light_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 实体上报当前状态
            if (ha_device->entity_light->command_light != NULL)
                homeAssistant_device_send_entity_state(CONFIG_HA_ENTITY_LIGHT, ha_device->entity_light->command_light, ha_device->entity_light->command_light->light_state);
            else
                homeAssistant_device_send_entity_state(CONFIG_HA_ENTITY_LIGHT, light_cur, light_cur->light_state);
            light_cur = light_cur->next;
        }
    }

#endif
    // 更新所有sensor 实体
#if CONFIG_ENTITY_ENABLE_SENSOR
    ha_sensor_entity_t *sensor_cur = ha_device->entity_sensor->sensor_list->next;
    if (sensor_cur != ha_device->entity_sensor->sensor_list)
    {
        while (sensor_cur != ha_device->entity_sensor->sensor_list)
        {
            if (sensor_cur->entity_config_topic != NULL)
            {
                homeAssistant_create_sensor_data(sensor_cur, homeAssistant_device_create());
                homeAssistant_mqtt_port_public(sensor_cur->entity_config_topic, sensor_cur->config_data, 0, 0);
                vPortFree(sensor_cur->config_data);
            }
            // 实体上线
            if (sensor_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(sensor_cur->availability_topic, sensor_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 更新实体状态
            homeAssistant_device_send_entity_state(ha_device->entity_sensor->entity_type, sensor_cur, 0);
            sensor_cur = sensor_cur->next;
        }
    }

#endif
    // 更新所有binary_sensor 实体
#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
    ha_Bsensor_entity_t *binary_sensor_cur = ha_device->entity_binary_sensor->binary_sensor_list->next;
    if (binary_sensor_cur != ha_device->entity_binary_sensor->binary_sensor_list)
    {
        while (binary_sensor_cur != ha_device->entity_binary_sensor->binary_sensor_list)
        {
            homeAssistant_create_binary_sensor_data(binary_sensor_cur, homeAssistant_device_create());
            homeAssistant_mqtt_port_public(binary_sensor_cur->entity_config_topic, binary_sensor_cur->config_data, 0, 1);
            vPortFree(binary_sensor_cur->config_data);
            // 实体上线
            if (binary_sensor_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(binary_sensor_cur->availability_topic, binary_sensor_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 更新实体状态
            homeAssistant_device_send_entity_state(ha_device->entity_binary_sensor->entity_type, binary_sensor_cur, binary_sensor_cur->state);
            binary_sensor_cur = binary_sensor_cur->next;
        }
    }
#endif

#if CONFIG_ENTITY_ENABLE_TEXT
    ha_text_entity_t *text_cur = ha_device->entity_text->text_list->next;
    if (text_cur != ha_device->entity_text->text_list)
    {
        while (text_cur != ha_device->entity_text->text_list)
        {
            homeAssistant_create_text_data(text_cur, homeAssistant_device_create());
            homeAssistant_mqtt_port_public(text_cur->entity_config_topic, text_cur->config_data, 0, 0);
            vPortFree(text_cur->config_data);
            //
            if (text_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(text_cur->availability_topic, text_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 更新实体状态
            if (text_cur->text_value != NULL)
            {
                homeAssistant_device_send_entity_state(ha_device->entity_text->entity_type, text_cur, 0);
            }
            text_cur = text_cur->next;
        }
    }

#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    ha_number_entity_t *number_cur = ha_device->entity_number->number_list->next;
    if (number_cur != ha_device->entity_number->number_list)
    {
        while (number_cur != ha_device->entity_number->number_list)
        {
            homeAssistant_create_number_data(number_cur, homeAssistant_device_create());
            homeAssistant_mqtt_port_public(number_cur->entity_config_topic, number_cur->config_data, 0, 0);
            vPortFree(number_cur->config_data);
            //
            if (number_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(number_cur->availability_topic, number_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 更新实体状态
            if (number_cur->number_value != NULL)
            {
                homeAssistant_device_send_entity_state(ha_device->entity_number->entity_type, number_cur, 0);
            }
            number_cur = number_cur->next;
        }
    }

#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    ha_climateHVAC_t *climateHVAC_cur = ha_device->entity_climateHVAC->climateHVAC_list->next;
    if (climateHVAC_cur != ha_device->entity_climateHVAC->climateHVAC_list)
    {
        while (climateHVAC_cur != ha_device->entity_climateHVAC->climateHVAC_list)
        {
            homeAssistant_create_climate_HVAC_data(climateHVAC_cur, homeAssistant_device_create());
            homeAssistant_mqtt_port_public(climateHVAC_cur->entity_config_topic, climateHVAC_cur->config_data, 0, 0);
            vPortFree(climateHVAC_cur->config_data);

            if (climateHVAC_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(climateHVAC_cur->availability_topic, climateHVAC_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 更新实体状态
            //  if (climateHVAC_cur.) {
            //      homeAssistant_device_send_entity_state(ha_device->entity_climateHVAC->entity_type, climateHVAC_cur, 0);

            // }
            climateHVAC_cur = climateHVAC_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_SELECT
    ha_select_t *select_cur = ha_device->entity_select->select_list->next;
    if (select_cur != ha_device->entity_select->select_list)
    {
        while (select_cur != ha_device->entity_select->select_list)
        {
            homeAssistant_create_select_data(select_cur, homeAssistant_device_create());
            homeAssistant_mqtt_port_public(select_cur->entity_config_topic, select_cur->config_data, 0, 0);
            vPortFree(select_cur->config_data);
            //
            // if (select_cur->availability_topic!=NULL)
            //     ret = homeAssistant_mqtt_port_public(select_cur->availability_topic, select_cur->payload_available, 0, 1);
            // else  ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            // 更新实体状态
            if (select_cur->options != NULL)
            {
                homeAssistant_device_send_entity_state(ha_device->entity_select->entity_type, select_cur, 0);
            }
            select_cur = select_cur->next;
        }
    }
#endif

#if CONFIG_ENTITY_ENABLE_BUTTON

    ha_btn_entity_t *button_cur = ha_device->entity_button->button_list->next;
    if (button_cur != ha_device->entity_button->button_list)
    {
        while (button_cur != ha_device->entity_button->button_list)
        {
            if (button_cur->entity_config_topic != NULL)
            {
                homeAssistant_create_button_data(button_cur, homeAssistant_device_create());
                homeAssistant_mqtt_port_public(button_cur->entity_config_topic, button_cur->config_data, 0, 0);
                vPortFree(button_cur->config_data);
            }
            // 实体上线
            if (button_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(button_cur->availability_topic, button_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            button_cur = button_cur->next;
        }
    }
#endif

#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
    ha_devTrig_entity_t *devTrig_cur = ha_device->entity_devTrig->devTrig_list->next;
    if (devTrig_cur != ha_device->entity_devTrig->devTrig_list)
    {
        while (devTrig_cur != ha_device->entity_devTrig->devTrig_list)
        {
            if (devTrig_cur->entity_config_topic != NULL)
            {
                homeAssistant_create_device_trigger_data(devTrig_cur, homeAssistant_device_create());
                homeAssistant_mqtt_port_public(devTrig_cur->entity_config_topic, devTrig_cur->config_data, 0, 0);
                vPortFree(devTrig_cur->config_data);
            }
            devTrig_cur = devTrig_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_SCENE
    ha_scene_entity_t *scene_cur = ha_device->entity_scene->scene_list->next;
    if (scene_cur != ha_device->entity_scene->scene_list)
    {
        while (scene_cur != ha_device->entity_scene->scene_list)
        {
            if (scene_cur->entity_config_topic != NULL)
            {
                homeAssistant_create_scene_data(scene_cur, homeAssistant_device_create());
                homeAssistant_mqtt_port_public(scene_cur->entity_config_topic, scene_cur->config_data, 0, 0);
                vPortFree(scene_cur->config_data);
            }
            // 实体上线
            if (scene_cur->availability_topic != NULL)
                ret = homeAssistant_mqtt_port_public(scene_cur->availability_topic, scene_cur->payload_available, 0, 1);
            else
                ret = homeAssistant_mqtt_port_public(ha_device->availability_topic, ha_device->payload_available, 0, 1);
            scene_cur = scene_cur->next;
        }
    }
#endif
    homeAssistant_device_send_status(HOMEASSISTANT_STATUS_ONLINE);
}

static void homeAssistant_mqtt_init(homeAssisatnt_device_t *ha_dev)
{
    if (ha_dev == NULL)
    {
        HA_LOG_E("client is NULL\r\n");
        return;
    }
    homeAssistant_mqtt_port_init(ha_dev);
}

void homeAssistant_device_init(homeAssisatnt_device_t *ha_dev, void (*event_cb)(ha_event_t, homeAssisatnt_device_t *))
{
    if (ha_dev == NULL)
    {
        HA_LOG_E("param is NULL");
        return;
    }
    static char *buff = NULL;

    ha_device = ha_dev;
    homeAssistant_get_sta_mac((char *)STA_MAC);
    if (ha_device->mqtt_info.mqtt_clientID == NULL)
    {
        ha_device->mqtt_info.mqtt_clientID = pvPortMalloc(strlen(CONFIG_HA_MQTT_CLIENT_DEF_ID) + 13);
        memset(ha_device->mqtt_info.mqtt_clientID, 0, strlen(CONFIG_HA_MQTT_CLIENT_DEF_ID) + 13);
        sprintf(ha_device->mqtt_info.mqtt_clientID, "%s-%02x%02x%02x%02x%02x%02x", CONFIG_HA_MQTT_CLIENT_DEF_ID, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
    }
    if (ha_device->mqtt_info.mqtt_host == NULL)
        ha_device->mqtt_info.mqtt_host = CONFIG_HA_MQTT_SERVER_HOST;
    if (ha_device->mqtt_info.port == 0)
        ha_device->mqtt_info.port = CONFIG_HA_MQTT_SERVER_PORT;
    if (ha_device->mqtt_info.mqtt_username == NULL)
        ha_device->mqtt_info.mqtt_username = CONFIG_HA_MQTT_CLIENT_DEF_USERNAME;
    if (ha_device->mqtt_info.mqtt_password == NULL)
        ha_device->mqtt_info.mqtt_password = CONFIG_HA_MQTT_CLIENT_DEF_PASSWORLD;
    if (ha_device->mqtt_info.mqtt_keeplive == 0)
        ha_device->mqtt_info.mqtt_keeplive = CONFIG_HA_MQTT_CLIENT_DEF_KEEPALIVE;

    if (ha_device->mqtt_info.will.will_msg == NULL)
    {
        ha_device->mqtt_info.will.will_msg = "offline";
        ha_device->mqtt_info.will.will_msg_len = strlen("offline");
    }
    if (ha_device->mqtt_info.will.will_topic == NULL)
    {
        buff = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + 12 + strlen("///status"));
        memset(buff, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + 12 + strlen("///status"));
        sprintf(buff, "%s/%02x%02x%02x%02x%02x%02x/status", CONFIG_HA_AUTOMATIC_DISCOVERY, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
        ha_device->mqtt_info.will.will_topic = buff;
        ha_device->mqtt_info.will.will_qos = 0;
        ha_device->mqtt_info.will.will_retain = true;
    }
    ha_device->mqtt_info.mqtt_connect_status = false;

    HA_LOG_I(".....................MQTT client connect info......................\r\n");
    HA_LOG_I("clientID:%s\r\n", ha_dev->mqtt_info.mqtt_clientID);
    HA_LOG_I("host:%s\r\n", ha_dev->mqtt_info.mqtt_host);
    HA_LOG_I("port:%d\r\n", ha_dev->mqtt_info.port);
    HA_LOG_I("username:%s\r\n", ha_dev->mqtt_info.mqtt_username);
    HA_LOG_I("password:%s\r\n", ha_dev->mqtt_info.mqtt_password);
    HA_LOG_I("will topic:%s\r\n", ha_device->mqtt_info.will.will_topic);
    HA_LOG_I("will msg:%s\r\n", ha_device->mqtt_info.will.will_msg);
    HA_LOG_I("...................................................................\r\n");

    if (ha_device->name == NULL)
        ha_device->name = CONFIG_HA_DEVICE_NAME;
    if (ha_device->hw_version == NULL)
        ha_device->hw_version = CONFIG_HA_DEVICE_HW_VERSION;
    if (ha_device->model == NULL)
        ha_device->model = CONFIG_HA_DEVICE_MODULE;
    if (ha_device->identifiers == NULL)
    {
        ha_device->identifiers = pvPortMalloc(strlen(ha_device->name) + 5);
        memset(ha_device->identifiers, 0, strlen(ha_device->name) + 5);
        sprintf(ha_device->identifiers, "%s_%02x%02x", ha_device->name, STA_MAC[4], STA_MAC[5]);
    }
    if (ha_device->sw_version == NULL)
        ha_device->sw_version = CONFIG_HA_DEVICE_SW_VERSION;
    if (ha_device->payload_available == NULL)
        ha_device->payload_available = "online";
    if (ha_device->payload_not_available == NULL)
        ha_device->payload_not_available = "offline";
    if (ha_device->manufacturer == NULL)
        ha_device->manufacturer = CONFIG_HA_DEVICE_MANUFACTURER;

    if (ha_device->availability_topic == NULL)
    {
        ha_device->availability_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + 12 + strlen("///status"));
        memset(ha_device->availability_topic, 0, strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + 12 + strlen("///status") + 1);
        sprintf(ha_device->availability_topic, "%s/%02x%02x%02x%02x%02x%02x/status", CONFIG_HA_AUTOMATIC_DISCOVERY, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
        // ha_device->availability_topic = ha_device->mqtt_info.will.will_topic;
    }
    // 初始化 binary sensor 实体
#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
    ha_device->entity_binary_sensor = pvPortMalloc(sizeof(ha_Bsensor_entity_t));
    ha_device->entity_binary_sensor->entity_type = CONFIG_HA_ENTITY_BINARY_SENSOR;
    ha_device->entity_binary_sensor->binary_sensor_list = pvPortMalloc(sizeof(ha_Bsensor_entity_t));
    ha_device->entity_binary_sensor->binary_sensor_list->prev = ha_device->entity_binary_sensor->binary_sensor_list;
    ha_device->entity_binary_sensor->binary_sensor_list->next = ha_device->entity_binary_sensor->binary_sensor_list;
#endif
    // 初始化Sensor 实体
#if CONFIG_ENTITY_ENABLE_SENSOR
    ha_device->entity_sensor = pvPortMalloc(sizeof(ha_sensor_entity_t));
    ha_device->entity_sensor->entity_type = CONFIG_HA_ENTITY_SENSOR;
    ha_device->entity_sensor->sensor_list = pvPortMalloc(sizeof(ha_sensor_entity_t));
    ha_device->entity_sensor->sensor_list->prev = ha_device->entity_sensor->sensor_list;
    ha_device->entity_sensor->sensor_list->next = ha_device->entity_sensor->sensor_list;
#endif
    // 初始化Light 实体
#if CONFIG_ENTITY_ENABLE_LIGHT
    ha_device->entity_light = pvPortMalloc(sizeof(ha_lhlist_t));
    ha_device->entity_light->entity_type = CONFIG_HA_ENTITY_LIGHT;
    ha_device->entity_light->light_list = pvPortMalloc(sizeof(ha_lh_entity_t));
    ha_device->entity_light->light_list->prev = ha_device->entity_light->light_list;
    ha_device->entity_light->light_list->next = ha_device->entity_light->light_list;
#endif
    // 初始化开关实体双向连接表
#if CONFIG_ENTITY_ENABLE_SWITCH
    ha_device->entity_switch = pvPortMalloc(sizeof(ha_swlist_t));
    ha_device->entity_switch->entity_type = CONFIG_HA_ENTITY_SWITCH;
    ha_device->entity_switch->switch_list = pvPortMalloc(sizeof(ha_sw_entity_t));
    ha_device->entity_switch->switch_list->prev = ha_device->entity_switch->switch_list;
    ha_device->entity_switch->switch_list->next = ha_device->entity_switch->switch_list;
#endif

#if CONFIG_ENTITY_ENABLE_TEXT
    ha_device->entity_text = pvPortMalloc(sizeof(ha_text_list_t));
    ha_device->entity_text->entity_type = CONFIG_HA_ENTITY_TEXT;
    ha_device->entity_text->text_list = pvPortMalloc(sizeof(ha_text_entity_t));
    ha_device->entity_text->text_list->prev = ha_device->entity_text->text_list;
    ha_device->entity_text->text_list->next = ha_device->entity_text->text_list;
#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    ha_device->entity_number = pvPortMalloc(sizeof(ha_number_list_t));
    ha_device->entity_number->entity_type = CONFIG_HA_ENTITY_NUMBER;
    ha_device->entity_number->number_list = pvPortMalloc(sizeof(ha_number_entity_t));
    ha_device->entity_number->number_list->prev = ha_device->entity_number->number_list;
    ha_device->entity_number->number_list->next = ha_device->entity_number->number_list;
#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    ha_device->entity_climateHVAC = pvPortMalloc(sizeof(ha_climateHVAC_list_t));
    ha_device->entity_climateHVAC->entity_type = CONFIG_HA_ENTITY_CLIMATE_HVAC;
    ha_device->entity_climateHVAC->climateHVAC_list = pvPortMalloc(sizeof(ha_climateHVAC_t));
    ha_device->entity_climateHVAC->climateHVAC_list->prev = ha_device->entity_climateHVAC->climateHVAC_list;
    ha_device->entity_climateHVAC->climateHVAC_list->next = ha_device->entity_climateHVAC->climateHVAC_list;
#endif
#if CONFIG_ENTITY_ENABLE_SELECT
    ha_device->entity_select = pvPortMalloc(sizeof(ha_select_list_t));
    ha_device->entity_select->entity_type = CONFIG_HA_ENTITY_SELECT;
    ha_device->entity_select->select_list = pvPortMalloc(sizeof(ha_select_t));
    ha_device->entity_select->select_list->prev = ha_device->entity_select->select_list;
    ha_device->entity_select->select_list->next = ha_device->entity_select->select_list;

#endif
#if CONFIG_ENTITY_ENABLE_BUTTON
    ha_device->entity_button = pvPortMalloc(sizeof(ha_btn_list_t));
    ha_device->entity_button->entity_type = CONFIG_HA_ENTITY_BUTTON;
    ha_device->entity_button->button_list = pvPortMalloc(sizeof(ha_btn_entity_t));
    ha_device->entity_button->button_list->prev = ha_device->entity_button->button_list;
    ha_device->entity_button->button_list->next = ha_device->entity_button->button_list;
#endif

#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
    ha_device->entity_devTrig = pvPortMalloc(sizeof(ha_devTrig_list_t));
    ha_device->entity_devTrig->entity_type = CONFIG_HA_ENTITY_DEVICE_TRIGGER;
    ha_device->entity_devTrig->devTrig_list = pvPortMalloc(sizeof(ha_devTrig_entity_t));
    ha_device->entity_devTrig->devTrig_list->prev = ha_device->entity_devTrig->devTrig_list;
    ha_device->entity_devTrig->devTrig_list->next = ha_device->entity_devTrig->devTrig_list;
#endif
#if CONFIG_ENTITY_ENABLE_SCENE
    ha_device->entity_scene = pvPortMalloc(sizeof(ha_scene_list_t));
    ha_device->entity_scene->entity_type = CONFIG_HA_ENTITY_SCENE;
    ha_device->entity_scene->scene_list = pvPortMalloc(sizeof(ha_scene_entity_t));
    ha_device->entity_scene->scene_list->prev = ha_device->entity_scene->scene_list;
    ha_device->entity_scene->scene_list->next = ha_device->entity_scene->scene_list;
#endif
    ha_device->event_cb = event_cb;
    homeAssistant_mqtt_init(ha_device);
    vPortFree(buff);
}

void homeAssistant_device_start(void)
{
    if (ha_device == NULL)
    {
        HA_LOG_E("param is NULL\r\n");
        return;
    }
    homeAssistant_mqtt_port_start();
}

void homeAssisatnt_device_stop(void)
{
    if (ha_device == NULL)
    {
        HA_LOG_E("param is NULL\r\n");
        return;
    }
    homeAssistant_device_send_status(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    homeAssistant_mqtt_port_stop();
}

void homeAssistant_device_send_status(bool status)
{
    if (ha_device->mqtt_info.mqtt_connect_status)
    {
        if (ha_device->availability_topic == NULL)
            goto __c_topic;
        else
            goto send_status;
    __c_topic:
        ha_device->availability_topic = pvPortMalloc(strlen(CONFIG_HA_AUTOMATIC_DISCOVERY) + 12 + strlen("//status"));
        memset(ha_device->availability_topic, 0, 128);
        sprintf(ha_device->availability_topic, "%s/%02x%02x%02x%02x%02x%02x/status", CONFIG_HA_AUTOMATIC_DISCOVERY, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
    send_status:
        if (ha_device->payload_available == NULL)
            ha_device->payload_available = "online";
        if (ha_device->payload_not_available == NULL)
            ha_device->payload_not_available = "offline";
        homeAssistant_mqtt_port_public(ha_device->availability_topic, status ? ha_device->payload_available : ha_device->payload_not_available, 0, 1);
    }
    else
    {
        HA_LOG_E("MQTT server is disconnect\r\n");
    }
}

void homeAssistant_device_add_entity(char *entity_type, void *ha_entity_list)
{
    // 添加开关实体
#if CONFIG_ENTITY_ENABLE_SWITCH
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH))
    {
        // 添加 switch 节点
        ha_sw_entity_t *switch_node = (ha_sw_entity_t *)ha_entity_list;
        entity_swith_add_node(switch_node);
    }
#endif
    // 添加light 实体
#if CONFIG_ENTITY_ENABLE_LIGHT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT))
    {
        HA_LOG_I("HomeAssistant add light entity\r\n");
        ha_lh_entity_t *light_node = (ha_lh_entity_t *)ha_entity_list;
        entity_light_add_node(light_node);
    }
#endif
    // 添加传感器
#if CONFIG_ENTITY_ENABLE_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR))
    {
        HA_LOG_I("HomeAssistant add sensor entity\r\n");
        ha_sensor_entity_t *sensor_node = (ha_sensor_entity_t *)ha_entity_list;
        entity_sensor_add_node(sensor_node);
    }
#endif
    // 添加二进制传感器实体
#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);
        ha_Bsensor_entity_t *binary_sensor_node = (ha_Bsensor_entity_t *)ha_entity_list;
        entity_binary_sensor_add_node(binary_sensor_node);
    }
#endif

#if CONFIG_ENTITY_ENABLE_TEXT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_TEXT))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);
        ha_text_entity_t *text_node = (ha_text_entity_t *)ha_entity_list;
        entity_text_add_node(text_node);
    }
#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_NUMBER))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);
        ha_number_entity_t *number_node = (ha_number_entity_t *)ha_entity_list;
        entity_number_add_node(number_node);
    }
#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_CLIMATE_HVAC))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);
        ha_climateHVAC_t *climateHVAC_node = (ha_climateHVAC_t *)ha_entity_list;
        entity_climate_HVAC_add_node(climateHVAC_node);
    }
#endif
#if CONFIG_ENTITY_ENABLE_SELECT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SELECT))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);
        ha_select_t *select_node = (ha_select_t *)ha_entity_list;
        entity_select_add_node(select_node);
    }
#endif

#if CONFIG_ENTITY_ENABLE_BUTTON
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BUTTON))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);
        ha_btn_entity_t *button_node = (ha_btn_entity_t *)ha_entity_list;
        entity_button_add_node(button_node);
    }
#endif

#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_DEVICE_TRIGGER))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);
        ha_devTrig_entity_t *devTrig_node = (ha_devTrig_entity_t *)ha_entity_list;
        entity_device_trigger_add_node(devTrig_node);
    }
#endif
#if CONFIG_ENTITY_ENABLE_SCENE
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SCENE))
    {
        HA_LOG_I("HomeAssistant add %s entity\r\n", entity_type);

        ha_scene_entity_t *scene_node = (ha_scene_entity_t *)ha_entity_list;
        entity_scene_add_node(scene_node);
    }
#endif
}

void homeAssistant_device_delete_entity(char *entity_type, char *unique_id)
{
    bool ret = 1;
#if CONFIG_ENTITY_ENABLE_SWITCH
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_switch_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_SWITCH);
    }
#endif

#if CONFIG_ENTITY_ENABLE_LIGHT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_light_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_LIGHT);
    }
#endif

#if CONFIG_ENTITY_ENABLE_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_sensor_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_SENSOR);
    }
#endif

#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_binary_sensor_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, entity_type);
    }
#endif

#if CONFIG_ENTITY_ENABLE_TEXT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_TEXT))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_text_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_TEXT);
    }
#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_NUMBER))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_number_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_NUMBER);
    }
#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_CLIMATE_HVAC))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_climate_HVAC_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_CLIMATE_HVAC);
    }
#endif
#if CONFIG_ENTITY_ENABLE_SELECT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SELECT))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_select_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_SELECT);
    }
#endif

#if CONFIG_ENTITY_ENABLE_BUTTON
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BUTTON))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_button_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_BUTTON);
    }
#endif

#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_DEVICE_TRIGGER))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_devTrig_delete_node(unique_id);
    }
#endif
#if CONFIG_ENTITY_ENABLE_SCENE
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SCENE))
    {
        HA_LOG_I("HomeAssistant delete %s entity\r\n", entity_type);
        ret = entity_scene_delete_node(unique_id);
        if (!ret)
            HA_LOG_E("delete \"%s\" %s error\r\n ", unique_id, CONFIG_HA_ENTITY_SCENE);
    }
#endif
}

int homeAssistant_device_send_entity_state(char *entity_type, void *ha_entity_list, unsigned short state)
{
    int ret_id = -1;
    if (entity_type == NULL || ha_entity_list == NULL)
    {
        HA_LOG_E("params error\r\n");
        return ret_id;
    }
#if CONFIG_ENTITY_ENABLE_SWITCH
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH))
    {
        ha_sw_entity_t *switch_node = (ha_sw_entity_t *)ha_entity_list;
        ret_id = homeAssistant_mqtt_port_public(switch_node->state_topic, state ? switch_node->payload_on : switch_node->payload_off, 0, 1);
    }
#endif
#if CONFIG_ENTITY_ENABLE_LIGHT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT))
    {
        ha_lh_entity_t *light_node = (ha_lh_entity_t *)ha_entity_list;

        if (light_node->payload_on != NULL && light_node->payload_off != NULL)
            ret_id = homeAssistant_mqtt_port_public(light_node->state_topic, state ? light_node->payload_on : light_node->payload_off, 0, 0);
        else
        {
            ret_id = homeAssistant_mqtt_port_public(light_node->state_topic, state ? "ON" : "OFF", 0, 0);
        }
        // 上报RGB值
        if (light_node->rgb.rgb_command_topic != NULL)
        {
            char *rgb_data = pvPortMalloc(16);
            if (light_node->rgb.rgb_value_template == NULL)
            {
                memset(rgb_data, 0, 16);
                sprintf(rgb_data, "%d,%d,%d", light_node->rgb.red, light_node->rgb.green, light_node->rgb.blue);
                ret_id = homeAssistant_mqtt_port_public(light_node->rgb.rgb_state_topic, rgb_data, 0, 0);
                if (ret_id < false)
                    HA_LOG_E("publish is fali\r\n");
            }
            vPortFree(rgb_data);
        }
        // 上报 HS 值
        if (light_node->hs.hs_command_topic != NULL)
        {
            char *hs_data = pvPortMalloc(16);
            if (light_node->hs.hs_value_template == NULL)
            {
                memset(hs_data, 0, 16);
                sprintf(hs_data, "%0.3f,%0.3f", light_node->hs.hue, light_node->hs.saturation);
                ret_id = homeAssistant_mqtt_port_public(light_node->hs.hs_state_topic, hs_data, 0, 0);
                if (ret_id < false)
                    HA_LOG_E("publish is fali\r\n");
            }
            vPortFree(hs_data);
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR))
    {
        ha_sensor_entity_t *sensor_node = (ha_sensor_entity_t *)ha_entity_list;
        if (sensor_node->sensor_data == NULL)
        {
            // HA_LOG_E("sensor_node sensor_data is NULL,data is state=%d", state);
            sensor_node->sensor_data = pvPortMalloc(16);
            memset(sensor_node->sensor_data, 0, 16);
            sprintf(sensor_node->sensor_data, "%d", state);
            ret_id = homeAssistant_mqtt_port_public(sensor_node->state_topic, sensor_node->sensor_data, 0, 1);
            memset(sensor_node->sensor_data, 0, 16);
            vPortFree(sensor_node->sensor_data);
            sensor_node->sensor_data = NULL;
        }
        else
            ret_id = homeAssistant_mqtt_port_public(sensor_node->state_topic, sensor_node->sensor_data, 0, 1);
    }
#endif
#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR))
    {
        ha_Bsensor_entity_t *binary_sensor_node = (ha_Bsensor_entity_t *)ha_entity_list;
        ret_id = homeAssistant_mqtt_port_public(binary_sensor_node->state_topic, state ? binary_sensor_node->payload_on : binary_sensor_node->payload_off, 0, 1);
    }
#endif

#if CONFIG_ENTITY_ENABLE_TEXT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_TEXT))
    {
        ha_text_entity_t *text_node = (ha_text_entity_t *)ha_entity_list;
        if (text_node->text_value != NULL && text_node->state_topic != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(text_node->state_topic, text_node->text_value, 0, 1);
        }
        else
            HA_LOG_E("text value is null\r\n");
    }
#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_NUMBER))
    {
        ha_number_entity_t *number_node = (ha_number_entity_t *)ha_entity_list;
        if (number_node->number)
            if (number_node->number_value == NULL)
                number_node->number_value = pvPortMalloc(16);
        memset(number_node->number_value, 0, 16);
        sprintf(number_node->number_value, number_node->step == 1 ? "%.0f" : "%f", number_node->number);
        if (number_node->number_value != NULL && number_node->state_topic != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(number_node->state_topic, number_node->number_value, 0, 1);
        }
        else
            HA_LOG_E("number value is null\r\n");
    }
#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_CLIMATE_HVAC))
    {
        ha_climateHVAC_t *climateHVAC_node = (ha_climateHVAC_t *)ha_entity_list;
        if (state)
        {
            // 返回模式
            if (climateHVAC_node->mode_state_topic != NULL && climateHVAC_node->modes[0] == NULL)
            {
                ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, modes_def[climateHVAC_node->modes_type], 0, 1);
            }
            else
            {
                ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, climateHVAC_node->modes[climateHVAC_node->modes_type], 0, 1);
            }
        }
        else
        {
            if (climateHVAC_node->mode_state_topic != NULL && climateHVAC_node->modes[0] == NULL)
            {
                ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, "off", 0, 1);
            }
            else
            {
                ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, "off", 0, 1);
            }
        }
    }
#endif

#if CONFIG_ENTITY_ENABLE_SELECT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SELECT))
    {
        ha_select_t *select_node = (ha_select_t *)ha_entity_list;
        if (select_node->options != NULL && select_node->state_topic != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(select_node->state_topic, select_node->options[select_node->option], 0, 1);
        }
        else
            HA_LOG_E("select value is null\r\n");
    }
#endif
#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_DEVICE_TRIGGER))
    {
        ha_devTrig_entity_t *devTrig_node = (ha_devTrig_entity_t *)ha_entity_list;
        if (devTrig_node->payload != NULL && devTrig_node->topic != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(devTrig_node->topic, devTrig_node->payload, 0, 1);
        }
        else
            HA_LOG_E("select value is null\r\n");
    }
#endif
#if CONFIG_ENTITY_ENABLE_BUTTON
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BUTTON))
    {
        ha_btn_entity_t *button_node = (ha_btn_entity_t *)ha_entity_list;
        if (button_node->command_topic != NULL && button_node->payload_press != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(button_node->command_topic, button_node->payload_press, 0, 1);
        }
        else
            HA_LOG_E("select value is null\r\n");
    }
#endif

    return ret_id;
}

void *homeAssistant_fine_entity(char *entity_type, const char *unique_id)
{
    if (entity_type == NULL || unique_id == NULL)
    {
        HA_LOG_E("parama is NULL\r\n");
        return NULL;
    }
    // 查找switch实体
#if CONFIG_ENTITY_ENABLE_SWITCH
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH))
    {
        ha_sw_entity_t *switch_cur = ha_device->entity_switch->switch_list->next;
        while (switch_cur != ha_device->entity_switch->switch_list)
        {
            if (!strncmp(switch_cur->unique_id, unique_id, strlen(unique_id)))
            {
                return switch_cur;
            }
            switch_cur = switch_cur->next;
        }
    }
#endif
    // 查找 light 实体
#if CONFIG_ENTITY_ENABLE_LIGHT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT))
    {
        ha_lh_entity_t *light_cur = ha_device->entity_light->light_list->next;
        while (light_cur != ha_device->entity_light->light_list)
        {
            if (!strncmp(light_cur->unique_id, unique_id, strlen(unique_id)))
            {
                return light_cur;
            }
            light_cur = light_cur->next;
        }
    }
#endif
    // 查找 sensor 实体
#if CONFIG_ENTITY_ENABLE_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR))
    {
        ha_sensor_entity_t *sensor_cur = ha_device->entity_sensor->sensor_list->next;
        while (sensor_cur != ha_device->entity_sensor->sensor_list)
        {
            if (!strncmp(sensor_cur->unique_id, unique_id, strlen(unique_id)))
            {
                return sensor_cur;
            }
            sensor_cur = sensor_cur->next;
        }
    }
#endif
    // 查找 binary sensor 实体
#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR))
    {
        ha_Bsensor_entity_t *binary_sensor_cur = ha_device->entity_binary_sensor->binary_sensor_list->next;
        while (binary_sensor_cur != ha_device->entity_binary_sensor->binary_sensor_list)
        {
            if (!strncmp(binary_sensor_cur->unique_id, unique_id, strlen(unique_id)))
            {
                return binary_sensor_cur;
            }
            binary_sensor_cur = binary_sensor_cur->next;
        }
    }
#endif

#if CONFIG_ENTITY_ENABLE_TEXT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_TEXT))
    {
        ha_text_entity_t *text_cur = ha_device->entity_text->text_list->next;
        while (text_cur != ha_device->entity_text->text_list)
        {
            if (!strncmp(text_cur->unique_id, unique_id, strlen(unique_id)))
                return text_cur;
            text_cur = text_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_NUMBER))
    {
        ha_number_entity_t *number_cur = ha_device->entity_number->number_list->next;
        while (number_cur != ha_device->entity_number->number_list)
        {
            if (!strncmp(number_cur->unique_id, unique_id, strlen(unique_id)))
                return number_cur;
            number_cur = number_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_CLIMATE_HVAC))
    {
        ha_climateHVAC_t *climateHVAC_cur = ha_device->entity_climateHVAC->climateHVAC_list->next;

        while (climateHVAC_cur != ha_device->entity_climateHVAC->climateHVAC_list)
        {
            if (!strncmp(climateHVAC_cur->unique_id, unique_id, strlen(unique_id)))
                return climateHVAC_cur;
            climateHVAC_cur = climateHVAC_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_SELECT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SELECT))
    {
        ha_select_t *select_cur = ha_device->entity_select->select_list->next;
        while (select_cur != ha_device->entity_select->select_list)
        {
            if (!strncmp(select_cur->unique_id, unique_id, strlen(unique_id)))
                return select_cur;
            select_cur = select_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_BUTTON
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BUTTON))
    {
        ha_btn_entity_t *button_cur = ha_device->entity_button->button_list->next;
        while (button_cur != ha_device->entity_button->button_list)
        {
            if (!strncmp(button_cur->unique_id, unique_id, strlen(unique_id)))
            {
                return button_cur;
            }
            button_cur = button_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_DEVICE_TRIGGER))
    {
        ha_devTrig_entity_t *devTrig_cur = ha_device->entity_devTrig->devTrig_list->next;

        while (devTrig_cur != ha_device->entity_devTrig->devTrig_list)
        {
            if (!strncmp(devTrig_cur->type, unique_id, strlen(unique_id)))
            {
                return devTrig_cur;
            }
            devTrig_cur = devTrig_cur->next;
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_SCENE
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SCENE))
    {
        ha_scene_entity_t *scene_cur = ha_device->entity_scene->scene_list->next;
        while (scene_cur != ha_device->entity_scene->scene_list)
        {
            if (!strncmp(scene_cur->unique_id, unique_id, strlen(unique_id)))
            {
                return scene_cur;
            }
            scene_cur = scene_cur->next;
        }
    }
#endif
    HA_LOG_E("There is no %s entity unique id %s\r\n", entity_type, unique_id);
    return NULL;
}

/**
 * @brief 根据entity_type和unique_id查找实体,并发送数据
 *
 * @param entity_type
 * @param unique_id
 * @param data
 * @return int
 */
int homeAssistant_device_quickly_send_data(char *entity_type, char *unique_id, char *data)
{
    int ret_id = -1;
    if (entity_type == NULL || unique_id == NULL)
    {
        HA_LOG_E("params error\r\n");
        return ret_id;
    }
#if CONFIG_ENTITY_ENABLE_SWITCH
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH))
    {
        ha_sw_entity_t *switch_node = (ha_sw_entity_t *)homeAssistant_fine_entity(entity_type, unique_id);
        if (data != NULL)
            ret_id = homeAssistant_mqtt_port_public(switch_node->state_topic, *data == '1' ? switch_node->payload_on : switch_node->payload_off, 0, 1);

        switch_node->switch_state = (*data == '1' ? 1 : 0);
    }
#endif
#if CONFIG_ENTITY_ENABLE_LIGHT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT))
    {
        ha_lh_entity_t *light_node = (ha_lh_entity_t *)homeAssistant_fine_entity(entity_type, unique_id);
        ;
        // 上报开关值
        if (strncmp(data, light_node->payload_on, strlen(light_node->payload_on)) == 0 || strncmp(data, light_node->payload_off, strlen(light_node->payload_off)) == 0)
        {
            ret_id = homeAssistant_mqtt_port_public(light_node->state_topic, data, 0, 1);
            light_node->light_state = (strncmp(data, light_node->payload_on, strlen(light_node->payload_on)) == 0 ? 1 : 0);
        }
        else
        {
            if (*data == '1' && strlen(data) == 1)
            {
                homeAssistant_mqtt_port_public(light_node->state_topic, light_node->payload_on, 0, 1);
                light_node->light_state = 1;
            }
            else if (*data == '0' && strlen(data) == 1)
            {
                homeAssistant_mqtt_port_public(light_node->state_topic, light_node->payload_off, 0, 1);
                light_node->light_state = 0;
            }
            else if (light_node->light_state)
            {
                if (data != NULL)
                {
                    ret_id = homeAssistant_mqtt_port_public(light_node->rgb.rgb_state_topic, data, 0, 1);
                    homeAssistant_get_light_rgb(light_node, data, strlen(data));
                }
            }
        }
    }
#endif
#if CONFIG_ENTITY_ENABLE_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR))
    {
        ha_sensor_entity_t *sensor_node = (ha_sensor_entity_t *)homeAssistant_fine_entity(entity_type, unique_id);

        if (sensor_node->sensor_data == NULL)
            sensor_node->sensor_data = pvPortMalloc(strlen(data));
        memset(sensor_node->sensor_data, 0, 16);
        if (data != NULL)
            sprintf(sensor_node->sensor_data, "%s", data);
        ret_id = homeAssistant_mqtt_port_public(sensor_node->state_topic, sensor_node->sensor_data, 0, 1);
        memset(sensor_node->sensor_data, 0, 16);
        vPortFree(sensor_node->sensor_data);
        sensor_node->sensor_data = NULL;
    }
#endif
#if CONFIG_ENTITY_ENABLE_BINARY_SENSOR
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR))
    {
        ha_Bsensor_entity_t *binary_sensor_node = (ha_Bsensor_entity_t *)homeAssistant_fine_entity(entity_type, unique_id);
        if (data != NULL && strlen(data) == 1)
            ret_id = homeAssistant_mqtt_port_public(binary_sensor_node->state_topic, *data == '1' ? binary_sensor_node->payload_on : binary_sensor_node->payload_off, 0, 1);
    }
#endif

#if CONFIG_ENTITY_ENABLE_TEXT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_TEXT))
    {
        ha_text_entity_t *text_node = (ha_text_entity_t *)homeAssistant_fine_entity(entity_type, unique_id);
        if (data != NULL && text_node->state_topic != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(text_node->state_topic, data, 0, 1);
            if (text_node->text_value == NULL)
                text_node->text_value = pvPortMalloc(strlen(data));

            memset(text_node->text_value, 0, strlen(data));
            strcpy(text_node->text_value, data);
        }
        else
            HA_LOG_E("text value is null\r\n");
    }
#endif
#if CONFIG_ENTITY_ENABLE_NUMBER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_NUMBER))
    {
        ha_number_entity_t *number_node = (ha_number_entity_t *)homeAssistant_fine_entity(entity_type, unique_id);

        if (number_node->number_value == NULL)
            number_node->number_value = pvPortMalloc(strlen(data));
        memset(number_node->number_value, 0, strlen(data));
        sprintf(number_node->number_value, "%s", data);
        if (number_node->number_value != NULL && number_node->state_topic != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(number_node->state_topic, number_node->number_value, 0, 1);
        }
        else
            HA_LOG_E("number value is null\r\n");
    }
#endif
#if CONFIG_ENTITY_ENABLE_CLIMATE_HVAC
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_CLIMATE_HVAC))
    {
        ha_climateHVAC_t *climateHVAC_node = (ha_climateHVAC_t *)homeAssistant_fine_entity(entity_type, unique_id);

        if (strcmp(data, "0FF") != 0 && strcmp(data, "ON") != 0)
        {
            // 返回模式
            ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, data, 0, 1);

            if (climateHVAC_node->mode_state_topic != NULL && climateHVAC_node->modes[0] == NULL)
            {
                for (size_t i = 0; i < 6; i++)
                {
                    if (strncmp(data, modes_def[i], strlen(modes_def[i])) == 0)
                    {
                        ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, modes_def[i], 0, 1);
                    }
                }
            }
            else
            {
                for (size_t i = 0; i < sizeof(climateHVAC_node->modes); i++)
                {
                    if (strncmp(data, climateHVAC_node->modes[i], strlen(climateHVAC_node->modes[i])) == 0)
                    {
                        ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, climateHVAC_node->modes[i], 0, 1);
                    }
                }
            }
        }
        else
        {

            // 发送温度值
            if (*data >= '0' && *data <= '9')
            {
                ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->temperature_state_topic, data, 0, 1);
            }
            else
            {
                if (climateHVAC_node->mode_state_topic != NULL)
                {
                    if (climateHVAC_node->mode_state_topic != NULL && climateHVAC_node->modes[0] == NULL)
                    {
                        for (size_t i = 0; i < 6; i++)
                        {
                            if (strncmp(data, modes_def[i], strlen(modes_def[i])) == 0)
                            {
                                ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, modes_def[i], 0, 1);
                            }
                        }
                    }
                    else
                    {

                        ret_id = homeAssistant_mqtt_port_public(climateHVAC_node->mode_state_topic, climateHVAC_node->modes[0], 0, 1);
                    }
                }
            }
        }
    }
#endif

#if CONFIG_ENTITY_ENABLE_SELECT
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SELECT))
    {
        ha_select_t *select_node = (ha_select_t *)homeAssistant_fine_entity(entity_type, unique_id);
        // 判断 DATA 是否是0~9
        if (*data >= '0' && *data <= '9')
        {
            select_node->option = atoi(data);
        }
        else
        {
            return -1;
        }

        if (select_node->options != NULL && select_node->state_topic != NULL)
        {

            ret_id = homeAssistant_mqtt_port_public(select_node->state_topic, select_node->options[select_node->option], 0, 1);
        }
        else
            HA_LOG_E("select value is null\r\n");
    }
#endif
#if CONFIG_ENTITY_ENABLE_DEVICE_TRIGGER
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_DEVICE_TRIGGER))
    {
        ha_devTrig_entity_t *devTrig_node = (ha_devTrig_entity_t *)homeAssistant_fine_entity(entity_type, unique_id);

        if (devTrig_node->payload != NULL && devTrig_node->topic != NULL)
        {
            ret_id = homeAssistant_mqtt_port_public(devTrig_node->topic, devTrig_node->payload, 0, 1);
        }
        else
            HA_LOG_E("select value is null\r\n");
    }
#endif
    return ret_id;
}
