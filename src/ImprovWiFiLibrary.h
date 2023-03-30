#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Stream.h>
#include "improv.h"

#ifndef MAX_ATTEMPTS_WIFI_CONNECTION
#define MAX_ATTEMPTS_WIFI_CONNECTION 20
#endif
#ifndef DELAY_MS_WAIT_WIFI_CONNECTION
#define DELAY_MS_WAIT_WIFI_CONNECTION 500
#endif

/**
 * Main Improv WiFi class
 *
 * ### Description
 *
 * Handles the Improv WiFi protocol
 *
 * ### Example
 *
 * Simple example of using ImprovWiFi lib
 *
 * ```cpp
 * #include <ImprovWiFiLibrary.h>
 *
 * ImprovWiFi improvSerial(&Serial);
 *
 * void setup() {
 *   improvSerial.setDeviceInfo(improv::ChipFamily::CF_ESP32, "Firmware Name", "Firmware Version", "App Name");
 *   improvSerial.onImprovWiFiError(onImprovWiFiErrorCallback);
 *   improvSerial.onImprovWiFiConnected(onImprovWiFiConnectedCallback);
 * }
 *
 * void loop() {
 *   improvSerial.handleSerial();
 * }
 * ```
 *
 */
class ImprovWiFi
{
private:
  const char *const CHIP_FAMILY_DESC[5] = {"ESP32", "ESP32-C3", "ESP32-S2", "ESP32-S3", "ESP8266"};
  typedef void(OnImprovWiFiError)(improv::Error);
  typedef void(OnImprovWiFiConnected)(const char* ssid, const char* password);
  typedef bool(CustomConnectWiFi)(const char* ssid, const char* password);

  OnImprovWiFiError *onImprovWiFiErrorCallback;
  OnImprovWiFiConnected *onImprovWiFiConnectedCallback;
  CustomConnectWiFi *customConnectWiFiCallback;
  improv::ImprovWiFiParamsStruct improvWiFiParams;

  uint8_t _buffer[128];
  uint8_t _position = 0;

  Stream *serial;

  void sendDeviceUrl(improv::Command cmd);
  bool onCommandCallback(improv::ImprovCommand cmd);
  void onErrorCallback(improv::Error err);
  void setState(improv::State state);
  void sendResponse(std::vector<uint8_t> &response);
  void setError(improv::Error error);
  void getAvailableWifiNetworks();
  inline void replaceAll(std::string &str, const std::string &from, const std::string &to);

  //improv SDK
  bool parseImprovSerial(size_t position, uint8_t byte, const uint8_t *buffer);
  improv::ImprovCommand parseImprovData(const std::vector<uint8_t> &data, bool check_checksum = true);
  improv::ImprovCommand parseImprovData(const uint8_t *data, size_t length, bool check_checksum = true);
  std::vector<uint8_t> build_rpc_response(improv::Command command, const std::vector<std::string> &datum, bool add_checksum);

public:
  /**
   * ## Constructors
   **/

  /**
   * Create an instance of ImprovWiFi
   *
   * # Parameters
   *
   * - `serial` - Stream object used to handle requests, for the most cases use `Serial`
   */
  ImprovWiFi(Stream *serial)
  {
    this->serial = serial;
  }

  /**
   * ## Methods
   **/

  /**
   * Check if a communication has being started via serial. Call this method on loop().
   *
   */
  void handleSerial();
  void setDeviceInfo(improv::ChipFamily chipFamily, const char* firmwareName, const char* firmwareVersion, const char* deviceName, const char* deviceUrl);
  void setDeviceInfo(improv::ChipFamily chipFamily, const char* firmwareName, const char* firmwareVersion, const char* deviceName);
  void onImprovWiFiError(OnImprovWiFiError *errorCallback);
  void onImprovWiFiConnected(OnImprovWiFiConnected *connectedCallback);
  void setCustomConnectWiFi(CustomConnectWiFi *connectWiFiCallBack);

  /**
   * Tries to connect in a WiFi network
   *
   */
  bool tryConnectToWifi(const char* ssid, const char* password);

  /**
   * Check if cponnection is established
   *
   */
  bool isConnected();
};
