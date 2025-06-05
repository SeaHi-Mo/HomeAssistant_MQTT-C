/**
 * @file homeAssistantPort.h
 * @author Seahi (seahi-mo@foxmail.com)
 * @brief
 * @version 0.1
 * @date 2024-05-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef HOMEASSISTANTPORT_H
#define HOMEASSISTANTPORT_H
#include "homeAssistantMQTT.h"
#include "homeAssistantDevConfig.h"

// #define CONFIG_Ai_M6x
#define CONFIG_Ai_WB2

#define HA_LOG_ENABLE

#ifdef HA_LOG_ENABLE
#define DBG_TAG "HomeAssistant"
#define RECOLOR "\033[0m "
#define HA_LOG_D(...)                                   \
    do                                                  \
    {                                                   \
        printf("\033[0m[D][" DBG_TAG "] " __VA_ARGS__); \
        printf(RECOLOR);                                \
    } while (0)
#define HA_LOG_E(...)                                      \
    do                                                     \
    {                                                      \
        printf("\033[0;31m[E][" DBG_TAG "] " __VA_ARGS__); \
        printf(RECOLOR);                                   \
    } while (0)
#define HA_LOG_I(...)                                      \
    do                                                     \
    {                                                      \
        printf("\033[0;32m[I][" DBG_TAG "] " __VA_ARGS__); \
        printf(RECOLOR);                                   \
    } while (0)
#define HA_LOG_W(...)                                      \
    do                                                     \
    {                                                      \
        printf("\033[0;33m[W][" DBG_TAG "] " __VA_ARGS__); \
        printf(RECOLOR);                                   \
    } while (0)
#define HA_LOG_F(...)                                      \
    do                                                     \
    {                                                      \
        printf("\033[0;35m[F][" DBG_TAG "] " __VA_ARGS__); \
        printf(RECOLOR);                                   \
    } while (0)

#endif
int isPvPortMallocAllocated(void *ptr);
void homeAssistant_get_sta_mac(char *mac);
int homeAssistant_mqtt_port_init(homeAssisatnt_device_t *ha_dev);
int homeAssistant_mqtt_port_start(void);
int homeAssistant_mqtt_port_stop(void);
int homeAssistant_mqtt_port_public(const char *topic, const char *payload, int qos, bool retain);
int homeAssistant_mqtt_port_subscribe(const char *topic, int qos);
void homeAssistant_device_delete_entity(char *entity_type, char *unique_id);
void homeAssistant_mqtt_port_unsubscribe(const char *topic);
#endif