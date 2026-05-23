#pragma once

#include <Arduino.h>
#include <Preferences.h>

class CredStore {
public:
	static void saveWiFiCredentials(const String &ssid, const String &pass) {
		log_d("Explicitly saving credentials to Preferences...");

		Preferences preferences;
		preferences.begin(prefNamespace(), false);
		preferences.putString(keySsid(), ssid);
		preferences.putString(keyPass(), pass);
		preferences.end();

		log_d("Credentials committed successfully.");
	}

	static void clearWiFiCredentials() {
		log_d("Clearing WiFi credentials container...");

		Preferences preferences;
		preferences.begin(prefNamespace(), false);
		preferences.clear();
		preferences.end();

		log_d("Preferences namespace wiped clean.");
	}

	static bool loadWiFiCredentials(String &ssid, String &password) {
		Preferences preferences;
		if (!preferences.begin(prefNamespace(), true)) {
			log_d("[Preferences] Failed to open namespace");
			return false;
		}

		if (!preferences.isKey(keySsid())) {
			preferences.end();
			return false;
		}

		ssid = preferences.getString(keySsid(), "");
		password = preferences.getString(keyPass(), "");
		preferences.end();

		return ssid.length() > 0;
	}

private:
	static constexpr const char *prefNamespace() {
		return "wifi-creds";
	}

	static constexpr const char *keySsid() {
		return "ssid";
	}

	static constexpr const char *keyPass() {
		return "password";
	}
};

inline void saveWiFiCredentials(const String &ssid, const String &pass) {
	CredStore::saveWiFiCredentials(ssid, pass);
}

inline void clearWiFiCredentials() {
	CredStore::clearWiFiCredentials();
}

inline bool loadWiFiCredentials(String &ssid, String &password) {
	return CredStore::loadWiFiCredentials(ssid, password);
}

