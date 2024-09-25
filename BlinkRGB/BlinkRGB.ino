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
#define FIRMWARE_VERSION "2.4.0"

const char *ssid = "Hoang Le";
const char *password = "07032017";
Ticker apiTicker;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Preferences preferences;

void store_version_in_nvs() {
    preferences.begin("storage", false); // Open namespace storage
    preferences.putString("ver", FIRMWARE_VERSION); // Store version
    Serial.println("Firmware version stored in NVS: " + String(FIRMWARE_VERSION));
    preferences.end();
}

String get_version(String json) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "";
    }

    const char* version = doc["data"]["version"];
    return String(version);
}

String https_get_response() {
    HTTPClient http;
    http.begin("https://xsolar.energy/beapi/v1/firmware/get?deviceType=arduino");  // Specify your URL
    int httpCode = http.GET();
    String payload = "";

    if (httpCode > 0) {
        payload = http.getString();  // Get the response payload
    } else {
        printf("Error on HTTP request \n");
    }
    http.end();
    return payload;
}

void timer_callback() {
    String json_response = https_get_response();
    if (json_response == "") {
        printf("No response from API \n");
        return;
    }

    String new_version = get_version(json_response);
    if (new_version == "") {
        printf("Cannot parse version from JSON \n");
        return;
    }

    Serial.println("Current version: " + String(FIRMWARE_VERSION) + ", Version from API: " + new_version);

    if (new_version.compareTo(FIRMWARE_VERSION) > 0) {
        printf("OTA update available \n");

        // Tìm phân vùng factory
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
    } else {
        printf("No new version available \n");
    }
}

void setup() {
    Serial.begin(115200);
    
    //Initialize the LED strip
    strip.begin();
    strip.show();  // Initialize all pixels to 'off'

    //Store current firmware version in NVS (using Preferences)
    store_version_in_nvs();
    
    //Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        printf("Connecting to WiFi... \n");
    }
    printf("Connected to WiFi \n");

    // Call the timer_callback every 30 seconds
    apiTicker.attach(5, timer_callback); 
}

void loop() {
    //Animate the LED strip (a simplified version of your LED rainbow chase)
    static float offset = 0;
    for (int i = 0; i < LED_COUNT; i++) {
        float angle = offset + (i * 0.3);
        uint8_t red = sin(angle + 0) * 127 + 128;
        uint8_t green = sin(angle + 2.094) * 127 + 128;
        uint8_t blue = sin(angle + 4.188) * 127 + 128;
        strip.setPixelColor(i, strip.Color(red, green, blue));
    }
    strip.show();
    offset += 0.02;
    if (offset > 2 * PI) offset -= 2 * PI;

    delay(20);  // Frame duration
}
