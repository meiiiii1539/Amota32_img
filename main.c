#include "amota32.h"

static const char *TAG = "app";

void wifi_on_connect() {
    printf("Wi-Fi connected successfully!\n");
}

void wifi_error_callback(int error_code, char *msg) {
    printf("Wi-Fi error occurred! Error code: %d, Message: %s\n", error_code, msg);
}

void mqtt_on_connect() {
    printf("Wi-Fi connected successfully!\n");
}

void mqtt_error_callback(void *event) {
    printf("MQTT error occurred!\n");
}

void mqtt_get_fw_info(struct FW_INFO fw_info) {
    printf("Received firmware information:\n");
    printf("Firmware Name: %s\n", fw_info.fw_name);
    printf("Firmware Version: %s\n", fw_info.fw_ver);
    printf("Firmware Size: %lu\n", fw_info.fw_size);

    start_esp32_ota_update(CURRENT_FIRMWARE_VERSION, fw_info, NULL);
}

void exfw_error_callback(int error_code, char *msg) {
    printf("External firmware error occurred! Error code: %d, Message: %s\n", error_code, msg);
}

// void fw_download_implementation(esp_http_client_handle_t fw_download_client) {
//     printf("Firmware download initiated.\n");
//     // Implementation details for firmware download can be added here
// }

void init_error_callback(int error_code, char *msg) {
    printf("Initialization error occurred! Error code: %d, Message: %s\n", error_code, msg);
}

void fw_ver_equal(const char *current_esp_fw_ver, const char *target_ver) {
    printf("Current firmware version %s is equal to target version %s.\n", current_esp_fw_ver, target_ver);
}

void after_ota_finish(void *data) {
    printf("OTA update finished successfully!\n");
}

void after_ota_fail(int *error_code) {
    printf("OTA update failed! Error code: %d\n", *error_code);
}

void fw_params_no_specified(struct FW_INFO receive_info) {
    printf("No firmware parameters specified.\n");
}


void app_main(void) {
    amota_callbacks_t callbacks = {
        .wifi_on_connect = wifi_on_connect,
        .wifi_error_callback = wifi_error_callback,

        .mqtt_on_connect = mqtt_on_connect ,
        .mqtt_error_callback=mqtt_error_callback,
        .mqtt_get_fw_info=mqtt_get_fw_info,

        .exfw_error_callback = exfw_error_callback,
        // .fw_download_implementation=fw_download_implementation,

        .init_error_callback=init_error_callback,

        .fw_ver_equal=fw_ver_equal,
        .after_ota_finish=after_ota_finish,
        .after_ota_fail=after_ota_fail,
        .fw_params_no_specified=fw_params_no_specified,
    };

    printf("main: MQTT ERROR Callback Address: %p\n", callbacks.mqtt_error_callback);

    if (amota32_initialize(callbacks.init_error_callback) == ESP_OK) {
        amota32_event_group = xEventGroupCreate(); //創建事件組叫做amota32_event_group
        if (amota32_event_group == NULL) {
            ESP_LOGE(TAG, "Failed to create event group");
            return;
        }
        if (xTaskCreate(&amota_cli_uart_task, "amota_cli_uart_task", 8192, (void *)&callbacks, 10, NULL) != pdPASS) {
            ESP_LOGE(TAG, "Failed to create CLI UART task");
            return;
        }
        if (xTaskCreate(&amota_task, "amota_task", 16384, (void *)&callbacks, 5, NULL) != pdPASS) {
            ESP_LOGE(TAG, "Failed to create AMOTA task");
            return;
        }
    } else {
        ESP_LOGE(TAG, "AMOTA initialization failed");
    }
}