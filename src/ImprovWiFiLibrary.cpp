#include "WiFiType.h"
#include "improv.h"
#include "ImprovWiFiLibrary.h"

namespace ImprovWiFi
{

  const char* const CHIP_FAMILY_DESC[5] = {"ESP32", "ESP32-C3", "ESP32-S2", "ESP32-S3", "ESP8266"};

  uint8_t _buffer[16];
  uint8_t _position = 0;

  ImprovWiFiParamsStruct improvWiFiParams;


  //std::function<void(improv::Error)> onImprovWiFiError;

  OnImprovWiFiError onImprovWiFiError;
  OnImprovWiFiConnected onImprovWiFiConnected;


  void setup() {
    Serial.println("Setup");
    Serial.println(improvWiFiParams.firmwareName.c_str());
    Serial.println(improvWiFiParams.deviceName.c_str());
    Serial.println(improvWiFiParams.firmwareVersion.c_str());
    
  }
  void loop() {
    
    if (Serial.available() > 0) {
      uint8_t b = Serial.read();

      if (parse_improv_serial_byte(_position, b, _buffer, onCommandCallback, onErrorCallback)) {
        //TODO: limit of _buffer is 16
        _buffer[_position++] = b;      
      } else {
        _position = 0;
      }
    }

  }


  void onErrorCallback(improv::Error err) {
    onImprovWiFiError(err);
  }

  bool onCommandCallback(improv::ImprovCommand cmd) {

    switch (cmd.command) {
      case improv::Command::GET_CURRENT_STATE:
      {
        if (isConnected()) {
          set_state(improv::State::STATE_PROVISIONED);
          sendDeviceUrl(cmd.command);
        } else {
          set_state(improv::State::STATE_AUTHORIZED);
        }
        
        break;
      }

      case improv::Command::WIFI_SETTINGS:
      {
        if (cmd.ssid.empty()) {
          set_error(improv::Error::ERROR_INVALID_RPC);
          break;
        }
      
        set_state(improv::STATE_PROVISIONING);
        
        if (tryConnectToWifi(cmd.ssid, cmd.password)) {

          //blink_led(100, 3);
          
          //TODO: Persist credentials here
          set_error(improv::Error::ERROR_NONE);
          set_state(improv::STATE_PROVISIONED);
          sendDeviceUrl(cmd.command);  
          onImprovWiFiConnected(cmd.ssid, cmd.password);

        } else {
          set_state(improv::STATE_STOPPED);
          set_error(improv::ERROR_UNABLE_TO_CONNECT);
          onImprovWiFiError(improv::ERROR_UNABLE_TO_CONNECT);
        }
        
        break;
      }

      case improv::Command::GET_DEVICE_INFO:
      {
        std::vector<std::string> infos = {
          // Firmware name
          improvWiFiParams.firmwareName,
          // Firmware version
          improvWiFiParams.firmwareVersion,
          // Hardware chip/variant
          CHIP_FAMILY_DESC[improvWiFiParams.chipFamily],
          // Device name
          improvWiFiParams.deviceName
        };
        std::vector<uint8_t> data = improv::build_rpc_response(improv::GET_DEVICE_INFO, infos, false);
        send_response(data);
        break;
      }

      case improv::Command::GET_WIFI_NETWORKS:
      {
        getAvailableWifiNetworks();
        break;
      }

      default: {
        set_error(improv::ERROR_UNKNOWN_RPC);
        return false;
      }
    }

    return true;
  }

  ImprovWiFiParamsStruct* getParams() {
    return &improvWiFiParams;
  }


  bool isConnected() {
    return (WiFi.status() == WL_CONNECTED);
  }

  void sendDeviceUrl(improv::Command cmd) {
    // URL where user can finish onboarding or use device
    // Recommended to use website hosted by device
    if (improvWiFiParams.deviceUrl.empty()) {
      improvWiFiParams.deviceUrl = String("http://" + WiFi.localIP().toString()).c_str();
    } else {
      replaceAll(improvWiFiParams.deviceUrl, "{LOCAL_IPV4}", WiFi.localIP().toString().c_str());
    }

    std::vector<uint8_t> data = improv::build_rpc_response(cmd, {improvWiFiParams.deviceUrl}, false);
    send_response(data);
  }

  void setOnImprovWiFiError(OnImprovWiFiError errorCallback) {
    onImprovWiFiError = errorCallback;
  }

  void setOnImprovWiFiConnected(OnImprovWiFiConnected connectedCallback) {
    onImprovWiFiConnected = connectedCallback;
  }


  bool tryConnectToWifi(std::string ssid, std::string password) {
    uint8_t count = 0;

    WiFi.begin(ssid.c_str(), password.c_str());

    while (!isConnected()) {
      //blink_led(500, 1);
      delay(500);

      if (count > MAX_ATTEMPTS_WIFI_CONNECTION) {
        WiFi.disconnect();
        return false;
      }
      count++;
    }

    return true;
  }


  void getAvailableWifiNetworks() {
    int networkNum = WiFi.scanNetworks();

    for (int id = 0; id < networkNum; ++id) { 
      std::vector<uint8_t> data = improv::build_rpc_response(
              improv::GET_WIFI_NETWORKS, {WiFi.SSID(id), String(WiFi.RSSI(id)), (WiFi.encryptionType(id) == WIFI_AUTH_OPEN ? "NO" : "YES")}, false);
      send_response(data);
      delay(1);
    }
    //final response
    std::vector<uint8_t> data =
            improv::build_rpc_response(improv::GET_WIFI_NETWORKS, std::vector<std::string>{}, false);
    send_response(data);
  }


  static inline void replaceAll(std::string &str, const std::string& from, const std::string& to)
  {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
  }

  void set_state(improv::State state) {  
    
    std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
    data.resize(11);
    data[6] = improv::IMPROV_SERIAL_VERSION;
    data[7] = improv::TYPE_CURRENT_STATE;
    data[8] = 1;
    data[9] = state;

    uint8_t checksum = 0x00;
    for (uint8_t d : data)
      checksum += d;
    data[10] = checksum;

    Serial.write(data.data(), data.size());
  }


  void send_response(std::vector<uint8_t> &response) {
    std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
    data.resize(9);
    data[6] = improv::IMPROV_SERIAL_VERSION;
    data[7] = improv::TYPE_RPC_RESPONSE;
    data[8] = response.size();
    data.insert(data.end(), response.begin(), response.end());

    uint8_t checksum = 0x00;
    for (uint8_t d : data)
      checksum += d;
    data.push_back(checksum);

    Serial.write(data.data(), data.size());
  }

  void set_error(improv::Error error) {
    std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
    data.resize(11);
    data[6] = improv::IMPROV_SERIAL_VERSION;
    data[7] = improv::TYPE_ERROR_STATE;
    data[8] = 1;
    data[9] = error;

    uint8_t checksum = 0x00;
    for (uint8_t d : data)
      checksum += d;
    data[10] = checksum;

    Serial.write(data.data(), data.size());
  }
  
}