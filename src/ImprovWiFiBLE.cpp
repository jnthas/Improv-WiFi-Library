// Only compile BLE support if explicitly enabled
#ifdef IMPROV_WIFI_BLE_ENABLED

#include "ImprovWiFiBLE.h"

#if defined(ARDUINO_ARCH_ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
    #include <WiFi.h>
#endif

// ==== ctor/dtor ====
ImprovWiFiBLE::~ImprovWiFiBLE() {
    // Minimal cleanup; BLEDevice::deinit() can be called by app if desired
}

// ==== public API (matching ImprovWiFi) ====

void ImprovWiFiBLE::setDeviceInfo(ImprovTypes::ChipFamily chipFamily,
                                  const char *firmwareName,
                                  const char *firmwareVersion,
                                  const char *deviceName,
                                  const char *deviceUrl) {
    if (server_) return; // already initialized
    chip_ = chipFamily;
    firmware_name_ = firmwareName ? firmwareName : "";
    firmware_version_ = firmwareVersion ? firmwareVersion : "";
    device_name_ = deviceName ? deviceName : "";
    device_friendly_name_ = deviceName ? deviceName : "";
    device_url_ = deviceUrl ? deviceUrl : "";

    // Initialize BLE and set up the Improv service
    BLEDevice::init(device_name_.c_str());
#if defined(ESP_PWR_LVL_P9)
    BLEDevice::setPower(ESP_PWR_LVL_P9);
#endif

#if defined(CONFIG_NIMBLE_ENABLED)
    // Prefer a robust address type on ESP32
    BLEDevice::setOwnAddrType(BLE_OWN_ADDR_RPA_RANDOM_DEFAULT);
#endif

    server_ = BLEDevice::createServer();
    server_->setCallbacks(this);

    service_ = server_->createService(SVC_UUID);

    ch_state_ = service_->createCharacteristic(
                    CHAR_STATE_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    ch_error_ = service_->createCharacteristic(
                    CHAR_ERROR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    ch_rpc_cmd_ = service_->createCharacteristic(
                      CHAR_RPC_CMD_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    ch_rpc_res_ = service_->createCharacteristic(
                      CHAR_RPC_RES_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    ch_caps_ =
        service_->createCharacteristic(CHAR_CAPS_UUID, BLECharacteristic::PROPERTY_READ);

    ch_rpc_cmd_->setCallbacks(this);

    // Seed characteristic values WITHOUT notify (no subscribers yet,
    // service not started — notify() would assert/null-deref).
    ch_caps_->setValue(&caps_, 1);
    ch_error_->setValue(&error_, 1);
    ch_state_->setValue(&state_, 1);

    service_->start();

    adv_ = BLEDevice::getAdvertising();

    // Build primary advertisement (UUID + Service Data + maybe short name)
    BLEAdvertisementData advData = buildAdvData(state_, caps_);
    adv_->setAdvertisementData(advData);

    // Optional: put the full name in the scan response (keeps primary ADV small)
    if (!device_name_.isEmpty()) {
        BLEAdvertisementData scan;
        scan.setName(device_name_.c_str());
        adv_->setScanResponseData(scan);
    }

    advertiseNow();
}

void ImprovWiFiBLE::setDeviceInfo(ImprovTypes::ChipFamily chipFamily,
                                  const char *firmwareName,
                                  const char *firmwareVersion,
                                  const char *deviceName) {
    setDeviceInfo(chipFamily, firmwareName, firmwareVersion, deviceName, nullptr);
}

void ImprovWiFiBLE::onImprovError(OnImprovError *errorCallback) {
    onImprovErrorCallback_ = errorCallback;
}

void ImprovWiFiBLE::onImprovConnected(OnImprovConnected *connectedCallback) {
    onImprovConnectedCallback_ = connectedCallback;
}

void ImprovWiFiBLE::onImprovIdentify(OnImprovIdentify *identifyCallback) {
    onImprovIdentifyCallback_ = identifyCallback;
}

void ImprovWiFiBLE::setCustomConnectWiFi(
    CustomConnectWiFi *connectWiFiCallBack) {
    customConnectWiFiCallback_ = connectWiFiCallBack;
}

bool ImprovWiFiBLE::tryConnectToWifi(const char *ssid, const char *password) {
    // Defaults match simple behavior of the serial variant
    return tryConnectToWifi(ssid, password, /*delayMs*/ 500, /*maxAttempts*/ 20);
}

bool ImprovWiFiBLE::tryConnectToWifi(const char *ssid, const char *password,
                                     uint32_t delayMs, uint8_t maxAttempts) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    WiFi.begin(ssid, password);

    for (uint8_t i = 0; i < maxAttempts; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        delay(delayMs);
    }
    return (WiFi.status() == WL_CONNECTED);
}

bool ImprovWiFiBLE::isAdvertising() {
    return (server_ && service_ && adv_); // espressif Nimble in Arduino lacks isAdvertising()
}

bool ImprovWiFiBLE::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// ==== BLE callbacks ====

void ImprovWiFiBLE::onDisconnect(BLEServer *) {
    if (adv_) {
        advertiseNow();
    }
}

void ImprovWiFiBLE::onWrite(BLECharacteristic *c) {
    if (c != ch_rpc_cmd_)
        return;
    const String v = c->getValue();
    if (v.length() < 3) {
        reportError(ERR_BAD_PACKET, ImprovTypes::Error::ERROR_INVALID_RPC);
        return;
    }
    handleRpc(reinterpret_cast<const uint8_t *>(v.c_str()), v.length());
}

// ==== internal helpers / RPCs ====

void ImprovWiFiBLE::handleRpc(const uint8_t *data, size_t len) {
    const uint8_t cmd = data[0];
    const uint8_t declared_len = data[1];

    if (declared_len + 3 != len) {
        reportError(ERR_BAD_PACKET, ImprovTypes::Error::ERROR_INVALID_RPC);
        return;
    }

    const uint8_t cs = data[len - 1];
    if (checksumLSB(data, len - 1) != cs) {
        reportError(ERR_BAD_PACKET, ImprovTypes::Error::ERROR_INVALID_RPC);
        return;
    }

    switch (cmd) {
        case 0x01: // Send Wi-Fi
            rpcSendWifi(&data[2], declared_len);
            break;
        case 0x02: // Identify
            rpcIdentify();
            break;
        default:
            reportError(ERR_UNKNOWN_CMD, ImprovTypes::Error::ERROR_INVALID_RPC);
            break;
    }
}

void ImprovWiFiBLE::rpcSendWifi(const uint8_t *p, size_t n) {
    if (n < 2) {
        reportError(ERR_BAD_PACKET, ImprovTypes::Error::ERROR_INVALID_RPC);
        return;
    }

    // Parse SSID
    const uint8_t ssid_len = p[0];
    if (1 + ssid_len > n) {
        reportError(ERR_BAD_PACKET, ImprovTypes::Error::ERROR_INVALID_RPC);
        return;
    }
    String ssid;
    ssid.reserve(ssid_len);
    for (uint8_t i = 0; i < ssid_len; i++)
        ssid += (char)p[1 + i];

    // Parse password
    size_t pos = 1 + ssid_len;
    if (pos >= n) {
        reportError(ERR_BAD_PACKET, ImprovTypes::Error::ERROR_INVALID_RPC);
        return;
    }
    const uint8_t pass_len = p[pos];
    if (pos + 1 + pass_len > n) {
        reportError(ERR_BAD_PACKET, ImprovTypes::Error::ERROR_INVALID_RPC);
        return;
    }
    String pass;
    pass.reserve(pass_len);
    for (uint8_t i = 0; i < pass_len; i++)
        pass += (char)p[pos + 1 + i];

    updateState(STATE_PROVISIONING);

    // Attempt connection (custom callback first, like ImprovWiFi)
    bool ok = false;
    if (customConnectWiFiCallback_) {
        ok = customConnectWiFiCallback_(ssid.c_str(), pass.c_str());
    } else {
        ok = tryConnectToWifi(ssid.c_str(), pass.c_str());
    }

    if (!ok) {
        reportError(ERR_CONNECT, ImprovTypes::Error::ERROR_UNABLE_TO_CONNECT);
        // Return to ready so clients can try again
        updateState(STATE_AUTHORIZED);
        advertiseNow();
        return;
    }

    // Success path
    if (onImprovConnectedCallback_) {
        onImprovConnectedCallback_(ssid.c_str(), pass.c_str());
    }

    updateState(STATE_PROVISIONED);
    advertiseNow();

    // Send device URL back (mirrors serial semantics)
    sendDeviceUrl();

    delay(250); // allow clients to read updated chars
}

void ImprovWiFiBLE::rpcIdentify() {
    if (onImprovIdentifyCallback_)
        onImprovIdentifyCallback_();
}

void ImprovWiFiBLE::sendDeviceUrl() {
    // RPC result framing: [last_cmd=0x01][len][url_len][url...][checksum]
    std::vector<uint8_t> buf;
    buf.reserve(4 + device_url_.length());
    buf.push_back(0x01);

    const std::string url = device_url_.c_str();
    const uint8_t url_len = static_cast<uint8_t>(url.size());
    const uint8_t payload_len = 1 + url_len; // [url_len][url]
    buf.push_back(payload_len);
    buf.push_back(url_len);
    buf.insert(buf.end(), url.begin(), url.end());

    buf.push_back(checksumLSB(buf.data(), buf.size()));
    ch_rpc_res_->setValue(buf.data(), buf.size());
    ch_rpc_res_->notify();
}

void ImprovWiFiBLE::updateState(uint8_t s) {
    state_ = s;
    if (ch_state_) {
        ch_state_->setValue(&state_, 1);
        ch_state_->notify();
    }
}

void ImprovWiFiBLE::updateError(uint8_t e) {
    error_ = e;
    if (ch_error_) {
        ch_error_->setValue(&error_, 1);
        ch_error_->notify();
    }
}

void ImprovWiFiBLE::updateCaps(uint8_t caps) {
    caps_ = caps;
    if (ch_caps_) {
        ch_caps_->setValue(&caps_, 1);
    }
}

void ImprovWiFiBLE::reportError(uint8_t bleErr, ImprovTypes::Error apiErr) {
    updateError(bleErr);
    if (onImprovErrorCallback_)
        onImprovErrorCallback_(apiErr);
}

BLEAdvertisementData ImprovWiFiBLE::buildAdvData(uint8_t state,
        uint8_t caps) {
    BLEAdvertisementData ad;

    // Flags: LE General Discoverable + BR/EDR not supported
    ad.setFlags(ESP_BLE_ADV_FLAG_BREDR_NOT_SPT | ESP_BLE_ADV_FLAG_GEN_DISC);

    // REQUIRED: 128-bit Improv Service UUID in the same primary ADV as Service
    // Data (not scan response)
    ad.setCompleteServices(BLEUUID(SVC_UUID));

    // REQUIRED: Service Data UUID = 0x4677 with payload [0x77, 0x46, state, caps, 0,0,0,0]
    // 0x77, 0x46 are the Improv protocol magic bytes - REQUIRED for recognition
    uint8_t payload[8] = {0x77, 0x46, state, caps, 0x00, 0x00, 0x00, 0x00};
    ad.setServiceData(BLEUUID((uint16_t)SERVICE_DATA_UUID_16),
                      String((const char *)payload, sizeof(payload)));

    // Optional: a short name in the primary ADV helps some scanners; keep it
    // short to stay under 31B
    if (!device_name_.isEmpty()) {
        // Short name so we still fit UUID + service data
        ad.setShortName(device_name_.c_str());
    }

    return ad;
}

void ImprovWiFiBLE::advertiseNow() {
    if (!adv_)
        return;
    adv_->stop();

    // Rebuild adv data with current state/caps; preserve scan response (name)
    BLEAdvertisementData advData = buildAdvData(state_, caps_);
    adv_->setAdvertisementData(advData);

    adv_->start();
}

uint8_t ImprovWiFiBLE::checksumLSB(const uint8_t *data, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++)
        sum += data[i];
    return static_cast<uint8_t>(sum & 0xFF);
}

#endif // IMPROV_WIFI_BLE_ENABLED
