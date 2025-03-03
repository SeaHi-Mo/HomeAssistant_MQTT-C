<div style="text-align: center;">

# Entity User-Manuals 实体使用指南

</div>

## 前言
### 实体添加函数
实体的添加，固定使用同一个函数,并使用`entity_type`区分：
```c
void homeAssistant_device_add_entity(char* entity_type, void* ha_entity_list)
```
>参数说明：`entity_type`为实体类型，`ha_entity_list`为实体信息结构体指针。


### 发送信息函数
HomeAssistant_MQTT-C 中，发送信息使用同一个函数，并使用`entity_type`区分：
```c
int homeAssistant_device_send_entity_state(char* entity_type, void* ha_entity_list, unsigned short state);
```
>参数说明：`entity_type`为实体类型，`ha_entity_list`为实体信息结构体指针，`state`为状态值。

### entity_type 实体类型：

| 实体类型 | 说明 | 是否已支持|
| :----: | :----: |:----: |
| CONFIG_HA_ENTITY_ALARM_CONTROL_PANEL | 报警控制面板 实体 |:x:
| CONFIG_HA_ENTITY_BINARY_SENSOR | 高低电平传感器 实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_BUTTON | 按钮实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_CAMERA | 摄像头实体 |:x:
| CONFIG_HA_ENTITY_COVER | 门类实体，窗帘、车门等 |:x:
| CONFIG_HA_ENTITY_DEVICE_TRACKER | 跟踪器实体，GPS定位等 |:x:
| CONFIG_HA_ENTITY_DEVICE_TRIGGER | 触发器实体 |:x:
| CONFIG_HA_ENTITY_EVENT | 事件实体 |:x:
| CONFIG_HA_ENTITY_FAN | 风扇实体 |:x:
| CONFIG_HA_ENTITY_HUMIDIFIER | 加湿器实体 |:x:
| CONFIG_HA_ENTITY_IMAGE | 图片实体 |:x:
| CONFIG_HA_ENTITY_CLIMATE_HVAC | 空调实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_LAWN_MOWER | 割草机实体 |:x:
| CONFIG_HA_ENTITY_LIGHT | 灯实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_LOCK | 门锁实体 |:x:
| CONFIG_HA_ENTITY_NUMBER | 数字实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_SCENE | 场景实体 |:x:
| CONFIG_HA_ENTITY_SELECT | 选择器实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_SENSOR | 传感器实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_SIREN | 警报器实体 |:x:
| CONFIG_HA_ENTITY_SWITCH | 开关实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_UPDATE | 更新实体 |:x:
| CONFIG_HA_ENTITY_TAG_SCANNER | 标签扫描仪实体 |:x:
| CONFIG_HA_ENTITY_TEXT | 文本实体 |:white_check_mark:|
| CONFIG_HA_ENTITY_VACUUM | 真空吸尘器实体 |:x:
| CONFIG_HA_ENTITY_VALVE | 阀门实体 |:x:
| CONFIG_HA_ENTITY_WATER_HEATER | 热水器实体 |:x:
### HomeAssistant MQTT 控制事件
HomeAssistant MQTT 控制事件用于接收来自HomeAssistant的控制指令，例如开关、按钮、数字等实体的控制指令。每种事件需要开启对应的实体资源才可以使用。事件列表如下：
| 事件类型 | 权限 |说明 |
| :----: | :----: |:----: |
| HA_EVENT_NONE | / |无事件 |
| HA_EVENT_MQTT_CONNECED | / |服务器连接成功事件 |
| HA_EVENT_MQTT_DISCONNECT | / |服务器断开事件 |
| HA_EVENT_HOMEASSISTANT_STATUS_ONLINE | / |HomeAssisstant 在线事件 |
| HA_EVENT_HOMEASSISTANT_STATUS_OFFLINE | / |HomeAssistant 掉线事件 |
| HA_EVENT_MQTT_COMMAND_SWITCH | 需开启Switch资源 |服务器下发开关命令事件，当在HA操作开关时，会触发这个事件 |
| HA_EVENT_MQTT_COMMAND_LIGHT_SWITCH | 需开启light资源 |light 灯的开关事件 |
| HA_EVENT_MQTT_COMMAND_LIGHT_RGB_UPDATE | 需开启light资源 |light 灯的RGB 颜色下发事件 |
| HA_EVENT_MQTT_COMMAND_LIGHT_BRIGHTNESS | 需开启light资源 |light 灯的亮度数据下发事件 |
| HA_EVENT_MQTT_COMMAND_TEXT_VALUE | 需开启Text资源  |服务器下发text内容事件 |
| HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_POWER | 需开启climateHVAC资源  |服务器下发的空调开关事件 |
| HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_MODES | 需开启climateHVAC资源 |设置模式 |
| HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_TEMP | 需开启climateHVAC资源 |设置温度 |
| HA_EVENT_MQTT_COMMAND_CLIMATE_HVAC_FAN_MODES | 需开启climateHVAC资源 |设置风力 |
| HA_EVENT_MQTT_COMMAND_SELECT_VALUE | 需开启select资源 |服务器下发select内容事件 |
| HA_EVENT_MQTT_COMMAND_BUTTON | 需开启button资源 |服务器下发button内容事件 |
| HA_EVENT_MQTT_ERROR | / |服务器错误事件 |

## Switch 

Switch 实体是HomeAssistant中的一种设备类型，用于表示可以打开或关闭的设备，例如灯泡、插座等。
### 步骤1.开启资源

任何实体，在使用之前都需要开启对应的资源，Switch资源,在HomeAssistant_MQTT-C 中，需要通过以下方式开启Switch资源：
- 1. 打开[homeAssistantDevConfig.h](../HomeAssistant-C/homeAssistantDevConfig.h)
- 2. 设置`#define MQTT_SWITCH_ENABLE`为`1`<br>
如：<br>![alt text](./IMG/sw_1.png)

### 步骤2.创建实体

> :warning:**前提条件:**<br>
> - 1.创建实体的前提条件是已经开启了对应的资源，例如Switch实体，需要开启Switch资源。<br>
> - 2.必须设备上线之前创建好实体，否则设备上线后，HomeAssistant无法识别实体。

Switch实体，在HomeAssistant_MQTT-C 中，需要通过以下方式创建实体：
- 1. 创建 Switch 实体信息结构体，并初始化。必填参数有`name`、`unique_id`，其他参数可选。<br>
```c
static ha_sw_entity_t sw = {
                .name = "开关",
                .unique_id = "sw_1",
                .icon = "mdi:power",//图标：可选
            };
```
- 2. 调用`homeAssistant_device_add_entity`函数，将实体添加到设备中，如：
```c
static ha_sw_entity_t sw = {
                .name = "开关",
                .unique_id = "sw_1",
               .icon = "mdi:power",
            };
homeAssistant_device_add_entity(CONFIG_HA_ENTITY_SWITCH, &sw);
```
>其中，`CONFIG_HA_ENTITY_SWITCH`为实体类型，`&sw`为实体信息结构体指针。

#### Switch 实体参数

|参数名|权限|类型|描述|
|:----|:----|:----|:----|
|name|必选|char*|实体名称|
|entity_config_topic|可选|char*|实体自动发现需要的topic，已经自动赋值，可以不配置|
|object_id|可选|char*|实体 工程id 可以为NULL|
|availability_mode|可选|char*|实体上下线的模式 可以为NULL|
|availability_template|可选|char*|实体上下线的数据格式，建议为NULL，采用默认|
|availability_topic|可选|char*|实体上下线上报的Topic,建议保持默认|
|command_topic|可选|char*|命令接收的Topic,需要订阅|
|state_topic|可选|char*|上报给HA的数据的Topic，默认已配置|
|device_class|可选|char*|设备类型，可以留空|
|enabled_by_default|可选|bool|默认LED的状态|
|encoding|可选|char*|编码方式|
|entity_category|可选|char*|实体属性，保持NULL|
|icon|可选|char*|图标|
|json_attributes_template|可选|char*|json 数据模板|
|optimistic|可选|char*|记忆模式|
|payload_available|可选|char*|在线消息内容 默认"online"|
|payload_not_available|可选|char*|离线消息内容 默认"offline"|
|payload_off|可选|char*|开关状态内容，默认"ON"|
|payload_on|可选|char*|开关状态内容，默认"OFF"|
|qos|可选|int|消息服务质量|
|retain|可选|bool|是否保留该信息|
|state_off|可选|char*|状态 关|
|state_on|可选|char*|状态 开|
|unique_id|必选|char*|唯一的识别码，这个必须配置|
|value_template|可选|char*|数据格式|

### 步骤3.接收控制指令
HomeAssistant_MQTT-C 中，Switch实体的控制指令，通过`ha_event_cb`回调函数接收，会触发`HA_EVENT_MQTT_COMMAND_SWITCH`事件，可以使用 `power_state` 判断开关指令:`0`为关，`1`为开。<br>
示例如下：
```c
void ha_event_cb(ha_event_t event, homeAssisatnt_device_t* ha_dev)
{
    switch (event)
    {
        // ... 其他事件
        case HA_EVENT_MQTT_COMMAND_SWITCH:
        {
            // 接收开关控制指令
              HA_LOG_I("<<<<<<<<<< HA_EVENT_MQTT_COMMAND_SWITCH=%s\r\n", ha_dev->entity_switch->command_switch->power_state?"ON":"OFF");
            break;
        }
        // ... 其他事件
    }
}
```

### 步骤4.发送开关状态
HomeAssistant_MQTT-C 中，Switch实体的状态，通过`homeAssistant_device_entity_update`函数发送，如：
```c
  homeAssistant_device_send_entity_state(CONFIG_HA_ENTITY_SWITCH, &sw, 1);
```
>其中，`CONFIG_HA_ENTITY_SWITCH`为实体类型，`&sw`为实体信息结构体指针，`1`为开关状态，`1`为开，`0`为关。

## Sensor 

Sensor 实体是HomeAssistant中的一种设备类型，用于表示可以测量某种状态的设备，例如温度、湿度、光照强度等。

### 步骤1.开启资源
任何实体，在使用之前都需要开启对应的资源，Sensor资源,在HomeAssistant_MQTT-C 中，需要通过以下方式开启Sensor资源：
- 1. 打开[homeAssistantDevConfig.h](../HomeAssistant-C/homeAssistantDevConfig.h)
- 2. 设置`#define MQTT_SENSOR_ENABLE`为`1`<br>
如：<br>![alt text](./IMG/sensor_1.png)

### 步骤2.创建实体
> :warning:**前提条件:**<br>
> - 1.创建实体的前提条件是已经开启了对应的资源，例如Sensor实体，需要开启Sensor资源。<br>
> - 2.必须设备上线之前创建好实体，否则设备上线后，HomeAssistant无法识别实体。

Sensor实体，在HomeAssistant_MQTT-C 中，需要通过以下方式创建实体：
- 1. 创建 Sensor 实体信息结构体，并初始化。必填参数有`name`、`unique_id`其他参数可选。<br>
  ```c
  static ha_sensor_entity_t sensor = {
                .name = "温度",
                .unique_id = "sensor_1",
            };
  ```
- 2. 调用`homeAssistant_device_add_entity`函数，将实体添加到设备中，如：
  ```c
  static ha_sensor_entity_t sensor = {
                .name = "温度",
                .unique_id = "sensor_1",
            };
  homeAssistant_device_add_entity(CONFIG_HA_ENTITY_SENSOR, &sensor);
  ```
  >其中，`CONFIG_HA_ENTITY_SENSOR`为实体类型，`&sensor`为实体信息结构体指针。
#### Sensor 实体参数
|参数名|权限|类型|描述|
|:----|:----|:----|:----|
|name|必选|char*|实体名称|
|entity_config_topic|可选|char*|配置主题|
|config_data|可选|char*|配置数据|
|object_id|可选|char*|对象ID|
|unique_id|必选|char*|唯一的识别码，这个必须配置|
|availability_mode|可选|char*|可用性模式|
|availability_template|可选|char*|可用性模板|
|availability_topic|可选|char*|可用性主题|
|device_class|可选|ha_sensor_class_t|设备类|
|payload_available|可选|char*|可用负载|
|payload_not_available|可选|char*|不可用负载|
|suggested_display_precision|可选|unsigned short|建议显示精度|
|enabled_by_default|可选|bool|默认启用|
|entity_category|可选|char*|实体类别|
|icon|可选|char*|图标|
|json_attributes_template|可选|char*|JSON属性模板|
|json_attributes_topic|可选|char*|JSON属性主题|
|last_reset_value_template|可选|char*|最后重置值模板|
|qos|可选|unsigned short|服务质量|
|retain|可选|bool|保留|
|state_class|可选|char*|状态类|
|state_topic|可选|char*|状态主题|
|unit_of_measurement|可选|char*|单位|
|value_template|可选|char*|值模板|
|expire_after|可选|unsigned short|过期时间|
|force_update|可选|bool|强制更新|
|sensor_data|可选|void*|传感器数据|


### 步骤3.发送传感器状态

## Binary sensor

### 步骤1.开启资源

### 步骤2.创建实体

### 步骤3.发送传感器状态

## Light    
### 步骤1.开启资源

### 步骤2.创建实体

### 步骤3.接收控制指令

### 步骤4.发送状态

## Number

### 步骤1.开启资源

### 步骤2.创建实体

### 步骤3.接收数字

### 步骤4.发送数字

## climateHVAC  

### 步骤1.开启资源

### 步骤2.创建实体

### 步骤3.接收控制指令

### 步骤4.发送状态
