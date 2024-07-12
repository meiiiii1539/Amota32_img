# Amota32


Amota32 (**A**uo **M**esh **OTA** for ESP**32**) 是一個在 ESP32 上協助使用者利用 AUO MESH OTA 功能的 SDK，採用 C 語言並搭配 Espressif IoT Development Framework ([ESP-IDF](https://github.com/espressif/esp-idf)) 的功能編寫而成。使用者可以用它來：

- 接收韌體更新通知
- 下載更新檔
- 完成 ESP32 本身 OTA 更新
- 暫存分派給其他設備的更新韌體檔

<br />

# Table of contents

- [Amota32](#amota32)
- [Table of contents](#table-of-contents)
- [1. Overview](#1-overview)
- [2. Feature](#2-feature)
- [3. Prerequisite](#3-prerequisite)
  - [3.1 所需硬體](#31-所需硬體)
  - [3.2 所需軟體](#32-所需軟體)
  - [3.3 ESP32 OTA 介紹](#33-esp32-ota-介紹)
    - [(1) A/B Partition Scheme](#1-ab-partition-scheme)
    - [(2) 非 OTA 更新(有線更新)](#2-非-ota-更新有線更新)
    - [(3) 磁碟分區設定](#3-磁碟分區設定)
  - [3.4 SPIFFS 介紹](#34-spiffs-介紹)
- [4. Usage](#4-usage)
  - [4.1 Configuration](#41-configuration)
    - [(1) SDK Configuration](#1-sdk-configuration)
    - [(2) PC 與 ESP32 建立連接](#2-pc-與-esp32-建立連接)
    - [(3) UART Configuration](#3-uart-configuration)
      - [連接USB-to-UART Bridge](#連接usb-to-uart-bridge)
      - [初始化UART](#初始化uart)
  - [4.2 專案建置](#42-專案建置)
    - [專案結構](#專案結構)
  - [4.3 API Reference](#43-api-reference)
    - [(1) 使用流程](#1-使用流程)
    - [流程圖：](#流程圖)
    - [(2) 自動授權旗幟](#2-自動授權旗幟)
    - [(3) 結構體定義](#3-結構體定義)
    - [韌體資訊結構體](#韌體資訊結構體)
    - [初始化結構體](#初始化結構體)
    - [(4) Callback Functions](#4-callback-functions)
      - [Examples](#examples)
    - [(5) APIs](#5-apis)
    - [void amota\_task(void \*pvParameters);](#void-amota_taskvoid-pvparameters)
    - [void amota\_cli\_uart\_task(void \*pvParameters)](#void-amota_cli_uart_taskvoid-pvparameters)
    - [esp\_err\_t start\_esp32\_ota\_update(const char \*current\_ver, struct FW\_INFO esp\_fw\_info, amota\_callbacks\_t \*callbacks)](#esp_err_t-start_esp32_ota_updateconst-char-current_ver-struct-fw_info-esp_fw_info-amota_callbacks_t-callbacks)
    - [esp\_err\_t start\_fw\_downloading(const char \*current\_ver, struct FW\_INFO extra\_fw\_info, amota\_callbacks\_t \*callbacks)](#esp_err_t-start_fw_downloadingconst-char-current_ver-struct-fw_info-extra_fw_info-amota_callbacks_t-callbacks)
    - [esp\_err\_t amota32\_initialize(init\_error\_callback\_t init\_error\_handle)](#esp_err_t-amota32_initializeinit_error_callback_t-init_error_handle)
  - [4.4 Command Line Interface](#44-command-line-interface)
- [5. Appendix](#5-appendix)
  - [5.1 安裝 ESP-IDF：使用 VSCODE 擴充套件](#51-安裝-esp-idf使用-vscode-擴充套件)
  - [5.2 安裝 ESP-IDF：手動安裝](#52-安裝-esp-idf手動安裝)
  - [5.3. 配置硬體](#53-配置硬體)
      - [簡單示意圖](#簡單示意圖)

<a name="Overview"></a>
<br />

# 1. Overview

Amota32 library 提供一系列 API，可以讓使用者在授權更新之後完成 ESP32 的 OTA，以及控制器的分派檔案下載。Amota32 亦提供多種更新授權方式，例如：手動授權或自動授權。使用者可以通過命令列介面(CLI)設定授權方法，確保更新過程的安全性和靈活性。

Amota32 OTA 和檔案下載的觸發機制基於 AUO MESH 上是否有被上傳新韌體。當 AUO MESH 上的韌體被更新時，平台會使用 MQTT 傳達新韌體的檔案資訊，而 ESP32 在接收到韌體資訊之後，再根據這些資訊組合成 REST API 請求，並在用戶授權更新後執行OTA或是下載檔案，如下圖左半部所示。此外，使用者也能夠使用 Amota32 CLI，透過命令列對 ESP32 下達其他指令，例如讀取、刪除檔案等，如下圖右半部所示。

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/Amota32%20(1).jpg" alt="overview" style="max-width: 100%; height: auto;" alien="center" />

上述流程皆封裝在 Amota32 提供的 API 中，使用者只須啟動相關任務，並且在獲取韌體許可通知時呼叫執行 OTA 或檔案下載的 API，即可獲得所需功能。

<br />
<br />

<a name="Feature"></a>

# 2. Feature

- ESP32 本身 OTA
- ESP32 本身 OTA 回滾機制
- ESP32 本身 OTA 自動授權更新機制
- 額外的韌體更新檔暫存
- 額外的韌體檔案寫入、讀取、刪除
- 提供串列埠遠端控制指令，使用者可在上位機通過命令列執行相關功能

<a name="Prerequisite"></a>
<br />

# 3. Prerequisite

<a name="Hardware"></a>

## 3.1 所需硬體

- ESP32 開發板
- USB Type-A 對 USB Micro-B 傳輸線
- 開發環境主機 (Windows、Linux 或 macOS)
- USB-to-UART Bridge
- 杜邦線

<a name="Toolchain"></a>
<br />

## 3.2 所需軟體

- 用於編譯 ESP32 程式碼的工具鏈 [詳見附錄安裝步驟 3](#VSCODE_extension)
- 建置工具 - CMake 和 Ninja
- ESP-IDF
- Amota32 library (本專案)

<a name="ESP32_OTA"></a>
<br />

## 3.3 ESP32 OTA 介紹

<a name="AB_Partition_Scheme"></a>

### (1) A/B Partition Scheme

微控器韌體常見的更新策略是採取 A/B 分區切換方法 (A/B Partition Scheme) 來完成，ESP32 的 OTA 更新亦是採用相同策略，因此快閃記憶體必須準備至少兩個應用系統分區，如下圖中的`ota_0`及`ota_1`。

當第一次執行 OTA 時，新的韌體將被寫入分區 `ota_0` 並自動更新狀態維護分區 `otadata` 中的下次開機啟動指標位置。當進行第二次 OTA 時，新韌體會被寫入分區 `ota_1`；同理，第三次則會寫進分區 `ota_0`。接續的更新皆按上述規則，如此循環。

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/ota_flash_mem.jpg" alt="flash mem" style="width: 100%; height: auto;" />
<br />

<a name="Non_OTA_update"></a>
<br />

### (2) 非 OTA 更新(有線更新)

當我們從電腦燒錄程式至 ESP32 時，預設皆是燒錄到起始位址 `0x10000` 所指向的分區(如上圖 `factory` 分區)。

<a name="Partition_CSV"></a>
<br />

### (3) 磁碟分區設定

ESP32 的應用程式專案使用 `partition.csv` 文件來儲存分區定義的資訊。表 1 顯示了預設配置(_Single factory app, no OTA_)的資訊，只有一個 factory 分區且沒有 OTA 分區，因此這樣的配置是無法支援 OTA 的。

<br />

**表 1、預設 partition table- Single factory app, no OTA**

| Name     | Type | SubType | Offset  | Size   | Flags |
| -------- | ---- | ------- | ------- | ------ | ----- |
| nvs      | data | nvs     | 0x9000  | 0x6000 |
| phy_init | data | phy     | 0xf000  | 0x1000 |
| factory  | app  | factory | 0x10000 | 1M     |

<br />

要讓 ESP32 支援 OTA 更新，我們需要將分區修改如表 2 的 _Factory app, two OTA definitions_ 配置，或是自行導入 Partition table。

<br />

**表 2、Factory app, two OTA definitions**

| Name     | Type | SubType | Offset   | Size   | Flags |
| -------- | ---- | ------- | -------- | ------ | ----- |
| nvs      | data | nvs     | 0x9000   | 0x6000 |
| phy_init | data | phy     | 0xf000   | 0x1000 |
| factory  | app  | factory | 0x10000  | 1M     |
| ota_0    | app  | ota_0   | 0x10000  | 1M     |
| ota_1    | app  | ota_1   | 0x210000 | 1M     |

<br />

<a name="SPIFFS"></a>
<br />

## 3.4 SPIFFS 介紹

Amota32 的另一功能是可以將額外的韌體更新檔下載至檔案系統內儲存，以作為其他控制器的韌體檔分派來源。本小節對 Amota32 使用的檔案系統 SPIFFS (SPI Flash File System) 作一簡單介紹。

SPIFFS 是一種適用於小型嵌入式系統的檔案系統，具有節省儲存空間和方便管理的特點。要在 ESP32 上使用 SPIFFS 檔案系統，需要在 `partition.csv` 文件中配置一個 SPIFFS 分區，如表 3 所列。

<br />

**表 3、Amota32 的 partition table**
| Name | Type | SubType | Offset | Size | Flags |
| -------- | ---- | ------- | -------- | -------- | ----- |
| nvs | data | nvs | 0x9000 | 0x4000 | |
| otadata | data | ota | 0xd000 | 0x2000 | |
| phy_init | data | phy | 0xf000 | 0x1000 | |
| ota_0 | app | ota | 0x10000 | 0x150000 | |
| ota_1 | app | ota | 0x160000 | 1M | |
| storage | data | spiffs | | 0x180000 | |

<br />

在應用程式的初始化階段，必須將 SPIFFS 檔案系統掛載起來。如果是第一次掛載，系統會嘗試將 SPIFFS 格式化，因此可能需要等待較長的時間。一旦初始化完成，爾後便可以通過標準的 C 語言檔案操作函數在 SPIFFS 中讀寫檔案。

目前 SPIFFS 系統不支援目錄。舉例來說，如果 SPIFFS 掛載在 `/spiffs` 下，裡面的檔案路徑就應該為 `/spiffs/temp.txt`；如果路徑是 `/spiffs/text/temp.txt`，則表示檔案名稱為 `text/temp.txt`。

<br />
<a name="Usage"></a>
<br />

# 4. Usage

Amota32 係基於 ESP-IDF 編寫而成，關於開發環境的完整安裝流程請見[附錄](#Appendix)。

<a name="Configuration"></a>


## 4.1 Configuration

### (1) SDK Configuration

<br />

**STEP 1: 設定目標裝置**

- 點選 View >Command Palette >ESP-IDF：Set Espressif Device Target

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/set_target_6.png" alt="six" style="width: 600px; height: auto;" />

<br />
<a name="SDK_config"></a>

<br />

**STEP 2: SDK 配置**

- 點選 View >Command Palette >ESP-IDF：SDK Configuration editor
- 在此可以設定 WiFi ssid, WiFi password, MQTTs broker url、broker port number 等等

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/sdk_conf_7-1.png" alt="Image" style="width: 600px; height: auto;" />
<br/>
<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/auo_mesh_ota_conf_7-2.png" alt="Image" style="width: 600px; height: auto;" />

<br />
<br />

**STEP3. 編譯器配置**

- 在 `.vscode/c_cpp_properties.json` 文件進行配置
- 詳情請參考[ESP-IDF 官方文件](https://github.com/espressif/vscode-esp-idf-extension/raw/master/docs/C_CPP_CONFIGURATION.md)說明

<br />

<a name="ESP32_and_PC_Connection"></a>
<br />


### (2) PC 與 ESP32 建立連接
<br />
用 Micro USB 線將 ESP32 開發版連接到電腦(如下圖所示)，正常情況下 USB Driver 將會自動安裝；若無自動安裝請自行確認 ESP 開發版型號所對應的 USB Driver，並上網自行安裝。 

<br />
<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/esp32_connect.png" alt="esp32 connect" style="width: 600px; height: auto;" />

<br />
<br />
連接到電腦後，檢查Windows裝置管理員中的COM列表，可以嘗試斷開再重新接上ESP32，便可以在設備管理器上確認ESP32使用的是哪一個COM，如下圖所示。<br />

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/device_manager.png" alt="com" style="width: 600px; height: auto;" />

<br />
<br />
確認ESP32使用的串列端口之後，可以使用PuTTy SSH Client工具驗證串口連接是否成功，需要在PuTTy SSH Client中輸入下方通訊參數：

    波特率 = 115200，數據位 = 8，停止位 = 1，奇偶校驗 = N
<br />
接著在終端機中開啟串口，觀察是否有顯示任何資訊，如果顯示的是人類可讀訊息(而非亂碼)則表示連接成功。

<a name="UART_Configuration"></a>
<br />

### (3) UART Configuration

在 ESP32 與 PC 連線成功後，您將能夠在串口監視器上看到 Amota32 的 LOG 輸出。如果需要與 ESP32 進行互動，則可以使用 Amota32 提供的命令列介面 (CLI)。Amota32 CLI 允許主機（Host machine）通過遠端命令執行 OTA 相關操作以及遠端調試。
<br />

#### 連接USB-to-UART Bridge
在使用 Amota32 CLI 之前，需要連接設備並啟用對應的 UART 串列埠。以下是設備連接的設置步驟：

  1. USB-to-UART Bridge 透過 USB 介面連接到電腦 (用於串列資料傳輸)。
  2. ESP32 和 USB-to-UART Bridge 之間透過杜邦線進行連接。
<br />

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/UART-to-USB.png" alt="uart-to-usb" style="width: 600px; height: auto;" />
<br />

詳細的腳位連接請參考[附錄](#Hardware_detail)
<br />

#### 初始化UART
在設置完硬體連接後，我們需要在 ESP32 上使用 `initialize_uart()` 初始化 UART。初始化 UART 是為了確保 ESP32 能夠正確的發送和接收串列資料，從而與 USB-to-UART Bridge 進行通訊。在初始化 UART 時，需要傳入以下參數：

- `baud_rate`: `115200`
  <!-- - 即每秒傳輸的bit數。115200 是常用的波特率。 -->
- `data_bits` : `UART_DATA_8_BITS`
  <!-- - 即每個字節的位元數。8 位元是標準配置，能夠滿足大多數通訊需求。 -->
- `parity` : `UART_PARITY_DISABLE`
  <!-- - 奇偶校驗，用於錯誤檢測。我們設置為不使用奇偶校驗。 -->
- `stop_bits` : `UART_STOP_BITS_1`
  <!-- - 停止位，表示一個數據包的結尾。 -->
- `flow_ctrl` : `UART_HW_FLOWCTRL_DISABLE`
  <!-- - 流量控制，用於管理資料流的速度。這裡設置為不使用硬體流量控制。 -->
- `source_clk` : `UART_SCLK_DEFAULT`
  <!-- - 時鐘源，用於提供 UART 所需的時鐘信號。設置為默認時鐘源。 -->

連接設置完成後，使用者便可以透過 Amota32 CLI 輸入命令，並透過 USB-to-UART Bridge 將命令發送到 ESP32。ESP32 在接收命令後執行相應的操作。


<a name="Build_Project"></a>
<br />

## 4.2 專案建置

### 專案結構

```
amota32/
├── CMakeLists.txt
├── partition.csv
├── src/
│ ├── CMakeLists.txt
│ ├── config.json
│ ├── amota32.c
│ ├── wifi.c
│ ├── mqtt.c
│ ├── ota.c
│ ├── file.c
│ ├── uart.c
│ ├── initialize.c
│ ├── nvs_authflag.c
│ ├── nvs_fileinfo.c
│ ├── global_variable.c
│ ├── include/
│ │ ├── amota32.h
│ │ ├── mqtt.h
│ │ ├── ota.h
│ │ ├── file.h
│ │ ├── uart.h
│ │ ├── initialize.h
│ │ ├── nvs_authflag.h
│ │ ├── nvs_fileinfo.h
│ │ ├── global_variable.h
├── examples/
│ ├── CMakeList.txt
│ ├── main.c

```

`src` 目錄下的原始碼係 Amota32 的功能實作，而 `CMakeLists.txt` 指示了相應的編譯期連結配置。使用者只需將 Amota32 放置到應用專案的適當位置，並在應用入口 `main.c` 中引入 `amota32.h` 頭文件即可以使用 Amota32 提供的各項功能。以下說明大致的使用步驟：

<br />

**STEP 1: 編寫主程式**

編寫主程式 `main.c`。

<br />

**STEP 2: Build 專案**

於 terminal 執行以下命令進行編譯

```sh
idf.py build
```

或是在 VSCode 介面上點選 _View >Command Palette >ESP-IDF：Build your Project_ 亦可。

<br />

**STEP 3: 燒錄韌體**

於 terminal 執行以下命令將專案建置完後的二進位映像檔燒錄到 ESP32

```sh
idf.py flash
```

或是在 VSCode 介面上點選 **View >Command Palette >ESP-IDF：Flash(Uart) your Project** 亦可。

<br />

**STEP 4: 監看應用程式 log**

在燒錄完畢後，可於 terminal 執行以下命令，監看應用程式的 log 輸出：

```
idf.py monitor
```

或是在 VSCode 介面上點選 **View >Command Palette >ESP-IDF：Monitor your Device**

<br />
<a name="APIs"></a>
<br />

## 4.3 API Reference

### (1) 使用流程
### 流程圖：
<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/swim_lane.jpg" alt="swim_lane" style="width: 100%; height: auto;" />


Amota32 提供一支 FreeRTOS 任務 [amota_task()](#amota_task) 來執行 OTA 管理，當此任務接收到 AUO MESH 傳來的韌體變更資訊後便會發佈通知，應用程式在收到通知後可以呼叫 [start_esp32_ota_update](#start_esp32_ota_update) 來執行 ESP32 本身的 OTA 更新。此外，該任務也負責下載額外的韌體更新檔，當應用程式接收到下載許可的通知後，可以呼叫 [start_fw_downloading()](#start_fw_downloading()) 將額外的韌體檔案下載至 ESP32 的檔案系統中。

為了方便上位機(例如另一控制器)通過命令行介面(CLI)來使用 OTA 的相關功能，Amota32 提供了另一個任務 [amota32_cli_uart_task()](#amota32_cli_uart_task) 負責建立 CLI 功能。如此，上位機便可以透過 UART 來進行讀取或刪除檔案、授權 OTA 更新等操作。

<br />

### (2) 自動授權旗幟

Amota32 定義了一個名為`auth_flag`的旗標來判斷是否自動授權更新。一般來說，`auth_flag`的預設為 `false`，表示用戶需要透過CLI來授權更新，這時使用者可以利用CLI更改設定為自動授權更新，爾後便不須在每次更新時做授權。

<br />

### (3) 結構體定義

### 韌體資訊結構體

Amota32定義了一個結構體類型`FW_INFO`，此類型會有兩個實體，一個為`auo_mesh_fw_attributes`用來儲存 AUO MESH 傳來的新韌體資訊；另一個為`file_info`，它被宣告成一個結構體陣列，用來儲存檔案系統內的檔案資訊。

```c
struct FW_INFO
{
    char fw_name[256]; //韌體名稱
    char fw_ver[128]; //韌體版本
    uint32_t fw_size; //韌體大小
}

struct FW_INFO auo_mesh_fw_attributes;
struct FW_INFO File_info[4];

```

### 初始化結構體

Amota32亦定義了另一個結構體類型`config_t`用來儲存初始化UART、NVS、Spiffs等的所有參數。使用者可以至`config.json`中更改所需的配置，程式會在`amota_initialize()`中將JSON檔內的資訊賦值到結構體實體中。
```c
typedef struct {
    char base_path[32];
    char partition_label[32];
    size_t max_files;
    bool format_if_mount_failed;
    int baud_rate;
    int data_bits;
    int parity;
    int stop_bits;
    int flow_ctrl;
    int rx_flow_ctrl_thresh;
    char main_cmd[5];
    size_t max_cmdline_length;  
    size_t max_cmdline_args;    
    uint32_t heap_alloc_caps;   
    int hint_color;             
    int hint_bold;
} config_t;

```

<br />

<a name="callback"></a>

### (4) Callback Functions

下方表格所列為Amota32 SDK 所提供的 callback functions 介面。

**表 4、Callback function功能介紹表**
| Name | Arguments | Function |
| -------- | ---- | ------- | 
| wifi_on_connect | (void) | 處理設備成功連接到WiFi網路時的操作 |
| wifi_error_callback | (int) `error_code`, (char *)`msg` | 處理WiFi連接過程中發生的錯誤，根據接收到的錯誤代碼和錯誤消息採取適當處理。 | 
| mqtt_on_connect | (void) | 處理設備成功連接到MQTT時的操作 | 
| mqtt_error_callback | (int) `error_code`, (char *)`msg` | 處理MQTT連接過程中發生的錯誤以及斷連後的操作，根據接收到的錯誤代碼和錯誤消息採取適當處理。 |
| mqtt_get_fw_info | (struct `FW_INFO`) `receive_info` | 處理設備從MQTT成功取得韌體資訊後的操作。取得韌體資訊後可做的後續操作有OTA更新、下載韌體。 | 
| exfw_error_callback | (int) `error_code`, (char *)`msg` | 處理檔案下載過程中發生的錯誤，根據接收到的錯誤代碼和錯誤消息採取適當處理。 |
| exfw_download_implementation | (esp_http_client_handle_t) `fw_download_client` | 處理接收額外韌體檔案的操作，預設處理是儲存進檔案系統。若有其他需求，可以在此函數中實作。 |
| init_error_callback | (int) `error_code`, (char) *`msg` | 處理amota32初始化過程中發生的錯誤以及斷連後的操作，根據接收到的錯誤代碼和錯誤消息採取適當處理。 |
| fw_ver_equal | (const char *) `current_esp_fw_ver`, (const char *)`target_ver` | 處理更新韌體檔的版本與現有韌體相同的操作。 |
| after_ota_finish | (void) | 處理ESP32 OTA更新成功後的的操作，預設處理是重新開機以切換分區。如果有其他需求，可以在此函數中實作。 |
| after_ota_fail | (int) `error_code`, (char *)`msg` | 處理OTA更新過程中發生的錯誤，根據接收到的錯誤代碼和錯誤消息採取適當處理。 |
| fw_params_no_specified | (struct `FW_INFO`) `receive_info` | 處理韌體資訊缺失的操作。 |



Amota32提供了 `amota_callbacks_t`結構體類型用來儲存各種callback function，使用者須於 `app_main` 函數中定義一個 `amota_callbacks_t` 的實體，並將callback function賦值給實體的各個字段(如下方範例所列)，以便在程式中的不同部分使用這些callback functions。其餘無實作的callback functions預設為NULL，不影響程式運行。

#### Examples

```c
//實作callback
void wifi_connect() {
    printf("Wi-Fi connected successfully!\n");
}
void mqtt_connect() {
    printf("MQTT connected successfully!\n");
}

void app_main(void) {
    //定義結構體實體
    amota_callbacks_t callbacks = {
        .wifi_on_connect = wifi_connect,
        .mqtt_on_connect = mqtt_connect
        //其餘無實作的callback預設為NULL
    };
}

```
<br>

### (5) APIs
<a name="amota_task"></a>

### void amota_task(void *pvParameters);

這是一支 FreeRTOS 任務，用來管理 OTA 更新及檔案系統下載的流程。要使用 Amota32 提供的 OTA 功能，您必須在應用程式啟動時建立該任務的實例。此任務透過不同的狀態來管理 Wi-Fi 連接、MQTT 連接，以及接收和處理韌體檔案資訊。

**Arguments:**

`pvParameter`: 創建任務時傳遞的callback structure

**Returns:**

- (void)

**Examples:**

```C
//建立amota32_task任務實例

event_group = xEventGroupCreate();

amota_callbacks_t callbacks = {
        .wifi_on_connect = wifi_connect,
    };

//第三個參數是給 task  的 stack size = 8,192*4 = 32,768 bytes // 第五個是 task priority
xTaskCreate(&amota_task, "amota_task", 8192, (void*)&callbacks, 5, NULL);
```

<br />

---

<a name="amota_cli_uart_task"></a>

### void amota_cli_uart_task(void *pvParameters)

這是一支 FreeRTOS 任務，它的作用是為 Amota32 建立命令行介面。它會監聽 UART 的接收數據，當收到`\n`(換行符)時，會將接收到的數據傳給命令行解釋器。

**Arguments:**

`pvParameter`: 創建任務時傳遞的callbacks structure

**Returns:**

- (void)

**Examples:**

```C
//建立amota_cli_uart_task任務實例，以建立CLI。
event_group = xEventGroupCreate();
amota_callbacks_t callbacks = {
        .wifi_on_connect = wifi_connect,
    };
xTaskCreate(&amota_cli_uart_task, "amota_cli_uart_task", 10000, (void*)&callbacks, 10, NULL);
```

<br />

---

<a name="start_esp32_ota_update"></a>

### esp_err_t start_esp32_ota_update(const char \*current_ver, struct FW_INFO esp_fw_info, amota_callbacks_t *callbacks)

啟動 ESP32 OTA 更新流程。

**Arguments:**

`current_ver`: 指向當前韌體版本字串的指標。
`esp_fw_info`: OTA 配置結構體，其中包含了待更新韌體的相關訊息。
`callbacks`: 儲存callback functions的結構體

**Returns:**

`ESP_OK` (esp_err_t): ok

**Examples:**

**方法1：自行創建任務循環等待 OTA_UPDATE_AVAILABLE_BIT**
```C
//呼叫start_esp32_ota_update，以實際運行ESP32的OTA
void wait_ota_update_bit(void *pvParameter){
  while (1) {
    //等待來自amota_task的事件
    EventBits_t event_bits = xEventGroupWaitBits(ota_event_group,
                                                    OTA_UPDATE_AVAILABLE_BIT,
                                                    pdTRUE,
                                                    pdFALSE,
                                                    portMAX_DELAY);
    if (event_bits & OTA_UPDATE_AVAILABLE_BIT) {
       //收到這個event bit之後，即可呼叫 做start_esp32_ota_update OTA更新
        start_esp32_ota_update(current_ver, auo_mesh_fw_attributes, NULL);
    }
  }
}

xTaskCreate(&wait_ota_update_bit, "wait_ota_update_bit", 10000, NULL, 10, NULL);

```
**方法2：實作`mqtt_get_fw_info` callback function**

```c
//實作callback
void fw_info_received(struct FW_INFO fw_info) {
    printf("Received firmware information:\n");
    printf("Firmware Name: %s\n", fw_info.fw_name);
    printf("Firmware Version: %s\n", fw_info.fw_ver);
    //呼叫API
    start_esp32_ota_update(current_ver, auo_mesh_fw_attributes, NULL);
}

void app_main(void) {
    //定義結構體實體
    amota_callbacks_t callbacks = {
         .mqtt_get_fw_info= fw_info_received,
    };
     if (amota32_initialize(NULL) == ESP_OK) {
        amota32_event_group = xEventGroupCreate(); //創建事件組叫做amota32_event_group
        xTaskCreate(&amota_task, "amota_task", 8192, (void *)&callbacks, 10, NULL)
    }
}

```
<br />

---

<a name="start_fw_downloading"></a>

### esp_err_t start_fw_downloading(const char \*current_ver, struct FW_INFO extra_fw_info, amota_callbacks_t *callbacks)

從 remote server 下載韌體檔案並將其寫入本地檔案系統。

**Arguments:**

`current_ver`: 指向當前韌體版本字串的指標。
`extra_fw_info`: 包含分派設備韌體檔的資訊之結構體。
`callbacks`: 儲存callback functions的結構體

**Returns:**

`ESP_OK` (esp_err_t): ok

**Examples:**

**方法1：自行創建任務循環等待 FILE_SYS_AVAILABLE_BIT**
```C
//呼叫start_fw_downloading，將額外的韌體檔案下載至檔案系統中

void wait_fw_update_bit(void *pvParameter){
  while (1) {
    //等待來自amotacli_uart_task的事件
    EventBits_t event_bits = xEventGroupWaitBits(amota32_event_group,
                                                    FILE_SYS_AVAILABLE_BIT,
                                                    pdTRUE,
                                                    pdFALSE,
                                                    portMAX_DELAY);

    if (event_bits & FILE_SYS_AVAILABLE_BIT) {
       //收到這個event bit之後，即可做檔案系統下載
        start_fw_downloading(current_ver, auo_mesh_fw_attributes);
    }
  }
}

xTaskCreate(&wait_fw_update_bit, "wait_fw_update_bit", 10000, NULL , 10, NULL);

```

**方法2：實作`mqtt_get_fw_info` callback function**
```c
//實作callback
void fw_info_received(struct FW_INFO fw_info) {
    printf("Received firmware information:\n");
    printf("Firmware Name: %s\n", fw_info.fw_name);
    printf("Firmware Version: %s\n", fw_info.fw_ver);
    //呼叫API
    start_esp32_ota_update(current_ver, auo_mesh_fw_attributes, NULL);
}

void app_main(void) {
    //定義結構體實體
    amota_callbacks_t callbacks = {
         .mqtt_get_fw_info= fw_info_received,
    };
     if (amota32_initialize(NULL) == ESP_OK) {
        amota32_event_group = xEventGroupCreate(); //創建事件組叫做amota32_event_group
        xTaskCreate(&amota_task, "amota_task", 8192, (void *)&callbacks, 10, NULL)
    }
}

```
<br />

---

<a name="amota32_initialize"></a>

### esp_err_t amota32_initialize(init_error_callback_t init_error_handle)

初始化 UART 通訊埠、NVS 系統、SPIFFS 檔案系統以及命令列介面，所有的初始化參數皆於`config.json`中配置。

**Arguments:**

`init_error_handle`：處理初始化錯誤的callback function

**Returns:**

- `ESP_OK` (esp_err_t): ok

**Examples:**

```C
//初始化，並將初始化結果返回給ret

amota_callbacks_t callbacks = {
        .init_error_callback = error_handle,
    };

if (amota32_initialize(callbacks.init_error_callback) == ESP_OK) {
        ESP_LOGI(TAG, "AMOTA initialization success.");
    } else {
        ESP_LOGE(TAG, "AMOTA initialization failed.");
    }

```
<br />


<a name="cli"></a>

## 4.4 Command Line Interface

| CLI                                                                             | 說明                                                                                                                          |
| ------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `amota ota-update [-y]`                                                         | 執行 ESP32 OTA 更新。若帶 -y 選項，系統便會直接執行更新；否則將詢問使用者是否確定要執行更新。                                 |
| `amota ota-check`                                                               | 檢查可用的 OTA 更新(包含 ESP32 本身與額外的更新檔)。                                                                          |
| `amota esp-version`                                                             | 顯示 ESP32 內的韌體版本。                                                                                                             |
| `amota esp-rollback`                                                            | 將 ESP32 的韌體回滾至前一版本。                                                                                               |
| `amota autoauth [-y\|-N]`                                                       | 設定自動授權。預設為 `N`，即不授權Amota32 自動執行 ESP32 OTA 更新，使用者每次都需要手動授權 Amota32 執行更新。 |
| `amota list-files`                                                              | 列出暫存檔案清單及檔案索引。                                                                                                  |
| `amota read-file --index=<index>`                                               | 根據索引來讀取指定的檔案內容。                                                                                                |
| `amota delete-file --index=<index>`                                             | 根據索引來刪除指定的檔案                                                                                                      |
| `amota set-wifi --ssid <ssid> --passward <password>`                            | 設定 WiFi 的 SSID 與密碼                                                                                                      |
| `amota set-mqtt --url <broker url> --port <port> --access-token <access token>` | 設定欲連接的 MQTT 服務器 URL、port number 與設備的 access token                                                               |
| `amota set-uart  --txd <TXD pin number> --rxd <RXD pin number>`                 | 設定 UART 所使用的 GPIO number，預設txd為GPIO 4、rxd為GPIO 5                                                                               |
| `amota --help`                                                                  | 命令行使用說明                                                                                                                |



<br />
<br />
<a name="Appendix"></a>

# 5. Appendix

以下說明**兩種**安裝開發環境所需軟體的方法：

- 整合開發環境的擴充套件(推薦)
- 手動安裝

![needed software](https://github.com/meiiiii1539/Amota32_img/raw/main/image/needed_software.png)

<a name="VSCODE_extension"></a>

## 5.1 安裝 ESP-IDF：使用 VSCODE 擴充套件

**STEP 1:** 下載擴充套件

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/download_extension_1.png" alt="[one" style="width: 1000px; height: auto;" />

**STEP2:** 點選 View >Command Palette，並輸入 configure esp-idf extension

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/configure_extension_2.png" alt="two" style="width: 1000px; height: auto;" />

**STEP 3:** 點選 Express，並選擇 IDF 下載來源、下載位置、toolpath 位置 (圖中參數僅作範例)

<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/download_idf_setting_3.png" alt="three" style="width: 1000px; height: auto;" />

**STEP 4:** 開始下載
您將看到一個顯示設置進度狀態的頁面，其中包括 ESP-IDF 下載進度、ESP-IDF 工具的下載和安裝進度，以及 Python 虛擬環境的建立過程。請耐心等待一段時間。


<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/wait_for%20download_4.png" alt="four" style="width: 1000px; height: auto;" />

<br />

<a name="Manual"></a>

## 5.2 安裝 ESP-IDF：手動安裝

請根據您的作業系統進行手動安裝，請參考官方文件：

- [Windows](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html)
- [Linux/MacOs](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html)

<br />

<a name="Hardware_detail"></a>

## 5.3. 配置硬體

在 [4.1 Configuration：SDK Configuration](#SDK_config) 的說明裡，我們將 ESP32 UART 的 TXD/RXD 分別設置到對應腳位 GPIO 4/GPIO 5，因此實體線路須如下圖所示，使用杜邦線將 TXD (GPIO 4) 連接到 USB-to-UART Bridge 的 RXD，以及將 RXD (GPIO 5) 連接到 Bridge 的 TXD。如此，上位機便可以通過串列埠和 Amota32 的命令行介面進行通訊互動。


<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/hw_install.png" alt="HW_安裝" style="width: 500px; height: auto;" />

#### 簡單示意圖
<img src="https://github.com/meiiiii1539/Amota32_img/raw/main/image/txdrxd.png" alt="HW_安裝" style="width: 500px; height: auto;" />
