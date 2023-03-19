#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "improv.h"

#define MAX_ATTEMPTS_WIFI_CONNECTION 20

namespace ImprovWiFi
{
  //Private

  typedef void (*OnImprovWiFiError)(improv::Error);
  typedef void (*OnImprovWiFiConnected)(std::string ssid, std::string password);
  


  bool isConnected();
  void sendDeviceUrl(improv::Command cmd);
  bool onCommandCallback(improv::ImprovCommand cmd);
  void onErrorCallback(improv::Error err);

  void set_state(improv::State state);
  void send_response(std::vector<uint8_t> &response);
  void set_error(improv::Error error);
  bool tryConnectToWifi(std::string ssid, std::string password);
  void getAvailableWifiNetworks();
  static inline void replaceAll(std::string &str, const std::string& from, const std::string& to);

  enum ChipFamily : uint8_t {
    CF_ESP32,
    CF_ESP32_C3,
    CF_ESP32_S2,
    CF_ESP32_S3,
    CF_ESP8266
  };


  struct ImprovWiFiParamsStruct {
    std::string firmwareName;
    std::string firmwareVersion;
    ImprovWiFi::ChipFamily chipFamily;
    std::string deviceName;
    std::string deviceUrl;
  };



  //Public 
  void setup();
  void loop();
  ImprovWiFiParamsStruct* getParams();
  void setOnImprovWiFiError(OnImprovWiFiError errorCallback);
  void setOnImprovWiFiConnected(OnImprovWiFiConnected connectedCallback);

}