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

//HomeAssistant 的MQTT 事件 只要部分事件，更多事件还在更新当中
typedef enum {
    HA_EVENT_NONE = 0,
    HA_EVENT_MQTT_CONNECED, //服务器连接成功事件
    HA_EVENT_MQTT_DISCONNECT,//服务器断开事件
    HA_EVENT_MQTT_COMMAND_SWITCH,//服务器下发开关命令事件，当在HA操作开关时，会触发这个事件
    HA_EVENT_MQTT_COMMAND_LIGHT_SWITCH,//light 灯的开关事件
    HA_EVENT_MQTT_COMMAND_LIGHT_RGB_UPDATE,//light 灯的RGB 颜色下发事件
    HA_EVENT_MQTT_COMMAND_LIGHT_BRIGHTNESS,//light 灯的亮度数据下发事件
    HA_EVENT_HOMEASSISTANT_STATUS_ONLINE,
    HA_EVENT_HOMEASSISTANT_STATUS_OFFLINE,
    HA_EVENT_MQTT_ERROR,
}ha_event_t;

/**
 * @brief 连接信息 ，这个没啥用，用不到
 *
*/
typedef struct homeAssisatnt_netinfo {
    uint8_t* ssid; //路由器名称，不支持中文和5GHz
    uint8_t* password;//路由器密码
    uint8_t* bssid;//路由器bssid
    uint8_t* ipv4_addr;//获取到的IP地址
}homeAssisatnt_netinfo_t;
/**
 * @brief 设备遗嘱，用于掉线之后发送下线信息
 *
*/
typedef struct homeAssisatnt_device_will
{
    uint8_t* will_topic; //遗嘱需要发送的Topic
    uint8_t* will_msg;  //需要发送的消息，一般: offline
    uint8_t will_msg_len;//消息的长度
    uint8_t will_qos;//服务质量
    bool will_retain;//是否保留
}homeAssisatnt_device_will_t;
/**
 * @brief MQTT 连接信息
 *
*/
typedef struct homeAssisatnt_mqtt_info {
    bool mqtt_connect_status; //MQTT 服务器连接状态，true 为已连接。false 为未连接
    uint8_t* mqtt_host;      //MQTT 服务器地址，支持域名解析
    uint16_t port;           // MQTT 服务器端口
    uint8_t* mqtt_clientID;  //MQTT 的客户端ID
    uint8_t* mqtt_username;  //MQTT 接入的用户名
    uint8_t* mqtt_password;  //MQTT 接入密码
    int mqtt_keeplive;      //保活时间
    homeAssisatnt_device_will_t will;//遗嘱内容
}ha_mqtt_info_t;
/**
 * @brief switch实体信息
 * @brief 成员含义可以参考：https://www.home-assistant.io/integrations/switch.mqtt/
*/
typedef struct homeAssisatnt_entity_switch {
    uint8_t* name;                   //实体名称 必须要赋值
    uint8_t* entity_config_topic;    //实体自动发现需要的topic，已经自动赋值，可以不配置
    uint8_t* object_id;              //实体 工程id 可以为NULL
    uint8_t* availability_mode;      //实体上下线的模式 可以为NULL
    uint8_t* availability_template;  //实体上下线的数据格式，建议为NULL，采用默认
    uint8_t* availability_topic;     //实体上下线上报的Topic,建议保持默认
    uint8_t* command_topic;          //命令接收的Topic,需要订阅
    uint8_t* state_topic;            //上报给HA的数据的Topic
    uint8_t* device_class;           //设备类型，可以留空
    bool enabled_by_default;         //默认LED的状态
    uint8_t* encoding;               //编码方式
    uint8_t* entity_category;        //实体属性，保持NULL
    uint8_t* icon;                    //图标
    uint8_t* json_attributes_template;//json 数据模板
    uint8_t* optimistic;              //记忆模式
    uint8_t* payload_available;       //在线消息内容 默认"online"
    uint8_t* payload_not_available;   //离线消息内容 默认"offline"
    uint8_t* payload_off;             //开关状态内容，默认"ON"
    uint8_t* payload_on;               //开关状态内容，默认"OFF"
    uint8_t qos;                      //消息服务质量
    bool retain;                       //是否保留该信息     
    uint8_t* state_off;               //状态 关          
    uint8_t* state_on;                  //状态 开
    uint8_t* unique_id;                // 唯一的识别码，这个必须配置 
    uint8_t* value_template;           //数据格式 
    uint8_t* config_data;      //开关的自动发现的json数据
    bool switch_state;                 //当前开关状态 
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
    uint8_t* config_data;
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
 * @brief 传感器实体
 *
*/
typedef enum {
    Class_None = 0,
    Class_apparent_power,
    Class_aqi,
    Class_atmospheric_pressure,
    Class_battery,
    Class_carbon_dioxide,
    Class_carbon_monoxide,
    Class_current,
    Class_data_rate,
    Class_data_size,
    Class_date,
    Class_distance,
    Class_duration,
    Class_energy,
    Class_energy_storage,
    Class_ha_enum,
    Class_frequency,
    Class_gas,
    Class_humidity,
    Class_illuminance,
    Class_irradiance,
    Class_moisture,
    Class_monetary,
    Class_nitrogen_dioxide,
    Class_nitrogen_monoxide,
    Class_nitrous_oxide,
    Class_ozone,
    Class_ph,
    Class_pm1,
    Class_pm25,
    Class_pm10,
    Class_power_factor,
    Class_power,
    Class_precipitation,
    Class_precipitation_intensity,
    Class_pressure,
    Class_reactive_power,
    Class_signal_strength,
    Class_sound_pressure,
    Class_speed,
    Class_sulphur_dioxide,
    Class_temperature,
    Class_timestamp,
    Class_volatile_organic_compounds,
    Class_volatile_organic_compounds_parts,
    Class_voltage,
    Class_volume,
    Class_volume_flow_rate,
    Class_volume_storage,
    Class_water,
    Class_weight,
    Class_wind_speed
}ha_sensor_class_t;

typedef  struct homeAssisatnt_entity_sensor {
    uint8_t* name;
    uint8_t* entity_config_topic;
    uint8_t* config_data;
    uint8_t* object_id;
    uint8_t* unique_id;
    uint8_t* availability_mode;
    uint8_t* availability_template;
    uint8_t* availability_topic;
    ha_sensor_class_t device_class;
    uint8_t* payload_available;
    uint8_t* payload_not_available;
    uint16_t suggested_display_precision;
    bool enabled_by_default;
    uint8_t* entity_category;
    uint8_t* icon;
    uint8_t* json_attributes_template;
    uint8_t* json_attributes_topic;
    uint8_t* last_reset_value_template;
    uint8_t qos;
    uint8_t* state_class;
    uint8_t* state_topic;
    uint8_t* unit_of_measurement;
    uint8_t* value_template;
    uint16_t expire_after;
    bool force_update;
    void* sensor_data;

    struct homeAssisatnt_entity_sensor* prev;
    struct homeAssisatnt_entity_sensor* next;
}ha_sensor_entity_t;

typedef struct {
    uint8_t* entity_type;
    ha_sensor_entity_t* sensor_list;
}ha_sensorlist_t;


/**
 * @brief 二进制传感器实体
 *
*/
typedef enum {
    Bclass_None = 0,
    Bclass_battery,
    Bclass_battery_charging,
    Bclass_carbon_monoxide,
    Bclass_cold,
    Bclass_connectivity,
    Bclass_door,
    Bclass_garage_door,
    Bclass_gas,
    Bclass_heat,
    Bclass_light,
    Bclass_lock,
    Bclass_moisture,
    Bclass_motion,
    Bclass_moving,
    Bclass_occupancy,
    Bclass_opening,
    Bclass_plug,
    Bclass_power,
    Bclass_presence,
    Bclass_problem,
    Bclass_running,
    Bclass_afety,
    Bclass_smoke,
    Bclass_sound,
    Bclass_tamper,
    Bclass_update,
    Bclass_vibration,
    Bclass_window,
}ha_Bsensor_class_t;

typedef  struct homeAssisatnt_entity_binary_sensor {
    uint8_t* name;
    uint8_t* entity_config_topic;
    uint8_t* config_data;
    uint8_t* object_id;
    uint8_t* unique_id;
    uint8_t* availability_mode;
    uint8_t* availability_template;
    uint8_t* availability_topic;
    uint8_t* payload_available;
    uint8_t* payload_not_available;
    ha_Bsensor_class_t device_class;
    uint8_t* entity_category;
    uint8_t* icon;
    uint8_t* json_attributes_template;
    uint8_t* json_attributes_topic;
    uint8_t qos;
    uint8_t* state_class;
    uint8_t* state_topic;
    uint8_t* value_template;
    uint16_t expire_after;
    bool force_update;
    bool enabled_by_default;
    bool state;
    uint8_t* payload_off;
    uint8_t* payload_on;
    struct homeAssisatnt_entity_binary_sensor* prev;
    struct homeAssisatnt_entity_binary_sensor* next;
}ha_Bsensor_entity_t;

typedef struct {
    uint8_t* entity_type;
    ha_Bsensor_entity_t* binary_sensor_list;
}ha_binary_sensorlist_t;
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
    ha_sensorlist_t* entity_sensor;
    ha_binary_sensorlist_t* entity_binary_sensor;
    aiio_mqtt_client_handle_t mqtt_client;
    ha_mqtt_info_t mqtt_info;
    bool homeassistant_online;
    void (*event_cb)(ha_event_t event, struct homeAssisatnt_device* ha_dev);
}homeAssisatnt_device_t;

/**
 * @brief homeAssistant 初始化，里面包含了设备的默认信息的定义，MQTT的初始化
 *
 * @param ha_dev 设备句柄
*/
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
/**
 * @brief homeAssisatant_fine_entity
 *          查找实体
 * @param entity_type 实体类型
 * @param unique_id 实体的 unique id
 * @return void* 返回的实体指针
*/
void* homeAssisatant_fine_entity(uint8_t* entity_type, const char* unique_id);
#endif

