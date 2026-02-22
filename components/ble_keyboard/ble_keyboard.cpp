#ifdef USE_ESP32

#include "ble_keyboard.h"
#include "esphome/core/log.h"
#include <NimBLEServer.h>
#include <NimBLEDevice.h>
#include <NimBLEService.h>
#include <NimBLECharacteristic.h>
#include <NimBLEAdvertising.h>
#include <string>
#include <list>

namespace esphome {
namespace ble_keyboard {
static const char *const TAG = "ble_keyboard";

void Esp32BleKeyboard::setup() {
  if (this->setup_) {
    ESP_LOGW(TAG, "BLE keyboard already setup.");
    return;
  }
  ESP_LOGI(TAG, "Setting up BLE Keyboard component (no direct BLE init) ...");
  ESP_LOGI(TAG, "BLE Keyboard name: %s", name_.c_str());
  ESP_LOGI(TAG, "BLE Manufacturer: %s", manufacturer_id_.c_str());
  ESP_LOGI(TAG, "BLE Battery Level: %d", battery_level_);
  ESP_LOGI(TAG, "BLE Pairing Code: %u", pairing_code_);
  ESP_LOGI(TAG, "BLE Advertising on start: %s", advertise_on_start_ ? "true" : "false");
  ESP_LOGI(TAG, "BLE Reconnect: %s", reconnect_ ? "true" : "false");
  ESP_LOGI(TAG, "BLE Release Delay: %u", release_delay_);
  ESP_LOGI(TAG, "BLE Default Delay: %u", default_delay_);
  // ESPHome 2026.x handles Bluetooth stack initialization.
  // Set a custom device name for easier discovery
  bleKeyboard.setName("ESP32KB");
  // Optionally set manufacturer (uncomment if needed)
  // bleKeyboard.setManufacturer("ESPHome");
  // Release all keys to start clean
  bleKeyboard.releaseAll();
  ESP_LOGI(TAG, "Starting BLE advertising... (handled by ESPHome)");
  // bleKeyboard.begin() must NOT be called here; ESPHome manages BLE initialization.
  this->setup_ = true;
}

void Esp32BleKeyboard::stop() {
  if (!this->setup_) {
    ESP_LOGW(TAG, "Attempting to use without setup.  Not doing anything.");
    return;
  }
  ESP_LOGD(TAG, "stop()");
  if (this->reconnect_ && pServer) {
    ESP_LOGD(TAG, "advertiseOnDisconnect(false)");
    pServer->advertiseOnDisconnect(false);
  }
  bleKeyboard.releaseAll();
}

void Esp32BleKeyboard::update() {
  if (state_sensor_) {
    state_sensor_->publish_state(this->is_connected());
  }
}

bool Esp32BleKeyboard::is_connected() {
  bool connected = bleKeyboard.isConnected();
  ESP_LOGD(TAG, "BLE Keyboard connection state: %s", connected ? "connected" : "disconnected");
  return connected;
}

void Esp32BleKeyboard::update_timer() {
  this->cancel_timeout((const std::string) TAG);
  this->set_timeout((const std::string) TAG, release_delay_, [this]() { this->release(); });
}

void Esp32BleKeyboard::press(std::string message) {
  if (!this->setup_) {
    ESP_LOGW(TAG, "Attempting to use without setup.  Not doing anything.");
    return;
  }
  if (this->is_connected()) {
    if (message.length() >= 5) {
      for (unsigned i = 0; i < message.length(); i += 5) {
        bleKeyboard.print(message.substr(i, 5).c_str());
        delay(default_delay_);
      }
      return;
    }
    bleKeyboard.print(message.c_str());
  }
}

void Esp32BleKeyboard::press(uint8_t key, bool with_timer) {
  if (!this->setup_) {
    ESP_LOGW(TAG, "Attempting to use without setup.  Not doing anything.");
    return;
  }
  if (this->is_connected()) {
    if (with_timer) {
      this->update_timer();
    }
    bleKeyboard.press(key);
  }
}

void Esp32BleKeyboard::press(MediaKeyReport key, bool with_timer) {
  if (!this->setup_) {
    ESP_LOGW(TAG, "Attempting to use without setup.  Not doing anything.");
    return;
  }
  if (this->is_connected()) {
    if (with_timer) {
      this->update_timer();
    }
    bleKeyboard.press(key);
  }
}

void Esp32BleKeyboard::release() {
  if (!this->setup_) {
    ESP_LOGW(TAG, "Attempting to use without setup.  Not doing anything.");
    return;
  }
  if (this->is_connected()) {
    this->cancel_timeout((const std::string) TAG);
    bleKeyboard.releaseAll();
  }
}
}  // namespace ble_keyboard
}  // namespace esphome

#endif
