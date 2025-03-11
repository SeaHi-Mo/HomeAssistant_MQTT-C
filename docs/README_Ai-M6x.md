# Ai-M6x 移植文档

这个是Ai-M61 和Ai-M6x系列移植HomeAssistant-C 库的移植说明。

## 准备工作

在正式移植之前，需要以前准备一些条件，本文不会继续介绍这些条件的实现方式，因为它们都有参考：
- AiPi-Open-Kits SDK环境搭建，参考教程：[智能家居之旅，第三站：Ai-M61/M62 接入HomeAssistant 实现点灯](https://bbs.ai-thinker.com/forum.php?mod=viewthread&tid=44595&extra=page%3D1)
- 一片Ai-M6x的开发板，Ai-M61-32S或者其他型号，Ai-M62-12F或其他型号
- 克隆好了Ai-M6x_HomeAssistant-C 源码

请确认你已经具备了上面的条件再继续看下面的内容

##  源码移植

**强烈建议选择一个已经可以连接WiFi的例程或者源码，如果没有，你可以使用这个仓库中的例程：**
>https://gitee.com/Ai-Thinker-Open/HomeAssistant_example

然后打开工程的`CMakeLists.txt`文件，在`list(REMOVE_ITEM SRC_FILES "./main/main.c")`之前，添加以下内容：
```
aux_source_directory(../../../aiTinkerCloud_SDK/driver SRC_FILES)
aux_source_directory(.../../../aiTinkerCloud_SDK/protocol SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/system SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/utils SRC_FILES)

# aux_source_directory(./../../aiTinkerCloud_SDK/application SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/http-parser/src SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/aiio/tcp_transport SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/aiio/tcp_transport SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_tls SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_tls/aiio-tls-crypto SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/aiio/mqtt/aiio_mqtt SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/aiio/mqtt/aiio_mqtt/lib SRC_FILES)
aux_source_directory(../../../aiTinkerCloud_SDK/3rdparty/blufi SRC_FILES)

# 添加头文件的引用路径（Add .h include directories）
sdk_add_include_directories(../../../aiTinkerCloud_SDK/driver)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/protocol)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/system)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/utils)

# sdk_add_include_directories(../../aiTinkerCloud_SDK/application)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/http-parser/include)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/tcp_transport/include)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/tcp_transport/private_include)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_tls)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_tls/aiio-tls-crypto)
sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio)

# sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_http_client/include)
# sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_http_client/lib/include)
# sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_http_server/include)
# sdk_add_include_directories(../../../aiTinkerCloud_SDK/3rdparty/aiio/aiio_http_server/src/port)
sdk_add_include_directories(./../../../aiTinkerCloud_SDK/3rdparty/aiio/mqtt/aiio_mqtt/lib/include)
sdk_add_include_directories(./../../../aiTinkerCloud_SDK/3rdparty/aiio/mqtt/aiio_mqtt/include)

# sdk_add_include_directories(./../../../aiTinkerCloud_SDK/3rdparty/blufi)

# 添加HomeAssistant 库
file(GLOB_RECURSE HA ${CMAKE_CURRENT_SOURCE_DIR}/../Ai-M6x_HomeAssistant-C/HomeAssistant-C/*.c)
sdk_add_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Ai-M6x_HomeAssistant-C/HomeAssistant-C)
```
这里面的内容就是添加MQTT 依赖库和HomeAssisttant的连接库，你可以看一下`HomeAssistant_example/HomeAssistant_basic/CMakeLists.txt`,里边有很好的参考。

## 修改接口
HomeAssistant-C 的默认接口是Ai-M6x,为了防止编译顺利，强烈建议检查一下接口信息，打开[homeAssistantPort.h](../HomeAssistant-C/homeAssistantPort.h),检查宏定 **#define CONFIG_Ai_M6x** 是否被注释，如果被注释掉，你需要启用它来使用Ai-M6x的MQTT接口，同时把 **#define CONFIG_Ai_WB2** 注释掉：
```
#define CONFIG_Ai_M6x
// #define CONFIG_Ai_WB2
```

## 编译测试

紧接着你就可以引用`#include "homeAssistantPort.h"`来使用HomeAssistant-C，然后使用：
```
make 
```
正常的结果应该是这样的：
>[100%] Linking C executable build_out/HomeAssistant_basic_bl616.elf
>Generate /home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/HomeAssistant_basic_bl616.bin
>[100%] Built target HomeAssistant_basic_bl616.elf
>make -C build combine
>[13:52:39.179] - bflb firmware post process : V1.2.3
>[13:52:39.179] - Chipname: bl616
>[13:52:39.179] - Board config dir: ./../../../bl61x_SDK/os//bsp/board/bl616dk/config
>[13:52:39.179] - Create partition using partition_cfg_4M.toml
>[13:52:39.181] - Create dts for /home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/>HomeAssistant_basic_bl616.bin
>[13:52:39.181] - Create dts using bl_factory_params_IoTKitA_auto.dts
>[13:52:39.185] - 4K header found,append dts file
>[13:52:39.186] - Copy boot2_bl616_isp_release_v8.1.1.bin
>[13:52:39.187] - Copy mfg_bl616_gu_214e807d5_v2.43.bin
>[13:52:39.187] - Create dts for /home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/>mfg_bl616_gu_214e807d5_v2.43.bin
>[13:52:39.187] - Create dts using bl_factory_params_IoTKitA_auto.dts
>[13:52:39.191] - 4K header found,append dts file
>[13:52:39.191] - 
>Process /home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/HomeAssistant_basic_bl616.bin
>[13:52:39.191] - ========= sp image create =========
>[13:52:39.192] - Flash config crc: b'b5fec518'
>[13:52:39.192] - Clock config crc: b'4a05f490'
>[13:52:39.192] - Flash  Image
>[13:52:39.192] - Image Offset:0x1000
>[13:52:39.195] - Image hash is b'd73afdd0171dfff169513906735acc5951a96fbe5477e171e98999e837feb42c'
>[13:52:39.195] - Encrypt efuse data
>[13:52:39.196] - Image Offset:0x1000
>[13:52:39.196] - Image hash ignore,not calculate
>[13:52:39.196] - Bootheader config crc: b'9967d4ea'
>[13:52:39.197] - 
>Process /home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/boot2_bl616_isp_release_v8.1.1.bin
>[13:52:39.197] - ========= sp image create =========
>[13:52:39.197] - Flash config crc: b'b5fec518'
>[13:52:39.197] - Clock config crc: b'4a05f490'
>[13:52:39.197] - Flash  Image
>[13:52:39.197] - Image Offset:0x2000
>[13:52:39.197] - Image hash is b'fd64e376c12bbfd82fa5c503566db91617aca853bee27117bdfd7d7b5134e4d2'
>[13:52:39.197] - Encrypt efuse data
>[13:52:39.199] - Image Offset:0x2000
>[13:52:39.199] - Image hash ignore,not calculate
>[13:52:39.200] - Bootheader config crc: b'5404cf92'
>[13:52:39.200] - 
>Process /home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/mfg_bl616_gu_214e807d5_v2.43.bin
>[13:52:39.200] - ========= sp image create =========
>[13:52:39.200] - Flash config crc: b'f8de2a48'
>[13:52:39.200] - Clock config crc: b'0b34ef89'
>[13:52:39.200] - Flash  Image
>[13:52:39.200] - Image Offset:0x1000
>[13:52:39.201] - Image hash is b'd7fc31b393e3d0bc152ba9f3f917e41bb68c7df0d380cf08c77a843812495549'
>[13:52:39.202] - Encrypt efuse data
>[13:52:39.202] - Image Offset:0x1000
>[13:52:39.202] - Bootheader config crc: b'e1ebcf45'
>[13:52:39.203] - create OTA file:/home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/>HomeAssistant_basic_bl616.bin.ota
>[13:52:39.292] - create XZ file:/home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/>HomeAssistant_basic_bl616.xz
>[13:52:39.473] - create XZ OTA file:/home/seahi/GDBS/AiPi-Open-Kits/examples/HomeAssistant_example/HomeAssistant_basic/build/build_out/>HomeAssistant_basic_bl616.xz.ota
>Built target combine

