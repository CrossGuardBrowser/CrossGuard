// CrossGuard patch: MAC address spoofing
// Patch net::GetNetworkListImpl() to mask MAC addresses when
// macAddress fingerprint is active (type > 1).
//
// type > 1 时:
//   - macAddress.value 非空且能解析为 6 字节 (XX:XX:XX:XX:XX:XX) → 使用该 MAC
//   - 否则 → 全部填 0x00 (向后兼容旧行为)
//
// Applied to: net/base/network_interfaces_win.cc
//
// HOW TO APPLY:
// 1. Add includes at the top of network_interfaces_win.cc:
//    #include <array>
//    #include <cctype>
//    #include <cstdint>
//    #include "base/singleton_fingerprint.h"
//    #include "third_party/blink/public/common/fingerprint/fingerprint.h"
//
// 2. Add helper above GetNetworkListImpl() (or in anonymous namespace):
//
//    namespace {
//    // 解析 "XX:XX:XX:XX:XX:XX" 或 "XX-XX-XX-XX-XX-XX",成功时写入 out 并返回 true。
//    bool ParseSpoofedMac(const std::string& s, std::array<uint8_t, 6>* out) {
//      if (s.size() != 17) return false;
//      for (int i = 0; i < 6; ++i) {
//        int hi_pos = i * 3;
//        int lo_pos = hi_pos + 1;
//        int sep_pos = hi_pos + 2;
//        if (sep_pos < 17 && s[sep_pos] != ':' && s[sep_pos] != '-') return false;
//        char hi = s[hi_pos];
//        char lo = s[lo_pos];
//        auto hexval = [](char c) -> int {
//          if (c >= '0' && c <= '9') return c - '0';
//          if (c >= 'a' && c <= 'f') return 10 + c - 'a';
//          if (c >= 'A' && c <= 'F') return 10 + c - 'A';
//          return -1;
//        };
//        int h = hexval(hi), l = hexval(lo);
//        if (h < 0 || l < 0) return false;
//        (*out)[i] = static_cast<uint8_t>((h << 4) | l);
//      }
//      return true;
//    }
//    }  // namespace
//
// 3. Replace the MAC address extraction block (around line 162-169)
//    in GetNetworkListImpl() with the patched version below:

// === ORIGINAL CODE (to be replaced) ===
//    std::optional<Eui48MacAddress> mac_address;
//    mac_address.emplace();
//    if (adapter->PhysicalAddressLength == mac_address->size()) {
//      std::copy_n(reinterpret_cast<const uint8_t*>(adapter->PhysicalAddress),
//                  mac_address->size(), mac_address->begin());
//    } else {
//      mac_address.reset();
//    }

// === PATCHED CODE (replace with) ===
    std::optional<Eui48MacAddress> mac_address;
    // CrossGuard: MAC address spoofing
    bool t_mask_mac = false;
    std::string t_mac_value;
    if (base::SingletonFingerprint::HasInstance()) {
      base::SingletonFingerprint* t_sfp =
          base::SingletonFingerprint::ForCurrentProcess();
      if (base::SingletonFingerprint::GetInit(t_sfp)) {
        const blink::fp::Fingerprint& t_fp = t_sfp->GetFingerprint();
        if (t_fp.macAddress.type > 1) {
          t_mask_mac = true;
          t_mac_value = t_fp.macAddress.value;
        }
      }
    }
    mac_address.emplace();
    if (adapter->PhysicalAddressLength == mac_address->size()) {
      if (t_mask_mac) {
        std::array<uint8_t, 6> spoofed{};
        if (!t_mac_value.empty() && ParseSpoofedMac(t_mac_value, &spoofed)) {
          std::copy_n(spoofed.begin(), mac_address->size(), mac_address->begin());
        } else {
          // 解析失败或未提供 → 全 0 (旧行为)
          mac_address->fill(0x00);
        }
      } else {
        std::copy_n(reinterpret_cast<const uint8_t*>(adapter->PhysicalAddress),
                    mac_address->size(), mac_address->begin());
      }
    } else {
      mac_address.reset();
    }
// === END PATCH ===
