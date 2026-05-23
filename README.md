# Improv WiFi Library

Improv is a free and open standard with ready-made SDKs that offer a great user experience to configure Wi-Fi on devices. More details [here](https://www.improv-wifi.com/).

This library provides an easier way to handle the Improv serial protocol in your firmware, allowing you to configure the WiFi via web browser as in the following [example](https://jnthas.github.io/improv-wifi-demo/).

Simplest use example:

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

## Bluetooth Support (Optional)

You can also use Improv over Bluetooth (uses about 25kb memory). BLE support is **optional** and must be explicitly enabled.

This fork's `ImprovWiFiBLE` is built against the BLE library **bundled with the ESP32 Arduino core** (`framework-arduinoespressif32/libraries/BLE`, a unified Bluedroid/NimBLE wrapper). No external NimBLE-Arduino dependency is required.

### Enabling BLE Support

Add the build flag to your `platformio.ini`:

```ini
build_flags =
    -DIMPROV_WIFI_BLE_ENABLED
```

That's it — no `lib_deps` entry is needed for BLE.

**Note:** BLE works on any ESP32-family SoC with Bluetooth hardware (ESP32, ESP32-C3, ESP32-S3, ESP32-C6, ESP32-C5, ESP32-P4 — anything with `SOC_BLE_SUPPORTED` in the IDF). Chips without a BLE radio (e.g. ESP32-S2, ESP32-C2) are not supported.

### Optional callbacks

- `onImprovError(cb)` — connection / RPC errors.
- `onImprovConnected(cb)` — Wi-Fi join succeeded.
- `onImprovIdentify(cb)` — client asked the device to identify itself (e.g. blink an LED, beep). The capability is advertised by default.
- `setCustomConnectWiFi(cb)` — override the default Wi-Fi connect logic.

### Example with BLE enabled:

```cpp
#include <ImprovWiFiBLE.h>

ImprovWiFiBLE improvBLE;

void onIdentify() {
  // Blink an LED, beep, etc.
}

void setup() {
  improvBLE.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32,
                          "My-Device-9a4c2b", "2.1.5", "My Device");
  improvBLE.onImprovError(/* ... */);
  improvBLE.onImprovConnected(/* ... */);
  improvBLE.onImprovIdentify(onIdentify);    // optional
  improvBLE.setCustomConnectWiFi(/* ... */); // optional
}

void loop() {
}
```

### Configuring a BLE Device

Provisioning happens from a browser over Web Bluetooth at
[improv-wifi.com](https://www.improv-wifi.com/). Open the page, click
**Connect device to Wi-Fi**, pick the advertised device, enter SSID and
password.

Browsers tested:

- **Chrome on macOS** — works out of the box.
- **Chrome on Android** — works out of the box.
- **iOS / iPadOS** — Safari and Chrome do **not** expose Web Bluetooth.
  Use the [Bluefy browser](https://apps.apple.com/app/bluefy-web-ble-browser/id1492822055)
  and open improv-wifi.com inside it.

### Using both Serial and BLE:

```cpp
#include <ImprovWiFiLibrary.h>
#ifdef IMPROV_WIFI_BLE_ENABLED
#include <ImprovWiFiBLE.h>
#endif

ImprovWiFi improvSerial(&Serial);
#ifdef IMPROV_WIFI_BLE_ENABLED
ImprovWiFiBLE improvBLE;
#endif

void setup() {
  improvSerial.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32, "My-Device-9a4c2b", "2.1.5", "My Device");
  #ifdef IMPROV_WIFI_BLE_ENABLED
  improvBLE.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32, "My-Device-9a4c2b", "2.1.5", "My Device");
  #endif
}

void loop() {
  improvSerial.handleSerial();
}
```

## Documentation

- Serial transport: [ImprovWiFiLibrary](docs/ImprovWiFiLibrary.md)
- BLE transport: [ImprovWiFiBLE](docs/ImprovWiFiBLE.md)

## License

This open source code is licensed under the MIT license (see [LICENSE](LICENSE)
for details).