# Ai-WB2 移植文档
这个是Ai-WB2系列模组移植HomeAssistant-C 库的移植说明。

## 准备工作

在正式移植之前，需要以前准备一些条件，本文不会继续介绍这些条件的实现方式，因为它们都有参考：
- Ai-Thinker-WB2 SDK环境搭建，参考教程：[Ai-WB2系列模组linux开发环境搭建](https://blog.csdn.net/Boantong_/article/details/125604649?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522171575296516800222812642%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=171575296516800222812642&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-3-125604649-null-null.nonecase&utm_term=WB2&spm=1018.2226.3001.4450)
- 一片Ai-WB2的开发板，或者模组
- 在SDK当中克隆好了HomeAssistant_MQTT-C 源码

>为了确保功能够正确被调用，建议在例程源码目录进行克隆

##  源码移植

### 步骤1
在 **HomeAssistant_MQTT-C**文件夹当中创建名为`bouffalo.mk`的文件，添加以下内容：

```c
COMPONENT_ADD_INCLUDEDIRS := ./HomeAssistant-C
COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_SRCDIRS := ./HomeAssistant-C
```

### 步骤2

在一个能够连接WiFi（SDK中的 `applications/wifi/station/station` 是个不错的选择）的工程引用HomeAssistant-C库，在工程中的`Makefile`文件中，在最后一行的上面添加以下内容：

```
INCLUDE_COMPONENTS += axk_common tcp_transport http-parser axk_tls axk_mqtt cjson 
INCLUDE_COMPONENTS += HomeAssistant_MQTT-C
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/HomeAssistant_MQTT-C
```

>其中 `EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/Ai-M6x_HomeAssistant-C` 是HomeAssistant_MQTT-C库的实际路径，如果你的HomeAssistant_MQTT-C库不在本例程目录中，则需要修改`$(PROJECT_PATH)`，让编译工具能够找到它。

## 修改接口
HomeAssistant-C 的默认接口是Ai-M6x,为了防止编译顺利，强烈建议检查一下接口信息，打开[homeAssistantPort.h](../HomeAssistant-C/homeAssistantPort.h),检查宏定 **#define CONFIG_Ai_WB2** 是否被注释，如果被注释掉，你需要启用它来使用Ai-WB2的MQTT接口，同时把 **#define CONFIG_Ai_M6x** 注释掉：
```
//#define CONFIG_Ai_M6x
#define CONFIG_Ai_WB2
```

## 编译测试

紧接着你就可以引用`#include "homeAssistantPort.h"`来使用HomeAssistant-C，然后使用：
```
make 
```
正常的结果应该是这样的：
>****** Please SET BL60X_SDK_PATH ******
>****** Trying SDK PATH [/home/seahi/GDBS/Ai-Thinker-WB2/applications/wifi/station/../../..]
>Your configuration chipname is Ai-Thinker Ai-WB2 Wi-Fi&BLE Module
>Building Finish. To flash build output. run 'make flash' or:
>c  ...
>cd /home/seahi/GDBS/Ai-Thinker-WB2/tools/flash_tool && env SDK_APP_BIN=/home/seahi/GDBS/Ai-Thinker-WB2/applications/wifi/station/>build_out/station.bin SDK_BOARD=evb SDK_NAME=station SDK_MEDIA_BIN= SDK_ROMFS_DIR= SDK_DTS= SDK_XTAL= >BL_FLASH_TOOL_INPUT_PATH_cfg2_bin_input=/home/seahi/GDBS/Ai-Thinker-WB2/applications/wifi/station/build_out/station.bin ./>bflb_iot_tool-ubuntu --chipname=BL602 --baudrate=921600 --port=/dev/ttyUSB0 --pt=/home/seahi/GDBS/Ai-Thinker-WB2/applications/wifi/>station/img_conf/partition_cfg_2M.toml --dts=/home/seahi/GDBS/Ai-Thinker-WB2/applications/wifi/station/img_conf/>bl_factory_params_IoTKitA_40M.dts --firmware=/home/seahi/GDBS/Ai-Thinker-WB2/applications/wifi/station/build_out/station.bin