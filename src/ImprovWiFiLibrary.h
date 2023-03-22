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

// TODO: Rename this class to ImprovWiFiSerial and create a super class called ImprovWiFi with common methods between Serial and BLE
class ImprovWiFi
{
private:
  const char* const CHIP_FAMILY_DESC[5] = {"ESP32", "ESP32-C3", "ESP32-S2", "ESP32-S3", "ESP8266"};
  typedef void (OnImprovWiFiError)(improv::Error);
  typedef void (OnImprovWiFiConnected)(std::string ssid, std::string password);
  typedef bool (CustomConnectWiFi)(std::string ssid, std::string password);

  OnImprovWiFiError* onImprovWiFiErrorCallback;
  OnImprovWiFiConnected* onImprovWiFiConnectedCallback;
  CustomConnectWiFi* customConnectWiFiCallback;
  improv::ImprovWiFiParamsStruct improvWiFiParams;

  uint8_t _buffer[128];
  uint8_t _position = 0;

  Stream* serial;

  void sendDeviceUrl(improv::Command cmd);
  bool onCommandCallback(improv::ImprovCommand cmd);
  void onErrorCallback(improv::Error err);
  void setState(improv::State state);
  void sendResponse(std::vector<uint8_t> &response);
  void setError(improv::Error error);
  bool tryConnectToWifi(std::string ssid, std::string password);
  void getAvailableWifiNetworks();
  inline void replaceAll(std::string &str, const std::string& from, const std::string& to);

  bool parseImprovSerial(size_t position, uint8_t byte, const uint8_t *buffer);
  improv::ImprovCommand parseImprovData(const std::vector<uint8_t> &data, bool check_checksum = true);
  improv::ImprovCommand parseImprovData(const uint8_t *data, size_t length, bool check_checksum = true);

public:
  ImprovWiFi(Stream* serial) {
    this->serial = serial;
  }
  void handleSerial();
  void setDeviceInfo(improv::ChipFamily chipFamily, std::string firmwareName, std::string firmwareVersion, std::string deviceName, std::string deviceUrl);
  void setDeviceInfo(improv::ChipFamily chipFamily, std::string firmwareName, std::string firmwareVersion, std::string deviceName);
  void onImprovWiFiError(OnImprovWiFiError *errorCallback);
  void onImprovWiFiConnected(OnImprovWiFiConnected *connectedCallback);
  void setCustomConnectWiFi(CustomConnectWiFi *connectWiFiCallBack);
  bool isConnected();
};
