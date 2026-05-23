# ImprovWiFiBLE

```cpp
class ImprovWiFiBLE : public BLECharacteristicCallbacks,
                      public BLEServerCallbacks
```

Improv WiFi BLE class.

## Description

Implements the [Improv-WiFi BLE protocol](https://www.improv-wifi.com/ble/)
on top of the BLE library bundled with the ESP32 Arduino core
(`framework-arduinoespressif32/libraries/BLE`, a unified Bluedroid/NimBLE
wrapper). No external NimBLE-Arduino dependency is required.

Only compiled when `IMPROV_WIFI_BLE_ENABLED` is defined.

Public API mirrors `ImprovWiFi` (the serial transport) so a sketch can use
either or both interchangeably.

## Example

```cpp
#include <ImprovWiFiBLE.h>

ImprovWiFiBLE improvBLE;

void onImprovError(ImprovTypes::Error err) { /* ... */ }
void onImprovConnected(const char *ssid, const char *pw) { /* ... */ }
void onIdentify() { /* blink LED, beep, etc. */ }
bool myConnect(const char *ssid, const char *pw) { /* ... */ return true; }

void setup() {
  improvBLE.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32,
                          "My-Device-9a4c2b", "2.1.5", "My Device");
  improvBLE.onImprovError(onImprovError);
  improvBLE.onImprovConnected(onImprovConnected);
  improvBLE.onImprovIdentify(onIdentify);       // optional
  improvBLE.setCustomConnectWiFi(myConnect);    // optional
}

void loop() {
  // No polling required — provisioning runs from BLE GATT callbacks.
}
```

Unlike `ImprovWiFi`, there is **no `handleSerial()` to call from
`loop()`** — incoming Improv RPCs are dispatched from BLE GATT write
callbacks driven by the BLE stack.

## BLE service layout

| Item | UUID | Properties |
|------|------|------------|
| Improv service | `00467768-6228-2272-4663-277478268000` | primary |
| Current state  | `...268001` | READ, NOTIFY |
| Error state    | `...268002` | READ, NOTIFY |
| RPC command    | `...268003` | WRITE, WRITE_NR |
| RPC response   | `...268004` | READ, NOTIFY |
| Capabilities   | `...268005` | READ |
| Adv. service data | `0x4677` (16-bit) | flags `[0x77, 0x46, state, caps, 0,0,0,0]` |

Advertised flags: `LE General Discoverable + BR/EDR not supported`.
Capabilities byte exposes `Identify` (bit 0) by default.

## Constructors

### 💡 ImprovWiFiBLE()

```cpp
ImprovWiFiBLE() = default;
```

Default constructor. BLE init happens inside the first
`setDeviceInfo(...)` call.

### 💡 ~ImprovWiFiBLE()

```cpp
~ImprovWiFiBLE();
```

Minimal cleanup. The BLE stack is **not** deinit-ed automatically; call
`BLEDevice::deinit()` from the sketch if you need it.

## Type definitions

### 🔘 typedef void(OnImprovError)(ImprovTypes::Error)

```cpp
typedef void(OnImprovError)(ImprovTypes::Error);
```

Callback invoked when any error occurs during protocol handling or
Wi-Fi connection. Same signature as the serial transport.

### 🔘 typedef void(OnImprovConnected)(const char *ssid, const char *password)

```cpp
typedef void(OnImprovConnected)(const char *ssid, const char *password);
```

Callback invoked when Wi-Fi join succeeds. The SSID/password are passed
so the sketch can persist them.

### 🔘 typedef void(OnImprovIdentify)()

```cpp
typedef void(OnImprovIdentify)();
```

Callback invoked when the Improv client issues the Identify RPC
(cmd `0x02`). Optional. Typical use: blink an LED, beep, flash a
display — anything that lets a user physically locate the device.
The `Identify` capability is advertised by default in the caps byte.

### 🔘 typedef bool(CustomConnectWiFi)(const char *ssid, const char *password)

```cpp
typedef bool(CustomConnectWiFi)(const char *ssid, const char *password);
```

Optional override for the default Wi-Fi connect logic. Return `true` on
success, `false` on failure (triggers `ERROR_UNABLE_TO_CONNECT`).

## Methods

### Ⓜ️ void setDeviceInfo(...)

```cpp
void setDeviceInfo(ImprovTypes::ChipFamily chipFamily,
                   const char *firmwareName,
                   const char *firmwareVersion,
                   const char *deviceName,
                   const char *deviceUrl);

void setDeviceInfo(ImprovTypes::ChipFamily chipFamily,
                   const char *firmwareName,
                   const char *firmwareVersion,
                   const char *deviceName);
```

Set details of your device **and** initialise BLE in one call:

1. `BLEDevice::init(deviceName)` + TX power + own-addr type.
2. Create GATT server, Improv service, all characteristics.
3. Seed initial state / error / caps values (without `notify()` —
   service not started yet).
4. `service_->start()`.
5. Build primary advertisement (Improv UUID + service data + short
   name) and scan response (full name).
6. Start advertising.

#### Parameters

- `chipFamily` — Chip variant: `CF_ESP32`, `CF_ESP32_C3`, `CF_ESP32_S2`,
  `CF_ESP32_S3`, `CF_ESP8266`. Reported back to the Improv client.
- `firmwareName` — Firmware name.
- `firmwareVersion` — Firmware version.
- `deviceName` — BLE device name (also used as short name in adv).
- `deviceUrl` — Optional URL returned after provisioning. The
  `{LOCAL_IPV4}` placeholder is substituted, e.g.
  `http://{LOCAL_IPV4}?name=Guest`. The four-arg overload omits it.

**Call exactly once.** Calling twice will re-init BLE on top of itself
and is not supported.

### Ⓜ️ void onImprovError(OnImprovError *cb)

```cpp
void onImprovError(OnImprovError *errorCallback);
```

Register the `OnImprovError` callback. Pass `nullptr` to clear.

### Ⓜ️ void onImprovConnected(OnImprovConnected *cb)

```cpp
void onImprovConnected(OnImprovConnected *connectedCallback);
```

Register the `OnImprovConnected` callback. Fires after a successful
Wi-Fi join, before the device URL response is sent over BLE.

### Ⓜ️ void onImprovIdentify(OnImprovIdentify *cb)

```cpp
void onImprovIdentify(OnImprovIdentify *identifyCallback);
```

Register the `OnImprovIdentify` callback. Fires on every Identify RPC
from the client. Optional — if unset, Identify is a no-op (the
capability is still advertised).

### Ⓜ️ void setCustomConnectWiFi(CustomConnectWiFi *cb)

```cpp
void setCustomConnectWiFi(CustomConnectWiFi *connectWiFiCallBack);
```

Register a custom Wi-Fi connect function. If set, it is used instead of
the built-in `tryConnectToWifi`.

### Ⓜ️ bool tryConnectToWifi(...)

```cpp
bool tryConnectToWifi(const char *ssid, const char *password);
bool tryConnectToWifi(const char *ssid, const char *password,
                      uint32_t delayMs, uint8_t maxAttempts);
```

Default Wi-Fi connect helper. Waits `delayMs` between polls (default
`DELAY_MS_WAIT_WIFI_CONNECTION` = 500) for up to `maxAttempts`
iterations (default `MAX_ATTEMPTS_WIFI_CONNECTION` = 20). Returns
`true` if `WL_CONNECTED` is observed in time, else `false` →
`ERROR_UNABLE_TO_CONNECT`.

### Ⓜ️ bool isConnected()

```cpp
bool isConnected();
```

Returns `WiFi.status() == WL_CONNECTED`. Same semantics as the serial
transport.

## BLE callbacks (inherited)

The class implements two ESP32 BLE library callback interfaces. These
are not part of the public Improv API — listed here for completeness.

### Ⓜ️ void onDisconnect(BLEServer *s) override

Restarts advertising when a peer disconnects, so the next client can
find the device.

### Ⓜ️ void onWrite(BLECharacteristic *c) override

Receives writes on the RPC-command characteristic
(`...268003`). Validates the minimum frame size and forwards to the
internal RPC dispatcher. All other characteristics ignore writes.

> **Note**
> The Bluedroid backend dispatches to `onWrite(BLECharacteristic*)`;
> the NimBLE backend dispatches the same call (the
> `ble_gap_conn_desc*` overload is library-internal). The plain
> single-arg override is the portable choice.

## Error codes (BLE side)

| Symbol         | Value | Meaning                              |
|----------------|-------|--------------------------------------|
| `ERR_NONE`     | 0x00  | No error.                            |
| `ERR_BAD_PACKET` | 0x01 | Frame length / checksum mismatch.  |
| `ERR_UNKNOWN_CMD`| 0x02 | RPC opcode not implemented.        |
| `ERR_CONNECT`  | 0x03  | Wi-Fi join failed.                   |
| `ERR_NOT_AUTH` | 0x04  | Authorization required.              |
| `ERR_UNKNOWN`  | 0xFF  | Catch-all.                           |

These are mapped to the high-level `ImprovTypes::Error` values
(`ERROR_INVALID_RPC`, `ERROR_UNABLE_TO_CONNECT`, …) for the
`OnImprovError` callback.

## State machine

| Symbol               | Value | Meaning                              |
|----------------------|-------|--------------------------------------|
| `STATE_AUTH_REQUIRED`| 0x01  | Physical authorization required (not used by this class — starts authorized). |
| `STATE_AUTHORIZED`   | 0x02  | Ready to receive Wi-Fi credentials.  |
| `STATE_PROVISIONING` | 0x03  | Attempting Wi-Fi join.               |
| `STATE_PROVISIONED`  | 0x04  | Wi-Fi joined; device URL returned.   |

Transitions are reflected in the State characteristic and in the
service-data field of the advertisement.

## Configuring a BLE Device

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
