# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repo purpose

Arduino library implementing the [Improv WiFi](https://www.improv-wifi.com/) provisioning protocol on ESP32 / ESP8266. Two transports, same API surface:

- **Serial** — `ImprovWiFi` ([src/ImprovWiFiLibrary.h](src/ImprovWiFiLibrary.h)). Always compiled.
- **BLE** — `ImprovWiFiBLE` ([src/ImprovWiFiBLE.h](src/ImprovWiFiBLE.h)). Compiled only when `-DIMPROV_WIFI_BLE_ENABLED` is set. ESP32-family only.

This is a fork of `jnthas/Improv-WiFi-Library`. BLE transport uses the **BLE library bundled with the ESP32 Arduino core** (Bluedroid/NimBLE wrapper under `framework-arduinoespressif32/libraries/BLE`) — no `NimBLE-Arduino` lib_dep needed.

## Build / run

Library has no standalone build target. Build via the example:

```
cd examples/arduino-BLE
pio run -e m5stack-nanoc6              # or -e m5stack-atoms3 / m5stamp-s3 / esp32p4_waveshare_devkit
pio run -e m5stack-nanoc6 -t upload
pio device monitor
```

[examples/arduino-BLE/platformio.ini](examples/arduino-BLE/platformio.ini) pulls the lib via `symlink://../..`, so edits to `src/` build immediately. Toolchain pinned to `pioarduino/platform-espressif32` 55.03.38 (Arduino 3 / IDF 5.x).

Plain (non-BLE) examples: [examples/SimpleWebServer](examples/SimpleWebServer), [examples/SimpleWebServerEsp8266](examples/SimpleWebServerEsp8266) — Arduino IDE style `.ino`, no `platformio.ini`.

`CMakeLists.txt` exists for ESP-IDF component consumption but **only registers the Serial source** — BLE not wired into the IDF component path.

No test suite. No linter config.

## Architecture

`ImprovTypes.h` is shared: enums (`Command`, `State`, `Error`, `ChipFamily`), the `ImprovCommand` struct, and tunables `MAX_ATTEMPTS_WIFI_CONNECTION` / `DELAY_MS_WAIT_WIFI_CONNECTION` (override via `-D`). Both transports speak the same RPC payload format; only framing differs.

Serial transport (`ImprovWiFi`):
- `handleSerial()` drains the `Stream*` byte-by-byte through `parseImprovSerial` into `parseImprovData` → `onCommandCallback`.
- Builds RPC responses with `build_rpc_response` (checksum-appended) and writes back to the same `Stream`.

BLE transport (`ImprovWiFiBLE`):
- Inherits `BLECharacteristicCallbacks` + `BLEServerCallbacks`. `setDeviceInfo()` sets up the GATT service (UUIDs `00467768-6228-2272-4663-2774782680{00..05}`) and starts advertising; no `begin()` call.
- RPC arrives via `onWrite` on `CHAR_RPC_CMD`; responses notify on `CHAR_RPC_RES`; state/error/caps are separate notifying characteristics.
- Advertisement includes 16-bit service-data UUID `0x4677` with current state + caps so improv-wifi.com can filter.
- Identify capability (`caps_ = 0x01`) is advertised by default and dispatches `onImprovIdentify()`.

Callbacks (set before/after `setDeviceInfo`):
- `onImprovError`, `onImprovConnected`, `setCustomConnectWiFi` — both transports.
- `onImprovIdentify` — **BLE only** (Serial protocol has no identify RPC).

Successful provisioning calls `onImprovConnectedCallback` with SSID/password — credential persistence (NVS, etc.) is the sketch's responsibility; the library does not store creds.

## Working on this fork

- Versions live in **three** files; bump together: [library.json](library.json), [library.properties](library.properties), and any example referencing version.
- BLE code must stay inside `#ifdef IMPROV_WIFI_BLE_ENABLED` so non-BLE builds (and ESP8266) still compile.
- Chips without a BLE radio (ESP32-S2, ESP32-C2) cannot use the BLE transport — guard with `SOC_BLE_SUPPORTED` if adding new BLE entry points.
- iOS Safari/Chrome lack Web Bluetooth — provisioning from iOS requires Bluefy browser (mention in user-facing docs, not code).
