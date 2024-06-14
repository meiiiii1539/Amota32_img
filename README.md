# Amota32
一個基於ESP-IDF編寫的SDK，提供完整的ESP32本身OTA 更新以及分派的韌體更新檔下載等操作。

# Table of contents
- [1. Overview](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#1-overview)
- [2. Feature](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#2-feature)
- [3. Prerequisite](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#3-prerequisite)
    - [3.1 所需硬體](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#31-%E6%89%80%E9%9C%80%E7%A1%AC%E9%AB%94)
    - [3.2 所需軟體](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#%32-E6%89%80%E9%9C%80%E8%BB%9F%E9%AB%94)
    - [3.3 Introduction to ESP32 OTA](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#33-introduction-to-esp32-ota)
    - [3.4 Introduction to SPIFFS](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#34-introduction-to-spiffs)
- [4. Usage](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#4-usage)
    - [4.1 專案建置](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#41-%E5%B0%88%E6%A1%88%E5%BB%BA%E7%BD%AE)
    - [4.2 Command Line](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#42-command-line)
    - [4.3 API Reference](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#43-api-reference)
- [5. Appendix](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#5-appendix)
    - [5.1 使用VSCODE擴充套件](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#51-%E4%BD%BF%E7%94%A8vscode%E6%93%B4%E5%85%85%E5%A5%97%E4%BB%B6)
    - [5.2 手動安裝](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#52-%E6%89%8B%E5%8B%95%E5%AE%89%E8%A3%9D)

<br>

# 1. Overview
Amota32 基於 ESP-IDF 編寫，提供使用者以 HTTP 取得遠端韌體檔案並進行ESP32無線更新的API。此外，使用者也可以使用 Amota32 API 以相同的方式取得其他設備的韌體檔，並將其儲存在 ESP32 內部的嵌入式檔案系統中。使用者能夠使用Amota32提供的命令列介面，對ESP32 下達指令，例如讀取或刪除檔案、授權 OTA 更新等。

![overview](https://github.com/meiiiii1539/Amota32/blob/readme/image/Amota32%20(1).jpg)

<br>

# 2. Feature
- **ESP32本身OTA**<br>
- **ESP32本身OTA回滾機制**<br>
- **ESP32本身OTA自動授權更新機制**<br>
- **韌體更新檔暫存**<br>
- **韌體檔案寫入、讀取、刪除**<br>
- **命令列執行相關功能**

<br>

# 3. Prerequisite
## 3.1 所需硬體
- ESP32板。
- USB 連接線USB A / micro USB B。
- 運行 Windows、Linux 或 macOS 的電腦。
- Uart2USB Bridge
- 杜邦線

## 3.2 所需軟體

- 用於編譯 ESP32 程式碼的工具鏈 [詳見附錄安裝步驟3](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#1-%E5%AE%89%E8%A3%9Desp-idf)
- 建置工具- CMake 和 Ninja
- ESP-IDF（Espressif IoT Development Framework）
- Amota32 project


## 3.3 Introduction to ESP32 OTA

Amota32提供ESP32的OTA更新，透過HTTP資料流的方式從AUO Mesh獲取韌體，並且做ESP32自身更新，通常OTA執行過後，會需要設備重新開機使新程式生效，Amota32設計會自動執行重新開機。

此外，OTA的韌體授權有許多方式，例如需要通過用戶授權的機制，或是自動執行更新等，Amota32提供用戶授權的機制，以及使用命令列設定授權的方法。

ESP32中有內建快閃記憶體，這塊記憶體內的資訊不會因為斷電而消失，所以一些需要持久化的資料會存在這裡。ESP32 的 OTA 更新主要是在快閃記憶體SubType為OTA的分區中進行。進行 OTA 更新時，至少需要有兩個以上的 OTA 分區，假設我們只有兩個 OTA 分區，第一次執行 OTA 時，新的韌體檔案會被寫入名為 "ota_0" 的 APP 分區，並且在 "otadata" 中更新 ESP32 執行的指標位置；第二次 OTA 時，新韌體檔會被寫入名為 "ota_1" 的 APP 分區；第三次則會再次寫入 "ota_0" 中，如此循環往復，如下圖。此外，當我們從電腦燒錄程式至ESP32時，會預設燒錄在位置為0x10000的分區，不管我們在此位置設置甚麼類型分區，ESP32都會將APP燒錄在此位置。

![flash mem](https://github.com/meiiiii1539/Amota32/blob/readme/image/ota_flash_mem.jpg)

在 ESP32 的項目中，partition.csv 文件定義了不同存儲區的分區信息。表格一為預設的配置"Single factory app, no OTA"，只有一個factory partition而且沒有OTA分區，此配置是無法做OTA的；我們需要將配置更改為"Factory app, two OTA definitions"選項(表格二)，或是自行導入Partition table。
#### 表格一：預設partition table- Single factory app, no OTA

| Name | Type |SubType| Offset | Size| Flags|
|-----|-----|-----|-----|-----|-----|
| nvs| data| nvs| 0x9000|0x6000|
| phy_init| data|phy| 0xf000|0x1000|
| factory| app|factory| 0x10000|1M|
#### 表格二：Factory app, two OTA definitions
| Name | Type |SubType| Offset | Size| Flags|
|-----|-----|-----|-----|-----|-----|
| nvs| data| nvs| 0x9000|0x6000|
| phy_init| data|phy| 0xf000|0x1000|
| factory| app|factory| 0x10000|1M|
| ota_0| app|ota_0| 0x10000|1M|
| ota_1| app|ota_1| 0x210000|1M|

#### 示例：列印Auo Mesh回傳之韌體屬性資訊

Amota OTA 的觸發機制在於，當 AUO Mesh 上的韌體屬性有更新時，AUO Mesh 會使用 MQTT 通訊協定將新韌體資訊發布給 ESP32。ESP32 接收到資訊後，會根據這些資訊組合成REST API，並以HTTP下載韌體更新檔案。由於需要通過 HTTP 獲取檔案，因此在進行 OTA 前，我們需要先連接到 WiFi。此外，我們也需要連接到 AUO Mesh 的 MQTTs Broker，並訂閱相關的 topic，以便在有新韌體檔案時能夠接收到更新資訊。下方範例程式展示ESP32連接WIFI後訂閱MQTT topic，並等待MQTTs broker回傳屬性資訊。


```c
#請先去configuration 配置mqtt broker url、port、wifi...的資訊
#include "amota32.h"

struct shared_keys shared_attributes;
struct file_info File_info[4];
void app_main(void) {
    
    nvs_initialize();
    amota32_event_group = xEventGroupCreate();

    char running_partition_label[sizeof(((esp_partition_t *)0)->label)]; 
    get_running_partition_label(running_partition_label, sizeof(running_partition_label));
    initialize_wifi(running_partition_label);
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(amota32_event_group,
                                               WIFI_CONNECTED_EVENT | WIFI_DISCONNECTED_EVENT,
                                               pdTRUE,  // 清除等待的事件標誌
                                               pdFALSE, // 不需要等待所有事件
                                               portMAX_DELAY);
        if (bits & WIFI_CONNECTED_EVENT) {
            initialize_mqtt(running_partition_label);
            bits = xEventGroupWaitBits(amota32_event_group,
                                       MQTT_CONNECTED_EVENT | MQTT_DISCONNECTED_EVENT,
                                       pdTRUE,  // 清除等待的事件標誌
                                       pdFALSE, // 不需要等待所有事件
                                       portMAX_DELAY);
            if (bits & MQTT_CONNECTED_EVENT) {
                ESP_LOGI("app_main", "MQTT connected");
                connect_and_publish_firmware_info();
                bits = xEventGroupWaitBits(amota32_event_group,
                                           OTA_CONFIG_FETCHED_EVENT,
                                           pdTRUE,  // 清除等待的事件標誌
                                           pdFALSE, // 不需要等待所有事件
                                           portMAX_DELAY);

                if (bits & OTA_CONFIG_FETCHED_EVENT) {
                    ESP_LOGI("app_main", "OTA config fetched");
                    ESP_LOGI("app_main", "Firmware info: server_title=%s, version=%s", shared_attributes.target_fw_server_title, shared_attributes.target_fw_ver);
                    break; 
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 避免忙碌等待
    }
}

```
運行成功後，可以在終端機上看到AUO Mesh目前的韌體資訊(檔案名稱以及版本)，獲得這兩個資訊之後可以將其作為RESR API的參數，如下所示。

![REST API](https://github.com/meiiiii1539/Amota32/blob/readme/image/RESP%20API.jpg)

獲得完整REST API後，即可呼叫`start_ota`函數，開始執行OTA動作。`start_ota`會先確認AUO Mesh上的韌體是否適用於ESP32(可能是控制盒或LED的韌體)以及是否為新韌體版本。如果都符合條件，將會初始化HTTP Client，並進行ESP32 OTA。

在OTA成功後，將會透過UART發送成功訊息給使用者，也會將新韌體的元資訊儲存進NVS區。因此在OTA開始前，必須調用`initialize_uart`以及`initialize_nvs`進行初始化。最後，ESP32將會重新啟動。

## 3.4 Introduction to SPIFFS
Amota32的另一功能是分派的韌體更新檔下載至檔案系統。ESP32有多種檔案系統選項，例如SPIFFS、FATFS、LittleFS...等，各有不同的特色，Amota32選用的是SPIFFS檔案系統。SPIFFS 檔案系統（SPI Flash File System）是一種適用於較小的嵌入式系統的檔案系統，具有節省存儲空間和方便管理的特點，而且在沒有雲端可以儲存，又沒有SD記憶卡的情況，這時SPIFFS便可以發揮很大的作用，，但是要注意的是，不要存了太多檔案而讓SPIFFS爆掉。

如果在 ESP32 上使用 SPIFFS 檔案系統，通常需要在 partition.csv 文件中配置一個 SPIFFS 分區。表格三是Amota32的 partition.csv 配置，包含 SPIFFS 分區。
#### 表格三：Amota32 的partition table
| Name | Type |SubType| Offset | Size| Flags|
|-----|-----|-----|-----|-----|-----|
| nvs| data| nvs| 0x9000|0x4000||
| otadata| data|ota| 0xd000|0x2000||
| phy_init| data|phy| 0xf000|0x1000||
| ota_0| app|ota| 0x10000|0x150000||
| ota_1| app|ota| 0x160000|1M||
|storage|data|spiffs||0x180000||

在應用程序中，需要初始化以掛載SPIFFS檔案系統，如果是第一次掛載，系統會嘗試將SPIFFS格式化，會花比較久的時間。初始化後，可以使用標準的 C 文件操作函數在 SPIFFS 中讀寫文件。

目前SPIFFS系統不支援目錄，舉例來說，如果SPIFFS掛載在/spiffs下，裡面的檔案路徑就應該為/spiffs/temp.txt，如果路徑是/spiffs/text/temp.txt，就代表文件名稱為text/temp.txt。

下方範例程式，演練如何將AUO Mesh上的韌體以http datastreaming傳輸進ESP32，並且寫入檔案系統中。


#### 示例：以https傳輸韌體進ESP32，並且寫入檔案系統中
```c
#include "amota32.h"

/*
請先在AUO Mesh為某設備指派一個韌體，
假設上傳的韌體名稱為：example，版本為1.0。
*/
void app_main(void) {
    
    /*初始化*/
    init_nvs();
    init_spiffs(5);
    esp_http_client_handle_t amota32_client;
    amota32_event_group = xEventGroupCreate();
    
    /*組合韌體的http url*/
    char url[512];
    sprintf(url, "http://demo.thingsboard.io/api/v1/%s/firmware?title=%s&version=%s",CONFIG_MQTT_ACCESS_TOKEN,"example","1.3");

    /*初始化wifi*/
    char running_partition_label[sizeof(((esp_partition_t *)0)->label)]; 
    get_running_partition_label(running_partition_label, sizeof(running_partition_label));
    initialize_wifi(running_partition_label);

    /*
    等待wifi連線，若連線則開始讀取data streaming，並且寫入檔案系統中
    寫入的檔案位置在於/spiffs/example1.3.txt
    */
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(amota32_event_group,
                                               WIFI_CONNECTED_EVENT | WIFI_DISCONNECTED_EVENT,
                                               pdTRUE,  // 清除等待的事件標誌
                                               pdFALSE, // 不需要等待所有事件
                                               portMAX_DELAY);
        if (bits & WIFI_CONNECTED_EVENT) {
            char *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
            esp_err_t err = init_and_open_http_connection(&amota32_client, url);
            if (err != ESP_OK){
                free(buffer);
                return;
            }

            int content_length =  esp_http_client_fetch_headers(amota32_client);
            ESP_LOGE(TAG, "content length: %d", content_length);
            int total_read_len = 0, read_len;

            if (total_read_len < content_length) {
                read_len = read_datastreaming_into_file(amota32_client, buffer, MAX_HTTP_RECV_BUFFER,content_length,"example","1.3");
                if (read_len <= 0) {
                    ESP_LOGE(TAG, "Error read data: %d", read_len);
                    fclose(fp);
                    return ;
                }
                ESP_LOGD(TAG, "read_len = %d", read_len);
            }
            esp_http_client_close(amota32_client);
            esp_http_client_cleanup(amota32_client);
            free(buffer);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

```

<br>

# 4. Usage 
## 4.1 專案建置
### 專案結構

```
amota32/
├── CMakeLists.txt
├── src/
│ ├── CMakeLists.txt
│ ├── amota32.c
│ ├── auto_auth.c
│ ├── file_info.c
│ ├── file.c
│ ├── handler.c
│ ├── wifi.c
│ ├── include/
│ │ ├── amota32.h
│ │ ├── auto_auth.h
│ │ ├── file_info.h
│ │ ├── file.h
│ │ ├── handler.h
│ │ ├── wifi.h
│ │ ├── define.h
├── examples/
│ ├── CMakeList.txt
│ ├── main.c

```
我們已經將所有提供給研發人員的 API 都放置在 `src` 資料夾中，並在 `CMakeLists.txt` 中完成了相應的連結配置。使用者只需在 `main.c` 中引入`amota32.h`頭文件即可。

**STEP1. 編寫主程式：**
將主程式放置於`main.c`，範例程式可參考[這裡](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#%E7%A4%BA%E4%BE%8B%E5%88%97%E5%8D%B0auo-mesh%E5%9B%9E%E5%82%B3%E4%B9%8B%E9%9F%8C%E9%AB%94%E5%B1%AC%E6%80%A7%E8%B3%87%E8%A8%8A)

**STEP2. build項目：**
於terminal運行以下命令來編譯項目
```
idf.py build
```
或是在VSCode介面上點選 **View >Command Palette >ESP-IDF：Build your Project**

**STEP3. flash項目：**
於terminal運行以下命令將項目燒錄到ESP32上
```
idf.py flash
```
或是在VSCode介面上點選 **View >Command Palette >ESP-IDF：Flash(Uart) your Project**

**STEP4. monitor項目：**
於terminal運行以下命令將項目燒錄到ESP32上
```
idf.py monitor
```
或是在VSCode介面上點選 **View >Command Palette >ESP-IDF：Monitor your Device**


## 4.2 Command Line

在使用Command Line之前，我們需要先進行設置，詳細的設置方法請參考[附錄](#https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#3-%E9%85%8D%E7%BD%AE%E7%A1%AC%E9%AB%94)

在配置UART時，需要使用以下參數


- baud_rate : 115200
- data_bits : UART_DATA_8_BITS
- parity : UART_PARITY_DISABLE
- stop_bits : UART_STOP_BITS_1
- flow_ctrl : UART_HW_FLOWCTRL_DISABLE
- source_clk : UART_SCLK_DEFAULT,

| CLI | 說明 |
|-----|-----|
| `amota ota-update [-y]`| 執行OTA 更新。(若無輸入沒有-y，系統便會詢問是否授權)|
| `amota ota-check`| 檢查可用的OTA更新。|
| `amota esp-version` | 顯示ESP32版本。|
| `amota esp-rollback`| 回滾到另一個分區。此ESP32的OTA（Over-the-Air 更新）在兩個OTA分區（ota_0 和 ota_1）之間切換。<br>當前分區發生故障或需要回滾時，可以從當前分區切換到另一個分區，以確保系統恢復到之前的穩定狀態。|
| `amota autoauth [-y\|-N]`| 設定自動授權。一般預設為`false`，即不自動為OTA做授權，使用者在每次OTA操作時都需要做驗證|
| `amota list-files`| 列出檔案列表。|
| `amota read-file --index=<index>`|讀取特定文件內容，並以UART傳輸至serial port。您可以使用ota list-files指令查看所需讀取的index值|
| `amota delete-file --index=<index>`|刪除特定文件內容。您可以使用ota list-files指令查看所需讀取的index值|
| `amota --help` |提供命令行使用幫助|



## 4.3 API Reference

### `rollback_ota_partition(char* partition1, char* partition2)`

#### Description：

此函式用於回滾 OTA 分區，從而恢復到先前的韌體版本。

#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`partition1`|char* |第一個 OTA 分區的標籤。|
|`partition2`|char* |第二個 OTA 分區的標籤。|

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|


#### Note：
回滾後請自行restart ESP32
___

### `initialize_wifi(const char *running_partition_label)`

#### Description：

此函式用於初始化 Wi-Fi，配置 Wi-Fi 客戶端的連接、保存和恢復 Wi-Fi 憑證，並啟動 Wi-Fi 客戶端。


#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`running_partition_label`|const char *|運行中的分區標籤或名稱。|

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|

___


### `initialize_mqtt(const char *running_partition_label)`

#### Description：

此函式用於初始化 MQTT 客戶端，配置 MQTT 代理的url、port和access token，並啟動 MQTT 客戶端。

#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`running_partition_label`|const char * |運行中的分區標籤或名稱。|

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|


___


### connection_state(BaseType_t actual_event)

#### Description：

此函式用於檢查連接狀態，並根據不同的事件採取相應的操作。


#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`actual_event`|BaseType_t |一個表示當前事件的位元組，用於確定 Wi-Fi 和 MQTT 連接狀態。|



#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`STATE_WAIT_WIFI`|enum state|當 Wi-Fi 未連接時|
|`STATE_WAIT_MQTT`|enum state|當 MQTT 未連接時|
|`STATE_CONNECTION_IS_OK`|enum state|當 Wi-Fi 和 MQTT 都已連接時。|


___

### fw_params_are_specified(struct shared_keys ota_config)

#### Description：

此函式用於檢查 OTA（空中升級）配置是否已經被指定。它檢查了所提供的 OTA 配置結構體，確保其中的重要參數已經被正確設置。

#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`ota_config`|struct |OTA 配置結構體，其中包含了待更新的韌體相關訊息。|

#### Return value：


| Value |Type |Description |
|-----|-----|-----|
|True|bool|表示所有必要的參數都已經被指定。|
|False|bool|其中任何一個參數（如韌體標題或版本）尚未被指定|

___
### start_ota(const char *current_ver, struct shared_keys ota_config);

#### Description：

此函式用於初始化 OTA 流程，並進行韌體的OTA。

#### Parameters：


| Name | Type |Description |
|-----|-----|-----|
|`current_ver`| const char *|指向當前韌體版本的字串指標。|
|`ota_config`|struct |OTA 配置結構體，其中包含了待更新的韌體相關訊息。|


#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|

#### Sample Code：：

```c
#include "amota32.h"
struct shared_keys shared_attributes;
struct file_info File_info[4];
void app_main(void) {
    initialize_uart(115200,UART_DATA_8_BITS,UART_PARITY_DISABLE,UART_STOP_BITS_1,UART_HW_FLOWCTRL_DISABLE,UART_SCLK_DEFAULT);
    initialize_nvs();
    amota32_event_group = xEventGroupCreate();

    char running_partition_label[sizeof(((esp_partition_t *)0)->label)]; 
    get_running_partition_label(running_partition_label, sizeof(running_partition_label));
    initialize_wifi(running_partition_label);
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(amota32_event_group,
                                               WIFI_CONNECTED_EVENT | WIFI_DISCONNECTED_EVENT,
                                               pdTRUE,  // 清除等待的事件標誌
                                               pdFALSE, // 不需要等待所有事件
                                               portMAX_DELAY);
        if (bits & WIFI_CONNECTED_EVENT) {
            initialize_mqtt(running_partition_label);
            bits = xEventGroupWaitBits(amota32_event_group,
                                       MQTT_CONNECTED_EVENT | MQTT_DISCONNECTED_EVENT,
                                       pdTRUE,  // 清除等待的事件標誌
                                       pdFALSE, // 不需要等待所有事件
                                       portMAX_DELAY);
            if (bits & MQTT_CONNECTED_EVENT) {
                ESP_LOGI("app_main", "MQTT connected");
                connect_and_publish_firmware_info();
                bits = xEventGroupWaitBits(amota32_event_group,
                                           OTA_CONFIG_FETCHED_EVENT,
                                           pdTRUE,  // 清除等待的事件標誌
                                           pdFALSE, // 不需要等待所有事件
                                           portMAX_DELAY);

                if (bits & OTA_CONFIG_FETCHED_EVENT) {
                    ESP_LOGI("app_main", "OTA config fetched");
                    ESP_LOGI("app_main", "Firmware info: server_title=%s, version=%s", shared_attributes.target_fw_server_title, shared_attributes.target_fw_ver);

                    start_ota(CURRENT_FIRMWARE_VERSION,shared_attributes);
                    break; 
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 避免忙碌等待
    }
}
```
___

### find_file_index_to_work(struct file_info *file_info ,int index, int label);


#### Description：

此函式用於從嵌入式檔案系統中讀取指定檔案並透過Uart進行傳輸。


#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`File_info`|struct |一價結構體陣列，主要儲存檔案系統的Metadata|
|`index`|int |檔案索引，指定要讀取的檔案。|
|`label`|int |0為讀取該索引之檔案，1為刪除該索引之檔案。|

#### Return value：
No

#### Sample Code：

```c
/*
此範例假設檔案系統中index為0的位置已經儲存檔案
*/
#include "library.h"

void app_main(void){
    struct file_info File_info[4];
    nvs_initialize();
    initializespiff_s(5);
    initialize_uart(115200,UART_DATA_8_BITS,UART_PARITY_DISABLE,UART_STOP_BITS_1,UART_HW_FLOWCTRL_DISABLE,UART_SCLK_DEFAULT);

    int label = 0 ; //做read的動作
    int index = 0 ; //對index=0的檔案
    find_file_index_to_work(File_info ,index, label); 

    
}
```
___

### start_file_sys(const char *current_ver, struct shared_keys device_file_config)

#### Description：
此函式用於從remote server下載file並將其寫入file system中。

#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`current_ver`|const char * |指向當前韌體版本的字串指標。|
|`device_file_config`|struct |包含檔案系統相關配置的結構體。|

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|


___

### initialize_uart(int baud_rate, uart_word_length_t data_bits, uart_parity_t parity, uart_stop_bits_t stop_bits, uart_hw_flowcontrol_t flow_ctrl, int intr_alloc_flags)


#### Description：

此函式用於初始化 UART 通訊端口的相關設置，包括波特率、資料位元、奇偶校驗、停止位元、流控制和中斷配置。

| Name | Type |Description |
|-----|-----|-----|
|`baud_rate`|int |波特率，指定 UART 通訊速率。|
|`data_bits`| uart_word_length_t|資料位元，指定每個 UART 傳輸的資料位元數。|
|`parity`| uart_parity_t|奇偶校驗，指定是否啟用奇偶校驗以及校驗位元的類型。|
|`stop_bits`|uart_stop_bits_t |停止位元，指定每個 UART 傳輸的停止位元數。|
|`flow_ctrl`|uart_hw_flowcontrol_t |流控制，指定是否啟用硬體流控制。|
|`intr_alloc_flags`| int|中斷分配標誌，指定分配給 UART 中斷的標誌。|



#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|
___

### initialize_nvs()


#### Description：
此函式用於初始化NVS系統，用於在ESP32上存持久性數據。

#### Parameters：
No


#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|
___

### initialize_spiffs(size_t max_file_num)

#### Description：
此函式用於初始化 SPIFFS 檔案系統，以在嵌入式設備上提供基於 Flash 的檔案存儲。


#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`max_file_num`|size_t |最大檔案數量，指定檔案系統中能夠容納的最大檔案數量。|

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|


___

### initialize_cli(char* main_command, size_t cmdline_max_num, size_t cmdline_max_length)
 

#### Description：
此函式用於初始化控制台，允許用戶在設備上輸入命令。

#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`main_command`|char* |主命令|
|`cmdline_max_num`|size_t |最大命令行參數數量|
|`cmdline_max_length`|size_t |最大命令行長度|

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|

___

### nvs_operation_after_booting()

#### Description：
此函式用於在設備啟動後執行 NVS操作，獲取auto_auth和Files_info。


#### Parameters：
No


___

### save_file_info(struct file_info *files_info, int label)

#### Description：
將檔案訊息保存到非易失性存儲（NVS）中。


#### Parameters：
No

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|

___

### add_file_info(struct file_info *files, size_t *head, size_t *tail, size_t max_count, const char *name, const char *ver, uint32_t len)

#### Description：
在files這個結構陣列中添加新的file_info。

#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`files`|struct |structure array的指針。|
|`head`|size_t * |structure array的頭指針，用於circular儲存。|
|`tail`|size_t * |structure array的尾指針，用於circular儲存。|
|`max_count`|size_t |structure array的最大容量。|
|`name`|const char * |刪除的file_info檔名。|
|`ver`|const char * |新加入的file_info版本。|
|`len`|uint32_t |新加入的file_info長度。|

#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|
___

### delete_file_info(struct file_info *files, size_t *head, size_t *tail, size_t max_count, const char *name, const char *version)

#### Description：

在files這個結構陣列中刪除指定的file_info。


#### Parameters：

| Name | Type |Description |
|-----|-----|-----|
|`files`|struct |structure array的指針。|
|`head`|size_t * |structure array的頭指針，用於circular儲存。|
|`tail`|size_t * |structure array的尾指針，用於circular儲存。|
|`max_count`| size_t|structure array的最大容量。|
|`name`| const char *|刪除的file_info檔名。|
|`ver`| const char *|新加入的file_info版本。|
|`len`|uint32_t |新加入的file_info長度。|


#### Return value：

| Value |Type |Description |
|-----|-----|-----|
|`ESP_OK`|esp_err_t|ok|

<br>

# 5. Appendix
有兩種方法可以安裝所有必須軟體：
- 整合開發環境的擴充套件(推薦)
- 手動安裝

![needed software](https://github.com/meiiiii1539/Amota32/blob/imgFolder/image/needed_software.png)
## 5.1 使用VSCODE擴充套件
### 1. 安裝ESP-IDF

**STEP1. 下載擴充套件** 
    
![one](https://github.com/meiiiii1539/Amota32/blob/imgFolder/image/download_extension_1.png)

**STEP2. 點選View >Command Palette，並輸入 configure esp-idf extension**     
    
![two](https://github.com/meiiiii1539/Amota32/blob/imgFolder/image/configure_extension_2.png)

**STEP3. 點選Express，並選擇IDF下載來源、下載位置、toolpath位置(圖上皆為預設參數)**     

![three](https://github.com/meiiiii1539/Amota32/blob/imgFolder/image/download_idf_setting_3.png)

**STEP4. 開始下載**    
您將看到一個顯示設置進度狀態的頁面，其中包括 ESP-IDF 下載進度、ESP-IDF 工具的下載和安裝進度，以及 Python 虛擬環境的建立過程。請耐心等待一段時間。
    
![four](https://github.com/meiiiii1539/Amota32/blob/imgFolder/image/wait_for%20download_4.png)

### 2. 導入 Amota32 專案

**STEP1. 設定目標裝置**

點選View >Command Palette >ESP-IDF：Set Espressif Device Target
    
![six](https://github.com/meiiiii1539/Amota32/blob/imgFolder/image/set_target_6.png)

**STEP2. SDK配置** 

點選**View >Command Palette >ESP-IDF：SDK Configuration editor**
    
在此可以設定WiFi帳密、MQTTs Broker URL、Broker Port Number等等

![Untitled](https://github.com/meiiiii1539/Amota32/blob/imgFolder/image/sdk_conf_7-1.png)

![Untitled](https://github.com/meiiiii1539/Amota32/blob/readme/image/auo_mesh_ota_conf_7-2.png)

**STEP3. 編譯器配置** 

`.vscode/c_cpp_properties.json`進行配置

請參考[這裡](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/C_CPP_CONFIGURATION.md)

### 3. 配置硬體
在[導入Amota專案 Step2：SDK配置](https://github.com/meiiiii1539/Amota32/tree/readme?tab=readme-ov-file#2-%E5%B0%8E%E5%85%A5-amota32-%E5%B0%88%E6%A1%88)時，我們將UART TXD與RXD分別設為4和5，這代表ESP32由UART GPIO 4 輸出數據，並由UART GPIO 5 接收數據。因此我們需要使用杜邦線連接ESP32的GRIO 4與UART2USB bridge的RXD，還有連接ESP32的GRIO 5與UART2USB bridge的TXD，如下圖所示，這樣用戶就可以透過序列埠傳送命令給ESP32，ESP32可以此輸出資料。

![HW_安裝](https://github.com/meiiiii1539/Amota32/blob/readme/image/hw_install.jpg)


## 5.2 手動安裝
對於手動過程，請根據您的作業系統進行選擇。
- [Windows](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html)
- [Linux/MacOs](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html)

<br>




