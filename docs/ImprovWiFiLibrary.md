<a name="line-15"></a>
# ImprovWiFi

```cpp
class ImprovWiFi
```

Main Improv WiFi class

#### Description

Handles the Improv WiFi protocol

#### Example

Simple example of using ImprovWiFi lib

```cpp
#include <ImprovWiFiLibrary.h>

ImprovWiFi improvSerial(&Serial);

void setup() {
  improvSerial.setDeviceInfo(improv::ChipFamily::CF_ESP32, "Firmware Name", "Firmware Version", "App Name");
  improvSerial.onImprovWiFiError(onImprovWiFiErrorCallback);
  improvSerial.onImprovWiFiConnected(onImprovWiFiConnectedCallback);
}

void loop() {
  improvSerial.handleSerial();
}
```


<a name="line-75"></a>
## Constructors

<a name="line-79"></a>
### 💡 ImprovWiFi(Stream *serial)

```cpp
ImprovWiFi(Stream *serial)
```

Create an instance of ImprovWiFi

#### Parameters

- `serial` - Stream object used to handle requests, for the most cases use `Serial`

<a name="line-91"></a>
## Methods

<a name="line-95"></a>
### Ⓜ️ void handleSerial()

```cpp
void handleSerial()
```

Check if a communication has being started via serial. Call this method on loop().


<a name="line-106"></a>
### Ⓜ️ bool tryConnectToWifi(std::string ssid, std::string password)

```cpp
bool tryConnectToWifi(std::string ssid, std::string password)
```

Tries to connect in a WiFi network


<a name="line-112"></a>
### Ⓜ️ bool isConnected()

```cpp
bool isConnected()
```

Check if cponnection is established

