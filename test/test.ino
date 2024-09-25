#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "esp_ota_ops.h"
#include "esp_partition.h"

#define LED_PIN 8
#define LED_COUNT 20
#define FIRMWARE_VERSION "2.3.0"

const char *ssid = "Hoang Le";
const char *password = "07032017";
Ticker apiTicker;

void timer_callback() {

        const esp_partition_t *factory_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

        if (factory_partition != NULL) {
            printf("Found factory partition, switching boot partition...\n");

            // Chuyển boot partition sang phân vùng factory
            esp_err_t err = esp_ota_set_boot_partition(factory_partition);
            if (err == ESP_OK) {
                printf("Factory partition set successfully, restarting...\n");
                esp_restart(); // Khởi động lại thiết bị để áp dụng firmware từ phân vùng factory
            } else {
                printf("Failed to set factory partition, error: %d\n", err);
            }
        } else {
            printf("No factory partition found!\n");
        }
}

void setup() {
    Serial.begin(115200);
    apiTicker.attach(5, timer_callback); // For testing, we can just call it directl
}

void loop() {
}
