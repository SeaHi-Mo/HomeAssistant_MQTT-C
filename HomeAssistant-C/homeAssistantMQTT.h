/**
 * @file homeAssisatntMQTT.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-03
 *
 * @copyright Copyright (c) 2024
 *
*/
#ifndef HOMEASSISTANTMQTT_H
#define HOMEASSISTANTMQTT_H

#include "homeAssistantDevConfig.h"
#include "aiio_mqtt_client.h"

#define HOMEASSISTANT_STATUS_ONLINE 1
#define HOMEASSISTANT_STATUS_OFFLINE 0

#define HOMEASSISTANT_CONFIG_DATA_SIZE 1024

typedef enum {
    HA_EVENT_NONE = 0,
    HA_EVENT_MQTT_CONNECED,
    HA_EVENT_MQTT_DISCONNECT,
    HA_EVENT_MQTT_COMMAND_SWITCH,
    HA_EVENT_MQTT_COMMAND_LIGHT_SWITCH,
    HA_EVENT_MQTT_COMMAND_LIGHT_RGB_UPDATE,
    HA_EVENT_MQTT_COMMAND_LIGHT_BRIGHTNESS,
    HA_EVENT_MQTT_ERROR,
}ha_event_t;
/**
 * @brief 连接信息
 *
*/
typedef struct homeAssisatnt_netinfo {
    uint8_t* ssid;
    uint8_t* password;
    uint8_t* bssid;
    uint8_t* ipv4_addr;
}homeAssisatnt_netinfo_t;
/**
 * @brief 设备遗嘱，用于掉线之后发送下线信息
 *
*/
typedef struct homeAssisatnt_device_will
{
    uint8_t* will_topic;
    uint8_t* will_msg;
    uint8_t will_msg_len;
    uint8_t will_qos;
    bool will_retain;
}homeAssisatnt_device_will_t;
/**
 * @brief MQTT 连接信息
 *
*/
typedef struct homeAssisatnt_mqtt_info {
    bool mqtt_connect_status;
    uint8_t* mqtt_host;
    uint16_t port;
    uint8_t* mqtt_clientID;
    uint8_t* mqtt_username;
    uint8_t* mqtt_password;
    int mqtt_keeplive;
    homeAssisatnt_device_will_t will;
}ha_mqtt_info_t;
/**
 * @brief switch实体信息
 * @brief 成员含义可以参考：https://www.home-assistant.io/integrations/switch.mqtt/
*/
typedef struct homeAssisatnt_entity_switch {
    uint8_t* name;
    uint8_t* entity_config_topic;
    uint8_t* object_id;
    uint8_t* availability_mode;
    uint8_t* availability_template;
    uint8_t* availability_topic;
    uint8_t* command_topic;
    uint8_t* state_topic;
    uint8_t* device_class;
    bool enabled_by_default;
    uint8_t* encoding;
    uint8_t* entity_category;
    uint8_t* icon;
    uint8_t* json_attributes_template;
    uint8_t* optimistic;
    uint8_t* payload_available;
    uint8_t* payload_not_available;
    uint8_t* payload_off;
    uint8_t* payload_on;
    uint8_t qos;
    bool retain;
    uint8_t* state_off;
    uint8_t* state_on;
    uint8_t* unique_id;
    uint8_t* value_template;
    uint8_t* switch_config_data;
    bool switch_state;
    struct homeAssisatnt_entity_switch* prev;
    struct homeAssisatnt_entity_switch* next;
}ha_sw_entity_t;

typedef struct {
    uint8_t* entity_type;
    ha_sw_entity_t* switch_list;
    ha_sw_entity_t* command_switch;
}ha_swlist_t;

/**
 * @brief ligth实体信息
 * @brief 成员含义可以参考：https://www.home-assistant.io/integrations/switch.mqtt/
*/
struct light_brightness_t {
    uint8_t* brightness_command_topic;
    uint8_t* brightness_command_template;
    uint8_t* brightness_scale;
    uint8_t* brightness_state_topic;
    uint8_t* brightness_value_template;
    uint8_t brightness;
};

struct light_color_temp_t {
    uint8_t* color_mode_state_topic;
    uint8_t* color_mode_value_template;
    uint8_t* color_temp_command_template;
    uint8_t* color_temp_command_topic;
    uint8_t* color_temp_state_topic;
    uint8_t* color_temp_value_template;
};

struct light_effect_t {
    uint8_t* effect_command_topic;
    uint8_t* effect_command_template;
    uint8_t* effect_list;
    uint8_t* effect_state_topic;
    uint8_t* effect_value_template;
};

struct light_hs_t {
    uint8_t* hs_command_template;
    uint8_t* hs_command_topic;
    uint8_t* hs_state_topic;
    uint8_t* hs_value_template;
};

struct light_rgb_t {
    uint8_t* rgb_command_template;
    uint8_t* rgb_command_topic;
    uint8_t* rgb_state_topic;
    uint8_t* rgb_value_template;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct light_rgbw_t {
    uint8_t* rgbw_command_template;
    uint8_t* rgbw_command_topic;
    uint8_t* rgbw_state_topic;
    uint8_t* rgbw_value_template;
};

struct light_rgbww_t {
    uint8_t* rgbww_command_template;
    uint8_t* rgbww_command_topic;
    uint8_t* rgbww_state_topic;
    uint8_t* rgbww_value_template;
};

struct light_white_t {
    uint8_t* white_command_topic;
    uint8_t* white_scale;
};

struct light_xy_t {

    uint8_t* xy_command_template;
    uint8_t* xy_command_topic;
    uint8_t* xy_state_topic;
    uint8_t* xy_value_template;
};

typedef  struct homeAssisatnt_entity_light {
    uint8_t* name;
    uint8_t* entity_config_topic;
    uint8_t* object_id;
    uint8_t* availability_mode;
    uint8_t* availability_template;
    uint8_t* availability_topic;
    uint8_t* command_topic;
    uint8_t* state_topic;
    uint8_t* device_class;
    bool enabled_by_default;
    uint8_t* encoding;
    uint8_t* entity_category;
    uint8_t* icon;
    uint8_t* json_attributes_template;
    uint8_t* optimistic;
    uint8_t* payload_available;
    uint8_t* payload_not_available;
    uint8_t* payload_off;
    uint8_t* payload_on;
    uint8_t qos;
    uint8_t* light_config_data;
    struct light_brightness_t brightness;
    struct light_color_temp_t color_temp;
    struct light_effect_t effect;
    struct light_hs_t hs;
    struct light_rgb_t rgb;
    struct light_rgbw_t rgbw;
    struct light_rgbww_t rbgww;
    struct light_white_t white;
    struct light_xy_t xy;
    bool retain;
    uint8_t* state_off;
    uint8_t* state_on;
    uint8_t* unique_id;
    uint8_t* value_template;
    bool light_state;

    struct homeAssisatnt_entity_light* prev;
    struct homeAssisatnt_entity_light* next;
}ha_lh_entity_t;

typedef struct {
    uint8_t* entity_type;
    ha_lh_entity_t* light_list;
    ha_lh_entity_t* command_light;
}ha_lhlist_t;

/**
 * @brief  设备信息
 *
*/
typedef struct homeAssisatnt_device {
    uint8_t* name;
    uint8_t* hw_version;
    uint8_t* sw_version;
    uint8_t* identifiers;
    uint8_t* manufacturer;
    uint8_t* model;
    uint8_t* via_device;
    uint8_t* suggest_area;
    uint8_t* availability_topic;
    uint8_t* payload_available;
    uint8_t* payload_not_available;
    homeAssisatnt_netinfo_t wifi_info;
    ha_swlist_t* entity_switch;
    ha_lhlist_t* entity_light;
    aiio_mqtt_client_handle_t mqtt_client;
    ha_mqtt_info_t mqtt_info;
    void (*event_cb)(ha_event_t event, struct homeAssisatnt_device* ha_dev);
}homeAssisatnt_device_t;

/**
 * @brief homeAssistant 初始化，里面包含了设备的默认信息的定义，MQTT的初始化
 *
 * @param ha_dev 设备句柄
*/
// void homeAssistant_device_init(homeAssisatnt_device_t* ha_dev);
void homeAssistant_device_init(homeAssisatnt_device_t* ha_dev, void(*event_cb)(ha_event_t, homeAssisatnt_device_t*));
/**
 * @brief HomeAssistant 启动连接
 *
*/
void homeAssistant_device_start(void);
/**
 * @brief HomeAssistant 断开连接
 *
*/
void homeAssisatnt_device_stop(void);
/**
 * @brief 添加实体
 *
 * @param entity_type 实体名称
 * @param ha_entity_list 实体描述
*/
void homeAssistant_device_add_entity(uint8_t* entity_type, void* ha_entity_list);
/**
 * @brief 发送设备状态
 *
 * @param status 1：发送“online” 0：发送“offline”
*/
void homeAssistant_device_send_status(bool status);

/**
 * @brief homeAssistan_device_send_entity_state
 *       发送实体状态
 * @param entity_type 实体类型
 * @param ha_entity_list 相应实体结构体
 * @param state 状态
 * @return int 成功返回消息ID，失败返回-1
*/
int homeAssistan_device_send_entity_state(uint8_t* entity_type, void* ha_entity_list, uint16_t state);
#endif

