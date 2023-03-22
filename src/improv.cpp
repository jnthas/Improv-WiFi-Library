#include "improv.h"

namespace improv {

//TODO move this method to ImprovWiFiLibrary too and remove this file
std::vector<uint8_t> build_rpc_response(Command command, const std::vector<std::string> &datum, bool add_checksum) {
  std::vector<uint8_t> out;
  uint32_t length = 0;
  out.push_back(command);
  for (const auto &str : datum) {
    uint8_t len = str.length();
    length += len + 1;
    out.push_back(len);
    out.insert(out.end(), str.begin(), str.end());
  }
  out.insert(out.begin() + 1, length);

  if (add_checksum) {
    uint32_t calculated_checksum = 0;

    for (uint8_t byte : out) {
      calculated_checksum += byte;
    }
    out.push_back(calculated_checksum);
  }
  return out;
}

#ifdef ARDUINO
std::vector<uint8_t> build_rpc_response(Command command, const std::vector<String> &datum, bool add_checksum) {
  std::vector<uint8_t> out;
  uint32_t length = 0;
  out.push_back(command);
  for (const auto &str : datum) {
    uint8_t len = str.length();
    length += len;
    out.push_back(len);
    out.insert(out.end(), str.begin(), str.end());
  }
  out.insert(out.begin() + 1, length);

  if (add_checksum) {
    uint32_t calculated_checksum = 0;

    for (uint8_t byte : out) {
      calculated_checksum += byte;
    }
    out.push_back(calculated_checksum);
  }
  return out;
}
#endif  // ARDUINO

}  // namespace improv