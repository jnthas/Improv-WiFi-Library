# arduino-BLE example

Improv Wi-Fi provisioning over BLE for ESP32. Provisions Wi-Fi credentials from an Improv-compatible client (e.g. https://www.improv-wifi.com/), persists them in NVS, and serves a minimal HTTP info page once connected.

## Behavior

- Boot: load saved SSID/password from NVS, attempt connect.
- After `TIME_TO_CONNECT` (10s) without Wi-Fi: start BLE advertising + Improv provisioning.
- On successful provision: credentials saved, HTTP server starts on port 80.
- 5 clicks on `BUTTON_PIN`: clear creds, disconnect Wi-Fi, reboot.
- LED blink patterns signal state (connect attempt, error, identify, HTTP hit).

## Build

PlatformIO. Targets in `platformio.ini`:

- `m5stack-nanoc6` (ESP32-C6, button on GPIO9)
- `m5stack-atoms3`
- `m5stamp-s3`
- `esp32p4_waveshare_devkit`

```
pio run -e m5stack-nanoc6 -t upload
pio device monitor
```

Library pulled via `symlink://../..` (parent Improv-WiFi-Library). Flag `-DIMPROV_WIFI_BLE_ENABLED` selects BLE transport.

## Per-board flags

- `LED_BLUE` — status LED GPIO (required).
- `BUTTON_PIN` — optional; enables OneButton cred-reset.

## Provision

1. Flash + boot. Wait 10s.
2. Open https://www.improv-wifi.com/ in Chrome/Edge on a BLE host.
3. Connect to advertised device, enter SSID/password.
4. On success, browse to device IP for info page.
