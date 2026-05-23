> This document was generated from file `ImprovWiFiLibrary.h` at 5/20/2026, 1:48:17 PM
<a name="line-18"></a>
# ImprovWiFi

```cpp
class ImprovWiFi /* line 44 */
```

Improv WiFi class

#### Description

Handles the Improv WiFi Serial protocol (https://www.improv-wifi.com/serial/)

#### Example

Simple example of using ImprovWiFi lib. A complete one can be seen in `examples/` folder.

```cpp
#include <ImprovWiFiLibrary.h>

ImprovWiFi improvSerial(&Serial);

void setup() {
  improvSerial.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32, "My-Device-9a4c2b", "2.1.5", "My Device");
}

void loop() {
  improvSerial.handleSerial();
}
```


<a name="line-71"></a>
## Constructors

<a name="line-75"></a>
### 💡 ImprovWiFi(Stream *serial)

```cpp
ImprovWiFi(Stream *serial) /* line 82 */
```

Create an instance of ImprovWiFi

#### Parameters

- `serial` - Pointer to stream object used to handle requests, for the most cases use `Serial`

<a name="line-87"></a>
## Type definition

<a name="line-91"></a>
### 🔘 typedef void(OnImprovError)(ImprovTypes::Error)

```cpp
typedef void(OnImprovError)(ImprovTypes::Error) /* line 94 */
```

Callback function called when any error occurs during the protocol handling or wifi connection.

<a name="line-96"></a>
### 🔘 typedef void(OnImprovConnected)(const char *ssid, const char *password)

```cpp
typedef void(OnImprovConnected)(const char *ssid, const char *password) /* line 99 */
```

Callback function called when the attempt of wifi connection is successful. It informs the SSID and Password used to that, it's a perfect time to save them for further use.

<a name="line-101"></a>
### 🔘 typedef bool(CustomConnectWiFi)(const char *ssid, const char *password)

```cpp
typedef bool(CustomConnectWiFi)(const char *ssid, const char *password) /* line 104 */
```

Callback function to customize the wifi connection if you needed. Optional.

<a name="line-106"></a>
## Methods

<a name="line-110"></a>
### Ⓜ️ void handleSerial()

```cpp
void handleSerial() /* line 114 */
```

Check if a communication via serial is happening. Put this call on your loop().


<a name="line-116"></a>
### Ⓜ️ void setDeviceInfo(ImprovTypes::ChipFamily chipFamily, const char *firmwareName, const char *firmwareVersion, const char *deviceName, const char *deviceUrl)

```cpp
void setDeviceInfo(ImprovTypes::ChipFamily chipFamily, const char *firmwareName, const char *firmwareVersion, const char *deviceName, const char *deviceUrl) /* line 129 */
```

Set details of your device.

#### Parameters

- `chipFamily` - Chip variant, supported are CF_ESP32, CF_ESP32_C3, CF_ESP32_S2, CF_ESP32_S3, CF_ESP8266. Consult ESP Home [docs](https://esphome.io/components/esp32.html) for more information.
- `firmwareName` - Firmware name
- `firmwareVersion` - Firmware version
- `deviceName` - Your device name
- `deviceUrl`- The local URL to access your device. A placeholder called {LOCAL_IPV4} is available to form elaboreted URLs. E.g. `http://{LOCAL_IPV4}?name=Guest`.
  There is overloaded method without `deviceUrl`, in this case the URL will be the local IP.


<a name="line-132"></a>
### Ⓜ️ void onImprovError(OnImprovError *errorCallback)

```cpp
void onImprovError(OnImprovError *errorCallback) /* line 135 */
```

Method to set the typedef OnImprovError callback.

<a name="line-137"></a>
### Ⓜ️ void onImprovConnected(OnImprovConnected *connectedCallback)

```cpp
void onImprovConnected(OnImprovConnected *connectedCallback) /* line 140 */
```

Method to set the typedef OnImprovConnected callback.

<a name="line-142"></a>
### Ⓜ️ void setCustomConnectWiFi(CustomConnectWiFi *connectWiFiCallBack)

```cpp
void setCustomConnectWiFi(CustomConnectWiFi *connectWiFiCallBack) /* line 145 */
```

Method to set the typedef CustomConnectWiFi callback.

<a name="line-147"></a>
### Ⓜ️ bool tryConnectToWifi(const char *ssid, const char *password)

```cpp
bool tryConnectToWifi(const char *ssid, const char *password) /* line 152 */
```

Default method to connect in a WiFi network.
It waits `DELAY_MS_WAIT_WIFI_CONNECTION` milliseconds (default 500) during `MAX_ATTEMPTS_WIFI_CONNECTION` (default 20) until it get connected. If it does not happen, an error `ERROR_UNABLE_TO_CONNECT` is thrown.


<a name="line-154"></a>
### Ⓜ️ bool tryConnectToWifi(const char *ssid, const char *password, uint32_t delayMs, uint8_t maxAttempts)

```cpp
bool tryConnectToWifi(const char *ssid, const char *password, uint32_t delayMs, uint8_t maxAttempts) /* line 157 */
```

Overload: tryConnectToWifi with custom delay and max attempts.

<a name="line-159"></a>
### Ⓜ️ bool isConnected()

```cpp
bool isConnected() /* line 163 */
```

Check if connection is established using `WiFi.status() == WL_CONNECTED`

