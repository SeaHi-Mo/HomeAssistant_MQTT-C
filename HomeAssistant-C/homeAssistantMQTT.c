
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
#include "aiio_wifi.h"
#include "homeAssistantMQTT.h"
#include "log.h"
#include "cJSON.h"
#define DBG_TAG "homeAssistantMQTT"

static aiio_mqtt_client_config_t ha_mqtt_cfg;
static homeAssisatnt_device_t* ha_device;

static uint8_t STA_MAC[6] = { 0 };

static cJSON* homeAssistant_device_create(void)
{
    if (ha_device==NULL)
    {
        LOG_E("device is NULL");
        return;
    }
    cJSON* root = cJSON_CreateObject();
    if (ha_device->name!=NULL) cJSON_AddStringToObject(root, "name", ha_device->name);
    if (ha_device->hw_version!=NULL) cJSON_AddStringToObject(root, "hw_version", ha_device->hw_version);
    if (ha_device->identifiers!=NULL) {
        cJSON* identifiers = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "identifiers", identifiers);
        cJSON_AddItemToArray(identifiers, cJSON_CreateString(ha_device->identifiers));
    }
    if (ha_device->manufacturer!=NULL)cJSON_AddStringToObject(root, "manufacturer", ha_device->manufacturer);
    if (ha_device->model!=NULL)cJSON_AddStringToObject(root, "model", ha_device->model);
    if (ha_device->suggest_area!=NULL)cJSON_AddStringToObject(root, "suggest_area", ha_device->suggest_area);
    if (ha_device->sw_version!=NULL)cJSON_AddStringToObject(root, "sw_version", ha_device->sw_version);
    if (ha_device->via_device!=NULL)cJSON_AddStringToObject(root, "via_device", ha_device->via_device);

    cJSON* connencts = cJSON_CreateArray();
    cJSON* MAC = cJSON_CreateArray();
    cJSON_AddItemToArray(connencts, MAC);
    char* mac_str = pvPortMalloc(32);
    memset(mac_str, 0, 32);
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
static void homeAssistant_create_switch_data(ha_sw_entity_t* switch_entity, cJSON* device_json)
{
    if (switch_entity==NULL) {
        LOG_E("entity switch buff is NULL");
        return;
    }
    cJSON* root = cJSON_CreateObject();
    if (switch_entity->name!=NULL)cJSON_AddStringToObject(root, "name", switch_entity->name);
    if (switch_entity->device_class!=NULL)cJSON_AddStringToObject(root, "device_class", switch_entity->device_class);
    if (switch_entity->unique_id!=NULL)cJSON_AddStringToObject(root, "unique_id", switch_entity->unique_id);
    if (switch_entity->object_id!=NULL)cJSON_AddStringToObject(root, "object_id", switch_entity->object_id);

    if (switch_entity->command_topic==NULL) {
        switch_entity->command_topic = pvPortMalloc(128);
        memset(switch_entity->command_topic, 0, 128);
        sprintf(switch_entity->command_topic, "%s/%s/set", ha_device->name, switch_entity->unique_id);
    }
    cJSON_AddStringToObject(root, "command_topic", switch_entity->command_topic);
    if (switch_entity->state_topic==NULL) {
        switch_entity->state_topic = pvPortMalloc(128);
        memset(switch_entity->state_topic, 0, 128);
        sprintf(switch_entity->state_topic, "%s/%s/state", ha_device->name, switch_entity->unique_id);
    }
    cJSON_AddStringToObject(root, "state_topic", switch_entity->state_topic);
    if (switch_entity->payload_off==NULL)switch_entity->payload_off = "OFF";
    if (switch_entity->payload_on==NULL) switch_entity->payload_on = "ON";

    cJSON_AddStringToObject(root, "payload_off", switch_entity->payload_off);
    cJSON_AddStringToObject(root, "payload_on", switch_entity->payload_on);

    if (switch_entity->availability_topic!=NULL)cJSON_AddStringToObject(root, "availability_topic", switch_entity->availability_topic);
    else cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (switch_entity->payload_available!=NULL)cJSON_AddStringToObject(root, "payload_available", switch_entity->payload_available);
    else cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (switch_entity->payload_not_available!=NULL)cJSON_AddStringToObject(root, "payload_not_available", switch_entity->payload_not_available);
    else cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);

    if (device_json!=NULL)cJSON_AddItemToObject(root, "device", device_json);
    if (switch_entity->icon!=NULL) cJSON_AddStringToObject(root, "icon", switch_entity->icon);

    switch_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

/**
 * @brief entity_swith_add_node
 *         添加一个开关实体
 * @param switch_new_node
*/
static void entity_swith_add_node(ha_sw_entity_t* switch_new_node)
{
    assert(ha_device->entity_switch->switch_list);

    //添加新的节点
    ha_sw_entity_t* switch_list_handle = ha_device->entity_switch->switch_list->prev;
    //开始创建新的实体数据
    homeAssistant_create_switch_data(switch_new_node, homeAssistant_device_create());
    if (switch_new_node->entity_config_topic==NULL) {
        switch_new_node->entity_config_topic = pvPortMalloc(128);
        memset(switch_new_node->entity_config_topic, 0, 128);
        sprintf(switch_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SWITCH, switch_new_node->unique_id);
    }

    if (ha_device->mqtt_info.mqtt_connect_status) {
        aiio_mqtt_client_publish(ha_device->mqtt_client, switch_new_node->entity_config_topic, switch_new_node->config_data, strlen(switch_new_node->config_data), 1, 1);
        if (switch_new_node->command_topic!=NULL)aiio_mqtt_client_subscribe(ha_device->mqtt_client, switch_new_node->command_topic, 1);
    }
    else {
        LOG_E("MQTT server is diconnenct");
    }
    //插入节点
    switch_list_handle->next = switch_new_node;
    switch_new_node->prev = switch_list_handle;
    switch_new_node->next = ha_device->entity_switch->switch_list;
    ha_device->entity_switch->switch_list->prev = switch_new_node;

    vPortFree(switch_new_node->config_data);
}
/**
 * @brief 创建Light 的config 数据
 *
 * @param light_entity
 * @param device_json
*/
static void homeAssistant_create_light_data(ha_lh_entity_t* light_entity, cJSON* device_json)
{
    if (light_entity==NULL) {
        LOG_E("entity light buff is NULL");
        return;
    }
    cJSON* root = cJSON_CreateObject();
    if (light_entity->name!=NULL)cJSON_AddStringToObject(root, "name", light_entity->name);
    if (light_entity->device_class!=NULL)cJSON_AddStringToObject(root, "device_class", light_entity->device_class);
    if (light_entity->unique_id!=NULL)cJSON_AddStringToObject(root, "unique_id", light_entity->unique_id);
    if (light_entity->object_id!=NULL)cJSON_AddStringToObject(root, "object_id", light_entity->object_id);
    if (light_entity->icon!=NULL)cJSON_AddStringToObject(root, "icon", light_entity->icon);
    if (light_entity->availability_template!=NULL)cJSON_AddStringToObject(root, "availability_template", light_entity->availability_template);
    if (light_entity->availability_topic!=NULL)cJSON_AddStringToObject(root, "availability_topic", light_entity->availability_topic);
    else cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (light_entity->payload_available!=NULL)cJSON_AddStringToObject(root, "payload_available", light_entity->payload_available);
    else  cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (light_entity->payload_not_available!=NULL)cJSON_AddStringToObject(root, "payload_not_available", light_entity->payload_not_available);
    else cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);

    if (light_entity->command_topic==NULL)
    {
        light_entity->command_topic = pvPortMalloc(128);
        memset(light_entity->command_topic, 0, 128);
        sprintf(light_entity->command_topic, "%s/%s/set", ha_device->name, light_entity->unique_id);
    }
    if (light_entity->state_topic==NULL)
    {
        light_entity->state_topic = pvPortMalloc(128);
        memset(light_entity->state_topic, 0, 128);
        sprintf(light_entity->state_topic, "%s/%s/state", ha_device->name, light_entity->unique_id);
    }

    cJSON_AddStringToObject(root, "command_topic", light_entity->command_topic);
    cJSON_AddStringToObject(root, "state_topic", light_entity->state_topic);
    if (light_entity->payload_off!=NULL)cJSON_AddStringToObject(root, "payload_off", light_entity->payload_off);
    if (light_entity->payload_on!=NULL)cJSON_AddStringToObject(root, "payload_on", light_entity->payload_on);
    //brightness
    if (light_entity->brightness.brightness_command_template!=NULL)cJSON_AddStringToObject(root, "brightness_command_template", light_entity->brightness.brightness_command_template);
    if (light_entity->brightness.brightness_command_topic!=NULL) cJSON_AddStringToObject(root, "brightness_command_topic", light_entity->brightness.brightness_command_topic);
    if (light_entity->brightness.brightness_state_topic!=NULL)cJSON_AddStringToObject(root, "brightness_state_topic", light_entity->brightness.brightness_state_topic);
    if (light_entity->brightness.brightness_scale!=NULL)cJSON_AddStringToObject(root, "brightness_scale", light_entity->brightness.brightness_scale);
    if (light_entity->brightness.brightness_value_template!=NULL)cJSON_AddStringToObject(root, "brightness_value_template", light_entity->brightness.brightness_value_template);
    // rbgw
    if (light_entity->rgbw.rgbw_command_template!=NULL)cJSON_AddStringToObject(root, "rgbw_command_template", light_entity->rgbw.rgbw_command_template);
    if (light_entity->rgbw.rgbw_command_topic!=NULL) cJSON_AddStringToObject(root, "rgbw_command_topic", light_entity->rgbw.rgbw_command_topic);
    if (light_entity->rgbw.rgbw_state_topic!=NULL)  cJSON_AddStringToObject(root, "rgbw_state_topic", light_entity->rgbw.rgbw_state_topic);
    if (light_entity->rgbw.rgbw_value_template!=NULL)cJSON_AddStringToObject(root, "rgbw_value_template", light_entity->rgbw.rgbw_value_template);
    //rbg
    if (light_entity->rgb.rgb_command_template!=NULL)cJSON_AddStringToObject(root, "rgb_command_template", light_entity->rgb.rgb_command_template);
    if (light_entity->rgb.rgb_command_topic!=NULL) cJSON_AddStringToObject(root, "rgb_command_topic", light_entity->rgb.rgb_command_topic);
    if (light_entity->rgb.rgb_state_topic != NULL)cJSON_AddStringToObject(root, "rgb_state_topic", light_entity->rgb.rgb_state_topic);
    if (light_entity->rgb.rgb_value_template!=NULL)cJSON_AddStringToObject(root, "rgb_value_template", light_entity->rgb.rgb_value_template);
    //添加设备信息
    if (device_json!=NULL)cJSON_AddItemToObject(root, "device", device_json);

    light_entity->config_data = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);
}


static void  entity_light_add_node(ha_lh_entity_t* light_new_node)
{
    // assert(ha_device->entity_light->light_list);
    ha_lh_entity_t* light_list_handle = ha_device->entity_light->light_list->prev;
    //开始创建新的实体数据
    homeAssistant_create_light_data(light_new_node, homeAssistant_device_create());
    if (light_new_node->entity_config_topic==NULL) {
        light_new_node->entity_config_topic = pvPortMalloc(128);
        memset(light_new_node->entity_config_topic, 0, 128);
        sprintf(light_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_LIGHT, light_new_node->unique_id);
    }
    if (ha_device->mqtt_info.mqtt_connect_status) {
        aiio_mqtt_client_publish(ha_device->mqtt_client, light_new_node->entity_config_topic, light_new_node->config_data, strlen(light_new_node->config_data), 1, 1);
        if (light_new_node->command_topic!=NULL)aiio_mqtt_client_subscribe(ha_device->mqtt_client, light_new_node->command_topic, 1);
        if (light_new_node->rgbw.rgbw_command_topic!=NULL)aiio_mqtt_client_subscribe(ha_device->mqtt_client, light_new_node->rgbw.rgbw_command_topic, 1);
        if (light_new_node->brightness.brightness_command_topic!=NULL)aiio_mqtt_client_subscribe(ha_device->mqtt_client, light_new_node->brightness.brightness_command_topic, 1);
        if (light_new_node->rgb.rgb_command_topic!=NULL)aiio_mqtt_client_subscribe(ha_device->mqtt_client, light_new_node->rgb.rgb_command_topic, 1);
    }
    else {
        LOG_E("MQTT server is diconnenct");
    }
    //插入节点
    light_list_handle->next = light_new_node;
    light_new_node->prev = light_list_handle;
    light_new_node->next = ha_device->entity_light->light_list;
    ha_device->entity_light->light_list->prev = light_new_node;
    vPortFree(light_new_node->config_data);
}

static char* sensor_class_type[] = { "None","apparent_power","aqi","atmospheric_pressure","battery",\
"carbon_dioxide","carbon_monoxide","current","data_rate","data_size","date","distance","duration",\
"energy","energy_storage","enum","frequency","gas","humidity","illuminance","irradiance","moisture",\
"monetary","nitrogen_dioxide","nitrogen_monoxide","nitrous_oxide","ozone","ph","pm1","pm25","pm10",\
"power_factor","power","precipitation","precipitation_intensity","pressure","reactive_power","signal_strength",\
"sound_pressure","speed","sulphur_dioxide","temperature","timestamp","volatile_organic_compounds","volatile_organic_compounds_parts",\
"voltage","volume","volume_flow_rate","volume_storage","water","weight","wind_speed" };

static void homeAssistant_create_sensor_data(ha_sensor_entity_t* sensor_entity, cJSON* device_json)
{
    if (sensor_entity==NULL) {
        LOG_E("entity light buff is NULL");
        return;
    }

    cJSON* root = cJSON_CreateObject();
    if (sensor_entity->name!=NULL)cJSON_AddStringToObject(root, "name", sensor_entity->name);
    if (sensor_entity->unique_id!=NULL)cJSON_AddStringToObject(root, "unique_id", sensor_entity->unique_id);
    if (sensor_entity->object_id!=NULL)cJSON_AddStringToObject(root, "object_id", sensor_entity->object_id);
    if (sensor_entity->icon!=NULL)cJSON_AddStringToObject(root, "icon", sensor_entity->icon);
    if (sensor_entity->availability_template!=NULL)cJSON_AddStringToObject(root, "availability_template", sensor_entity->availability_template);
    if (sensor_entity->availability_topic!=NULL)cJSON_AddStringToObject(root, "availability_topic", sensor_entity->availability_topic);
    else cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (sensor_entity->payload_available!=NULL)cJSON_AddStringToObject(root, "payload_available", sensor_entity->payload_available);
    else  cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (sensor_entity->payload_not_available!=NULL)cJSON_AddStringToObject(root, "payload_not_available", sensor_entity->payload_not_available);
    else cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);
    if (sensor_entity->device_class) cJSON_AddStringToObject(root, "device_class", sensor_class_type[sensor_entity->device_class]);
    if (sensor_entity->suggested_display_precision)cJSON_AddNumberToObject(root, "suggested_display_precision", sensor_entity->suggested_display_precision);
    if (sensor_entity->json_attributes_template!=NULL) cJSON_AddStringToObject(root, "json_attributes_template", sensor_entity->json_attributes_template);
    if (sensor_entity->json_attributes_topic!=NULL)cJSON_AddStringToObject(root, "json_attributes_topic", sensor_entity->json_attributes_topic);
    if (sensor_entity->state_class!=NULL)cJSON_AddStringToObject(root, "state_class", sensor_entity->state_class);
    if (sensor_entity->state_topic==NULL) {
        sensor_entity->state_topic = pvPortMalloc(128);
        memset(sensor_entity->state_topic, 0, 128);
        sprintf(sensor_entity->state_topic, "%02x%02x%02x%02x%02x%02x/%s/state", STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], sensor_entity->unique_id);
    }cJSON_AddStringToObject(root, "state_topic", sensor_entity->state_topic);

    if (sensor_entity->unit_of_measurement!=NULL) cJSON_AddStringToObject(root, "unit_of_measurement", sensor_entity->unit_of_measurement);
    if (sensor_entity->value_template!=NULL)cJSON_AddStringToObject(root, "value_template", sensor_entity->value_template);
    if (sensor_entity->expire_after)cJSON_AddNumberToObject(root, "expire_after", sensor_entity->expire_after);
    if (sensor_entity->force_update)cJSON_AddTrueToObject(root, "force_update");
    else cJSON_AddFalseToObject(root, "force_update");
    //添加设备信息
    if (device_json!=NULL)cJSON_AddItemToObject(root, "device", device_json);
    sensor_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void  entity_sensor_add_node(ha_sensor_entity_t* sensor_new_node)
{
    if (sensor_new_node==NULL) return;
    ha_sensor_entity_t* sensor_list_handle = ha_device->entity_sensor->sensor_list->prev;

    homeAssistant_create_sensor_data(sensor_new_node, homeAssistant_device_create());
    if (sensor_new_node->entity_config_topic==NULL) {
        sensor_new_node->entity_config_topic = pvPortMalloc(128);
        memset(sensor_new_node->entity_config_topic, 0, 128);
        sprintf(sensor_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_SENSOR, sensor_new_node->unique_id);

    }
    if (ha_device->mqtt_info.mqtt_connect_status) {
        aiio_mqtt_client_publish(ha_device->mqtt_client, sensor_new_node->entity_config_topic, sensor_new_node->config_data, strlen(sensor_new_node->config_data), 1, 1);
        LOG_D("Topic:%s", sensor_new_node->entity_config_topic);
        LOG_D("config data:%s", sensor_new_node->config_data);
    }
    else {
        LOG_E("MQTT server is diconnenct");
    }

    sensor_list_handle->next = sensor_new_node;
    sensor_new_node->prev = sensor_list_handle;
    sensor_new_node->next = ha_device->entity_sensor->sensor_list;
    ha_device->entity_sensor->sensor_list->prev = sensor_new_node;
    vPortFree(sensor_new_node->config_data);
}
/**
 * @brief 二进制传感器
 *
*/
static char* Bsensor_class_type[] = { "None","battery","battery_charging","carbon_monoxide","cold","connectivity","door","garage_door",\
    "gas","heat","light","lock","moisture","motion","moving","occupancy","opening","plug","power", "presence", "problem", "running","safety", "smoke", "sound", "tamper", "update", "vibration", "window" };

static void homeAssistant_create_binary_sensor_data(ha_Bsensor_entity_t* binary_sensor_entity, cJSON* device_json)
{
    if (binary_sensor_entity==NULL) {
        LOG_E("entity light buff is NULL");
        return;
    }

    cJSON* root = cJSON_CreateObject();
    if (binary_sensor_entity->name!=NULL)cJSON_AddStringToObject(root, "name", binary_sensor_entity->name);
    if (binary_sensor_entity->unique_id!=NULL)cJSON_AddStringToObject(root, "unique_id", binary_sensor_entity->unique_id);
    if (binary_sensor_entity->object_id!=NULL)cJSON_AddStringToObject(root, "object_id", binary_sensor_entity->object_id);
    if (binary_sensor_entity->icon!=NULL)cJSON_AddStringToObject(root, "icon", binary_sensor_entity->icon);
    if (binary_sensor_entity->availability_template!=NULL)cJSON_AddStringToObject(root, "availability_template", binary_sensor_entity->availability_template);

    if (binary_sensor_entity->availability_topic!=NULL)cJSON_AddStringToObject(root, "availability_topic", binary_sensor_entity->availability_topic);
    else cJSON_AddStringToObject(root, "availability_topic", ha_device->availability_topic);
    if (binary_sensor_entity->payload_available!=NULL)cJSON_AddStringToObject(root, "payload_available", binary_sensor_entity->payload_available);
    else  cJSON_AddStringToObject(root, "payload_available", ha_device->payload_available);
    if (binary_sensor_entity->payload_not_available!=NULL)cJSON_AddStringToObject(root, "payload_not_available", binary_sensor_entity->payload_not_available);

    else cJSON_AddStringToObject(root, "payload_not_available", ha_device->payload_not_available);
    if (binary_sensor_entity->device_class) cJSON_AddStringToObject(root, "device_class", Bsensor_class_type[binary_sensor_entity->device_class]);
    if (binary_sensor_entity->json_attributes_template!=NULL) cJSON_AddStringToObject(root, "json_attributes_template", binary_sensor_entity->json_attributes_template);
    if (binary_sensor_entity->json_attributes_topic!=NULL)cJSON_AddStringToObject(root, "json_attributes_topic", binary_sensor_entity->json_attributes_topic);
    if (binary_sensor_entity->state_class!=NULL)cJSON_AddStringToObject(root, "state_class", binary_sensor_entity->state_class);

    if (binary_sensor_entity->entity_category!=NULL)cJSON_AddStringToObject(root, "entity_category", binary_sensor_entity->entity_category);
    if (binary_sensor_entity->state_topic==NULL) {
        binary_sensor_entity->state_topic = pvPortMalloc(128);
        memset(binary_sensor_entity->state_topic, 0, 128);
        sprintf(binary_sensor_entity->state_topic, "%02x%02x%02x%02x%02x%02x/%s/state", STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5], binary_sensor_entity->unique_id);
    }
    cJSON_AddStringToObject(root, "state_topic", binary_sensor_entity->state_topic);
    if (binary_sensor_entity->payload_on==NULL) binary_sensor_entity->payload_on = "ON";
    cJSON_AddStringToObject(root, "payload_on", binary_sensor_entity->payload_on);
    if (binary_sensor_entity->payload_off==NULL) binary_sensor_entity->payload_off = "OFF";
    cJSON_AddStringToObject(root, "payload_off", binary_sensor_entity->payload_off);

    if (binary_sensor_entity->value_template!=NULL)cJSON_AddStringToObject(root, "value_template", binary_sensor_entity->value_template);
    if (binary_sensor_entity->expire_after)cJSON_AddNumberToObject(root, "expire_after", binary_sensor_entity->expire_after);
    if (binary_sensor_entity->force_update)cJSON_AddTrueToObject(root, "force_update");
    else cJSON_AddFalseToObject(root, "force_update");
    //添加设备信息
    if (device_json!=NULL)cJSON_AddItemToObject(root, "device", device_json);
    binary_sensor_entity->config_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

static void  entity_binary_sensor_add_node(ha_Bsensor_entity_t* binary_sensor_new_node)
{
    if (binary_sensor_new_node==NULL) return;
    ha_Bsensor_entity_t* binary_sensor_list_handle = ha_device->entity_binary_sensor->binary_sensor_list->prev;

    homeAssistant_create_binary_sensor_data(binary_sensor_new_node, homeAssistant_device_create());
    if (binary_sensor_new_node->entity_config_topic==NULL) {
        binary_sensor_new_node->entity_config_topic = pvPortMalloc(128);
        memset(binary_sensor_new_node->entity_config_topic, 0, 128);
        sprintf(binary_sensor_new_node->entity_config_topic, "%s/%s/%s/config", CONFIG_HA_AUTOMATIC_DISCOVERY, CONFIG_HA_ENTITY_BINARY_SENSOR, binary_sensor_new_node->unique_id);

    }
    if (ha_device->mqtt_info.mqtt_connect_status) {
        aiio_mqtt_client_publish(ha_device->mqtt_client, binary_sensor_new_node->entity_config_topic, binary_sensor_new_node->config_data, strlen(binary_sensor_new_node->config_data), 0, 1);
        LOG_D("Topic:%s", binary_sensor_new_node->entity_config_topic);
        LOG_D("config data:%s", binary_sensor_new_node->config_data);
    }
    else {
        LOG_E("MQTT server is diconnenct");
    }

    binary_sensor_list_handle->next = binary_sensor_new_node;
    binary_sensor_new_node->prev = binary_sensor_list_handle;
    binary_sensor_new_node->next = ha_device->entity_binary_sensor->binary_sensor_list;
    ha_device->entity_binary_sensor->binary_sensor_list->prev = binary_sensor_new_node;
    vPortFree(binary_sensor_new_node->config_data);
}



static void log_error_if_nonzero(const char* message, int error_code)
{
    if (error_code != 0) {
        LOG_E("Last error %s: 0x%x", message, error_code);
    }
}
/**
 * @brief homeAssistant_get_light_rgb
 *        获取RGB 值，适用于没有使用格式要求的情况，简单解析出RGB三基色值
 * @param light_entity
 * @param rgb_data
*/
static void homeAssistant_get_light_rgb(ha_lh_entity_t* light_entity, const char* rgb_data, uint16_t data_len)
{
    if (light_entity==NULL || rgb_data==NULL) {
        LOG_E("entity light buff is NULL");
        return;
    }
    char* data_str = pvPortMalloc(data_len);
    memset(data_str, 0, data_len);
    sprintf(data_str, "%.*s", data_len, rgb_data);
    LOG_F("rgb_data %s", data_str);
    if (light_entity->rgb.rgb_command_template==NULL) {
        char* rgb = strtok(data_str, ",");
        if (rgb)light_entity->rgb.red = atoi(rgb);
        rgb = strtok(NULL, ",");
        if (rgb) light_entity->rgb.green = atoi(rgb);
        rgb = strtok(NULL, ",");
        if (rgb) light_entity->rgb.blue = atoi(rgb);

    }
    vPortFree(data_str);
}
/**
 * @brief homeAssistant_get_command
 *      获取 服务器指令事件，并把相应实体指针指向对应的实体
 * @param topic
 * @param topic_len
 * @param data
 * @param data_len
 * @return ha_event_t
*/
static ha_event_t homeAssistant_get_command(const char* topic, uint16_t topic_len, const char* data, uint16_t data_len)
{
    if (topic==NULL ||data==NULL) {
        LOG_E("params is NULL");
        return HA_EVENT_NONE;
    }
    ha_event_t event;
    //识别homeassistant 平台的出生消息
    if (!strncmp(topic, CONFIG_HA_STATUS_TOPIC, topic_len)) {
        if (!strncmp(data, CONFIG_HA_STATUS_MESSAGE_ON, data_len)) {
            event = HA_EVENT_HOMEASSISTANT_STATUS_ONLINE;
            ha_device->homeassistant_online = true;
        }
        else {
            event = HA_EVENT_HOMEASSISTANT_STATUS_OFFLINE;
            ha_device->homeassistant_online = false;
        }
        return event;
    }
    //查找出switch 实体
    ha_sw_entity_t* switch_cur = ha_device->entity_switch->switch_list->next;

    while (switch_cur!=ha_device->entity_switch->switch_list) {
        if (!strncmp(topic, switch_cur->command_topic, topic_len)) {
            if (switch_cur->payload_on!=NULL)
                switch_cur->switch_state = strncmp(data, switch_cur->payload_on, data_len)?false:true;
            else
                switch_cur->switch_state = strncmp(data, "ON", data_len)?false:true;
            event = HA_EVENT_MQTT_COMMAND_SWITCH;
            //找出该实体之后，方便后面操作实体
            ha_device->entity_switch->command_switch = switch_cur;
            return event;
        }
        //指针指向下一个节点
        if (switch_cur->next==ha_device->entity_switch->switch_list) break;
        else
            switch_cur = switch_cur->next;
    }
    //查找相应的Light 实体
    ha_lh_entity_t* light_cur = ha_device->entity_light->light_list->next;
    while (light_cur!=ha_device->entity_light->light_list)
    {
        if (!strncmp(topic, light_cur->rgb.rgb_command_topic, topic_len)) {
            homeAssistant_get_light_rgb(light_cur, data, data_len);
            event = HA_EVENT_MQTT_COMMAND_LIGHT_RGB_UPDATE;
            light_cur->light_state = true;
            ha_device->entity_light->command_light = light_cur;
            return event;
        }
        else  if (!strncmp(topic, light_cur->command_topic, topic_len)) {
            if (light_cur->payload_on!=NULL)light_cur->light_state = strncmp(data, light_cur->payload_on, data_len)?false:true;
            else light_cur->light_state = strncmp(data, "ON", data_len)?false:true;
            event = HA_EVENT_MQTT_COMMAND_LIGHT_SWITCH;
            ha_device->entity_light->command_light = light_cur;
            return event;
        }

        if (light_cur->next==ha_device->entity_light->light_list)break;
        else
            light_cur = light_cur->next;
    }

    return event;
}
//单独更新所有实体的配置，当homeAssistant重启时更新实体上线
static void update_all_entity_to_homeassistant(void)
{
    //更新所有switch 实体
    int ret = 0;
    {
        ha_sw_entity_t* switch_cur = ha_device->entity_switch->switch_list->next;
        while (switch_cur!=ha_device->entity_switch->switch_list) {
            if (switch_cur->entity_config_topic!=NULL) {
                homeAssistant_create_switch_data(switch_cur, homeAssistant_device_create());
                ret = aiio_mqtt_client_publish(ha_device->mqtt_client, switch_cur->entity_config_topic, switch_cur->config_data, strlen(switch_cur->config_data), 0, 0);
                if (ret==0)
                    vPortFree(switch_cur->config_data);
            }
            //实体上线
            if (switch_cur->availability_topic!=NULL)
                ret = aiio_mqtt_client_publish(ha_device->mqtt_client, switch_cur->availability_topic, switch_cur->payload_available, strlen(switch_cur->payload_available), 0, 0);
            else  ret = aiio_mqtt_client_publish(ha_device->mqtt_client, ha_device->availability_topic, ha_device->payload_available, strlen(ha_device->payload_available), 0, 0);
            //实体上报当前状态
            if (ha_device->entity_switch->command_switch!=NULL)
                homeAssistan_device_send_entity_state(CONFIG_HA_ENTITY_SWITCH, ha_device->entity_switch->command_switch, ha_device->entity_switch->command_switch->switch_state);
            else
                homeAssistan_device_send_entity_state(CONFIG_HA_ENTITY_SWITCH, switch_cur, switch_cur->switch_state);

            switch_cur = switch_cur->next;
        }
    }
    //更新所有light 实体
    {
        ha_lh_entity_t* light_cur = ha_device->entity_light->light_list->next;
        while (light_cur!=ha_device->entity_light->light_list) {
            homeAssistant_create_light_data(light_cur, homeAssistant_device_create());
            ret = aiio_mqtt_client_publish(ha_device->mqtt_client, light_cur->entity_config_topic, light_cur->config_data, strlen(light_cur->config_data), 0, 0);
            if (ret==0)
                vPortFree(light_cur->config_data);
            //实体上线
            if (light_cur->availability_topic!=NULL)
                ret = aiio_mqtt_client_publish(ha_device->mqtt_client, light_cur->availability_topic, light_cur->payload_available, strlen(light_cur->payload_available), 0, 0);
            else  ret = aiio_mqtt_client_publish(ha_device->mqtt_client, ha_device->availability_topic, ha_device->payload_available, strlen(ha_device->payload_available), 0, 0);
            //实体上报当前状态
            if (ha_device->entity_light->command_light!=NULL)
                homeAssistan_device_send_entity_state(CONFIG_HA_ENTITY_LIGHT, ha_device->entity_light->command_light, ha_device->entity_light->command_light->light_state);
            else
                homeAssistan_device_send_entity_state(CONFIG_HA_ENTITY_LIGHT, light_cur, light_cur->light_state);
            light_cur = light_cur->next;
        }
    }
    //更新所有sensor 实体
    {
        ha_sensor_entity_t* sensor_cur = ha_device->entity_sensor->sensor_list->next;
        while (sensor_cur!=ha_device->entity_sensor->sensor_list) {
            if (sensor_cur->entity_config_topic!=NULL) {
                homeAssistant_create_light_data(sensor_cur, homeAssistant_device_create());
                ret = aiio_mqtt_client_publish(ha_device->mqtt_client, sensor_cur->entity_config_topic, sensor_cur->config_data, strlen(sensor_cur->config_data), 0, 0);
                if (ret==0)
                    vPortFree(sensor_cur->config_data);
            }
            //实体上线
            if (sensor_cur->availability_topic!=NULL)
                ret = aiio_mqtt_client_publish(ha_device->mqtt_client, sensor_cur->availability_topic, sensor_cur->payload_available, strlen(sensor_cur->payload_available), 0, 0);
            else  ret = aiio_mqtt_client_publish(ha_device->mqtt_client, ha_device->availability_topic, ha_device->payload_available, strlen(ha_device->payload_available), 0, 0);
            //更新实体状态
            homeAssistan_device_send_entity_state(ha_device->entity_sensor->entity_type, sensor_cur, 0);
            sensor_cur = sensor_cur->next;
        }
    }
    //更新所有binary_sensor 实体
    {
        ha_Bsensor_entity_t* binary_sensor_cur = ha_device->entity_binary_sensor->binary_sensor_list->next;
        while (binary_sensor_cur!= ha_device->entity_binary_sensor->binary_sensor_list) {

            homeAssistant_create_light_data(binary_sensor_cur, homeAssistant_device_create());
            ret = aiio_mqtt_client_publish(ha_device->mqtt_client, binary_sensor_cur->entity_config_topic, binary_sensor_cur->config_data, strlen(binary_sensor_cur->config_data), 0, 0);
            if (ret==0)
                vPortFree(binary_sensor_cur->config_data);
            //实体上线
            if (binary_sensor_cur->availability_topic!=NULL)
                ret = aiio_mqtt_client_publish(ha_device->mqtt_client, binary_sensor_cur->availability_topic, binary_sensor_cur->payload_available, strlen(binary_sensor_cur->payload_available), 0, 0);
            else  ret = aiio_mqtt_client_publish(ha_device->mqtt_client, ha_device->availability_topic, ha_device->payload_available, strlen(ha_device->payload_available), 0, 0);
            //更新实体状态
            homeAssistan_device_send_entity_state(ha_device->entity_binary_sensor->entity_type, binary_sensor_cur, binary_sensor_cur->state);
            binary_sensor_cur = binary_sensor_cur->next;
        }
    }

}

static aiio_err_t mqtt_event_cb(aiio_mqtt_event_handle_t event)
{
    int32_t event_id;
    aiio_mqtt_client_handle_t client = event->client;
    event_id = event->event_id;
    LOG_D("Event dispatched, event_id=%d", event_id);
    int msg_id;
    switch ((aiio_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ha_device->mqtt_info.mqtt_connect_status = true;
            ha_device->homeassistant_online = true;
            ha_device->event_cb(HA_EVENT_MQTT_CONNECED, ha_device);
            aiio_mqtt_client_subscribe(client, CONFIG_HA_STATUS_TOPIC, 0);

            LOG_D("HomeAssistant free heap size=%dbyte", aiio_os_get_free_heap_size());
            break;
        case MQTT_EVENT_DISCONNECTED:
            ha_device->mqtt_info.mqtt_connect_status = false;
            ha_device->event_cb(HA_EVENT_MQTT_DISCONNECT, ha_device);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            LOG_D("MQTT_EVENT_SUBSCRIBED");
            LOG_D("HomeAssistant free heap size=%dbyte", aiio_os_get_free_heap_size());
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            LOG_D("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            LOG_D("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            LOG_D("MQTT_EVENT_DATA");
            LOG_D("TOPIC=%.*s", event->topic_len, event->topic);
            LOG_D("DATA=%.*s", event->data_len, event->data);
            ha_event_t ha_event = homeAssistant_get_command(event->topic, event->topic_len, event->data, event->data_len);
            if (ha_event==HA_EVENT_HOMEASSISTANT_STATUS_ONLINE) {
                LOG_I("HomeAssistant is online,send device status");
                update_all_entity_to_homeassistant();
                homeAssistant_device_send_status(HOMEASSISTANT_STATUS_ONLINE);
            }

            if (ha_device->homeassistant_online)
                ha_device->event_cb(ha_event, ha_device);

            else
                LOG_E("HomeAssistant is offline");

            LOG_D("HomeAssistant free heap size=%dbyte", aiio_os_get_free_heap_size());
            break;
        case MQTT_EVENT_ERROR:
            // LOG_D("MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->aiio_tls_last_aiio_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->aiio_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno", event->error_handle->aiio_transport_sock_errno);
                LOG_D("Last errno string (%s)", strerror(event->error_handle->aiio_transport_sock_errno));
            }
            ha_device->event_cb(HA_EVENT_MQTT_ERROR, ha_device);
            break;
        default:
            LOG_D("Other event id:%d", event->event_id);
            break;
    }
    event->event_id = MQTT_EVENT_ANY;
    return AIIO_OK;
}

static void homeAssistant_mqtt_init(aiio_mqtt_client_handle_t* client)
{
    if (client==NULL) {
        LOG_E("client is NULL");
        return;
    }
    aiio_mqtt_client_config_t mqtt_cfg = {
        .host = ha_device->mqtt_info.mqtt_host,
        .port = ha_device->mqtt_info.port,
        .username = ha_device->mqtt_info.mqtt_username,
        .password = ha_device->mqtt_info.mqtt_password,
        .keepalive = ha_device->mqtt_info.mqtt_keeplive,
        .client_id = ha_device->mqtt_info.mqtt_clientID,
        .lwt_qos = ha_device->mqtt_info.will.will_qos,
        .lwt_retain = ha_device->mqtt_info.will.will_retain,
        .lwt_msg = ha_device->mqtt_info.will.will_msg,
        .lwt_msg_len = ha_device->mqtt_info.will.will_msg_len,
        .lwt_topic = ha_device->mqtt_info.will.will_topic,
        .event_handle = mqtt_event_cb,
    };
    memset(client, 0, sizeof(aiio_mqtt_client_handle_t));
    *client = aiio_mqtt_client_init(&mqtt_cfg);
    if (*client!= NULL) LOG_D("MQTT client init suceess!");
    else LOG_E("MQTT client init fail");
}

void homeAssistant_device_init(homeAssisatnt_device_t* ha_dev, void(*event_cb)(ha_event_t, homeAssisatnt_device_t*))
{
    if (ha_dev==NULL) {
        LOG_E("param is NULL");
        return;
    }
    static uint8_t* buff = NULL;

    ha_device = ha_dev;
    aiio_wifi_sta_mac_get(STA_MAC);
    if (ha_device->mqtt_info.mqtt_clientID==NULL)ha_device->mqtt_info.mqtt_clientID = CONFIG_HA_MQTT_CLIENT_DEF_ID;
    if (ha_device->mqtt_info.mqtt_host==NULL)ha_device->mqtt_info.mqtt_host = CONFIG_HA_MQTT_SERVER_HOST;
    if (ha_device->mqtt_info.port==0)ha_device->mqtt_info.port = CONFIG_HA_MQTT_SERVER_PORT;
    if (ha_device->mqtt_info.mqtt_username==NULL)ha_device->mqtt_info.mqtt_username = CONFIG_HA_MQTT_CLIENT_DEF_USERNAME;
    if (ha_device->mqtt_info.mqtt_password==NULL)ha_device->mqtt_info.mqtt_password = CONFIG_HA_MQTT_CLIENT_DEF_PASSWORLD;
    if (ha_device->mqtt_info.mqtt_keeplive==0)ha_device->mqtt_info.mqtt_keeplive = CONFIG_HA_MQTT_CLIENT_DEF_KEEPALIVE;

    if (ha_device->mqtt_info.will.will_msg==NULL) {
        ha_device->mqtt_info.will.will_msg = "offline";
        ha_device->mqtt_info.will.will_msg_len = strlen("offline");
    }
    if (ha_device->mqtt_info.will.will_topic==NULL) {
        buff = pvPortMalloc(128);
        memset(buff, 0, 128);
        sprintf(buff, "%s/%02x%02x%02x%02x%02x%02x/status", CONFIG_HA_AUTOMATIC_DISCOVERY, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
        ha_device->mqtt_info.will.will_topic = buff;
        ha_device->mqtt_info.will.will_qos = 0;
        ha_device->mqtt_info.will.will_retain = true;
    }
    ha_device->mqtt_info.mqtt_connect_status = false;

    LOG_I(".....................MQTT client connect info......................");
    LOG_I("clientID:%s", ha_dev->mqtt_info.mqtt_clientID);
    LOG_I("host:%s", ha_dev->mqtt_info.mqtt_host);
    LOG_I("port:%d", ha_dev->mqtt_info.port);
    LOG_I("username:%s", ha_dev->mqtt_info.mqtt_username);
    LOG_I("password:%s", ha_dev->mqtt_info.mqtt_password);
    LOG_I("will topic:%s", ha_device->mqtt_info.will.will_topic);
    LOG_I("will msg:%s", ha_device->mqtt_info.will.will_msg);
    LOG_I("...................................................................");
    homeAssistant_mqtt_init(&ha_device->mqtt_client);

    if (ha_device->name==NULL)ha_device->name = CONFIG_HA_DEVICE_NAME;
    if (ha_device->hw_version==NULL)ha_device->hw_version = CONFIG_HA_DEVICE_HW_VERSION;
    if (ha_device->identifiers==NULL)ha_device->identifiers = ha_device->hw_version;
    if (ha_device->sw_version==NULL)ha_device->sw_version = CONFIG_HA_DEVICE_SW_VERSION;
    if (ha_device->payload_available==NULL)ha_device->payload_available = "online";
    if (ha_device->payload_not_available==NULL)ha_device->payload_not_available = "offline";
    if (ha_device->manufacturer==NULL)ha_device->manufacturer = CONFIG_HA_DEVICE_MANUFACTURER;
    if (ha_device->model==NULL)ha_device->model = "Ai-M61";
    if (ha_device->availability_topic==NULL) {
        ha_device->availability_topic = pvPortMalloc(128);
        memset(ha_device->availability_topic, 0, 128);
        sprintf(ha_device->availability_topic, "%s/%02x%02x%02x%02x%02x%02x/status", CONFIG_HA_AUTOMATIC_DISCOVERY, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
        // ha_device->availability_topic = ha_device->mqtt_info.will.will_topic;
    }
    //初始化 binary sensor 实体
    ha_device->entity_binary_sensor = pvPortMalloc(sizeof(ha_Bsensor_entity_t));
    ha_device->entity_binary_sensor->entity_type = CONFIG_HA_ENTITY_BINARY_SENSOR;
    ha_device->entity_binary_sensor->binary_sensor_list = pvPortMalloc(sizeof(ha_Bsensor_entity_t));
    ha_device->entity_binary_sensor->binary_sensor_list->prev = ha_device->entity_binary_sensor->binary_sensor_list;
    ha_device->entity_binary_sensor->binary_sensor_list->next = ha_device->entity_binary_sensor->binary_sensor_list;
    //初始化Sensor 实体
    ha_device->entity_sensor = pvPortMalloc(sizeof(ha_sensor_entity_t));
    ha_device->entity_sensor->entity_type = CONFIG_HA_ENTITY_SENSOR;
    ha_device->entity_sensor->sensor_list = pvPortMalloc(sizeof(ha_sensor_entity_t));
    ha_device->entity_sensor->sensor_list->prev = ha_device->entity_sensor->sensor_list;
    ha_device->entity_sensor->sensor_list->next = ha_device->entity_sensor->sensor_list;
    //初始化Light 实体
    ha_device->entity_light = pvPortMalloc(sizeof(ha_lhlist_t));
    ha_device->entity_light->entity_type = CONFIG_HA_ENTITY_LIGHT;
    ha_device->entity_light->light_list = pvPortMalloc(sizeof(ha_lh_entity_t));
    ha_device->entity_light->light_list->prev = ha_device->entity_light->light_list;
    ha_device->entity_light->light_list->next = ha_device->entity_light->light_list;
    //初始化开关实体双向连接表
    ha_device->entity_switch = pvPortMalloc(sizeof(ha_swlist_t));
    ha_device->entity_switch->entity_type = CONFIG_HA_ENTITY_SWITCH;
    ha_device->entity_switch->switch_list = pvPortMalloc(sizeof(ha_sw_entity_t));
    ha_device->entity_switch->switch_list->prev = ha_device->entity_switch->switch_list;
    ha_device->entity_switch->switch_list->next = ha_device->entity_switch->switch_list;

    ha_device->event_cb = event_cb;
    vPortFree(buff);
}

void homeAssistant_device_start(void)
{
    if (ha_device==NULL) {
        LOG_E("param is NULL");
        return;
    }
    aiio_mqtt_client_start(ha_device->mqtt_client);
}

void homeAssisatnt_device_stop(void)
{
    if (ha_device==NULL) {
        LOG_E("param is NULL");
        return;
    }
    homeAssistant_device_send_status(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    aiio_mqtt_client_disconnect(ha_device->mqtt_client);
    aiio_mqtt_client_stop(ha_device->mqtt_client);
}

void homeAssistant_device_send_status(bool status)
{
    if (ha_device->mqtt_info.mqtt_connect_status) {
        if (ha_device->availability_topic==NULL) goto   __c_topic;
        else goto   send_status;
    __c_topic:
        ha_device->availability_topic = pvPortMalloc(128);
        memset(ha_device->availability_topic, 0, 128);
        sprintf(ha_device->availability_topic, "%s/%02x%02x%02x%02x%02x%02x/status", CONFIG_HA_AUTOMATIC_DISCOVERY, STA_MAC[0], STA_MAC[1], STA_MAC[2], STA_MAC[3], STA_MAC[4], STA_MAC[5]);
    send_status:
        if (ha_device->payload_available==NULL)ha_device->payload_available = "online";
        if (ha_device->payload_not_available==NULL)ha_device->payload_not_available = "offline";
        aiio_mqtt_client_publish(ha_device->mqtt_client, ha_device->availability_topic, status?ha_device->payload_available:ha_device->payload_not_available, status?strlen(ha_device->payload_available):strlen(ha_device->payload_not_available), 0, 0);
    }
    else {
        LOG_E("MQTT server is disconnect");
    }
}

void homeAssistant_device_add_entity(uint8_t* entity_type, void* ha_entity_list)
{
    //添加开关实体
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH)) {
        //添加 switch 节点
        ha_sw_entity_t* switch_node = (ha_sw_entity_t*)ha_entity_list;
        entity_swith_add_node(switch_node);
    }
    //添加light 实体
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT)) {
        LOG_I("HomeAssistant add light entity");
        ha_lh_entity_t* light_node = (ha_lh_entity_t*)ha_entity_list;
        entity_light_add_node(light_node);
    }
    //添加传感器
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR)) {
        LOG_I("HomeAssistant add sensor entity");
        ha_sensor_entity_t* sensor_node = (ha_sensor_entity_t*)ha_entity_list;
        entity_sensor_add_node(sensor_node);
    }
    //添加二进制传感器实体
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR)) {
        LOG_I("HomeAssistant add binary sensor entity");
        ha_Bsensor_entity_t* binary_sensor_node = (ha_Bsensor_entity_t*)ha_entity_list;
        entity_binary_sensor_add_node(binary_sensor_node);
    }

}

int homeAssistan_device_send_entity_state(uint8_t* entity_type, void* ha_entity_list, uint16_t state)
{
    int ret_id = -1;
    if (entity_type==NULL||ha_entity_list==NULL) {
        LOG_E("params error");
        return ret_id;
    }

    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH)) {
        ha_sw_entity_t* switch_node = (ha_sw_entity_t*)ha_entity_list;
        ret_id = aiio_mqtt_client_publish(ha_device->mqtt_client, switch_node->state_topic, state?switch_node->payload_on:switch_node->payload_off, state?strlen(switch_node->payload_on):strlen(switch_node->payload_off), 0, 0);
    }

    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT)) {
        ha_lh_entity_t* light_node = (ha_lh_entity_t*)ha_entity_list;
        //上报RGB值
        if (light_node->rgb.rgb_command_topic!=NULL) {
            char* rgb_data = pvPortMalloc(16);
            if (light_node->rgb.rgb_value_template==NULL) {
                memset(rgb_data, 0, 16);
                sprintf(rgb_data, "%d,%d,%d", light_node->rgb.red, light_node->rgb.green, light_node->rgb.blue);
                ret_id = aiio_mqtt_client_publish(ha_device->mqtt_client, light_node->rgb.rgb_state_topic, rgb_data, strlen(rgb_data), 0, 0);
                if (ret_id<false) LOG_E("publish is fali");
            }
            vPortFree(rgb_data);
        }
        if (light_node->payload_on!=NULL && light_node->payload_off!=NULL)
            ret_id = aiio_mqtt_client_publish(ha_device->mqtt_client, light_node->state_topic, state?light_node->payload_on:light_node->payload_off, state?strlen(light_node->payload_on):strlen(light_node->payload_off), 0, 0);
        else {
            ret_id = aiio_mqtt_client_publish(ha_device->mqtt_client, light_node->state_topic, state?"ON":"OFF", state?2:3, 0, 0);
        }
    }

    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR)) {
        ha_sensor_entity_t* sensor_node = (ha_sensor_entity_t*)ha_entity_list;
        if (sensor_node->sensor_data==NULL) {
            // LOG_E("sensor_node sensor_data is NULL,data is state=%d", state);
            sensor_node->sensor_data = pvPortMalloc(16);
            memset(sensor_node->sensor_data, 0, 16);
            sprintf(sensor_node->sensor_data, "%d", state);
            ret_id = aiio_mqtt_client_publish(ha_device->mqtt_client, sensor_node->state_topic, sensor_node->sensor_data, strlen(sensor_node->sensor_data), 0, 0);
            memset(sensor_node->sensor_data, 0, 16);
            vPortFree(sensor_node->sensor_data);
            sensor_node->sensor_data = NULL;
        }
        else
            ret_id = aiio_mqtt_client_publish(ha_device->mqtt_client, sensor_node->state_topic, sensor_node->sensor_data, strlen(sensor_node->sensor_data), 0, 0);

    }

    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR)) {
        ha_Bsensor_entity_t* binary_sensor_node = (ha_sensor_entity_t*)ha_entity_list;
        ret_id = aiio_mqtt_client_publish(ha_device->mqtt_client, binary_sensor_node->state_topic, state?binary_sensor_node->payload_on:binary_sensor_node->payload_off, state?strlen(binary_sensor_node->payload_on):strlen(binary_sensor_node->payload_off), 0, 0);
    }

    return ret_id;
}

void* homeAssisatant_fine_entity(uint8_t* entity_type, const char* unique_id)
{
    if (entity_type==NULL | unique_id==NULL) {
        LOG_E("parama is NULL");
        return NULL;
    }
    //查找switch实体
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SWITCH)) {
        ha_sw_entity_t* switch_cur = ha_device->entity_switch->switch_list->next;
        while (switch_cur!=ha_device->entity_switch->switch_list) {
            if (!strcmp(switch_cur->unique_id, unique_id)) {
                return switch_cur;
            }
            switch_cur = switch_cur->next;
        }
        LOG_E("There is no %s entity unique id %s", entity_type, unique_id);
    }
    //查找 light 实体
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_LIGHT)) {
        ha_lh_entity_t* light_cur = ha_device->entity_light->light_list->next;
        while (light_cur!=ha_device->entity_light->light_list) {
            if (!strcmp(light_cur->unique_id, unique_id)) {
                return light_cur;
            }
            light_cur = light_cur->next;
        }
        LOG_E("There is no %s entity unique id %s", entity_type, unique_id);
    }
    //查找 sensor 实体
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_SENSOR)) {
        ha_sensor_entity_t* sensor_cur = ha_device->entity_sensor->sensor_list->next;
        while (sensor_cur !=ha_device->entity_sensor->sensor_list) {
            if (!strcmp(sensor_cur->unique_id, unique_id)) {
                return sensor_cur;
            }
            sensor_cur = sensor_cur->next;
        }
        LOG_E("There is no %s entity unique id %s", entity_type, unique_id);
    }
    //查找 binary sensor 实体
    if (!strcmp(entity_type, CONFIG_HA_ENTITY_BINARY_SENSOR)) {
        ha_Bsensor_entity_t* binary_sensor_cur = ha_device->entity_binary_sensor->binary_sensor_list->next;
        while (binary_sensor_cur !=ha_device->entity_binary_sensor->binary_sensor_list) {
            if (!strcmp(binary_sensor_cur->unique_id, unique_id)) {
                return binary_sensor_cur;
            }
            binary_sensor_cur = binary_sensor_cur->next;
        }
        LOG_E("There is no %s entity unique id %s", entity_type, unique_id);
    }
    return NULL;
}
