// CrossGuard patch: MAC address spoofing
// Patch net::GetNetworkListImpl() to mask MAC addresses when
// macAddress fingerprint is active (type > 1).
// Applied to: net/base/network_interfaces_win.cc
//
// HOW TO APPLY:
// 1. Add include at the top of network_interfaces_win.cc:
//    #include "third_party/blink/public/common/fingerprint/singleton_fingerprint.h"
//
// 2. Replace the MAC address extraction block (around line 162-169)
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
    if (base::SingletonFingerprint::HasInstance()) {
      base::SingletonFingerprint* t_sfp =
          base::SingletonFingerprint::ForCurrentProcess();
      if (base::SingletonFingerprint::GetInit(t_sfp)) {
        blink::fp::Fingerprint t_fp = t_sfp->GetFingerprint();
        if (t_fp.macAddress.type > 1) {
          t_mask_mac = true;
        }
      }
    }
    mac_address.emplace();
    if (adapter->PhysicalAddressLength == mac_address->size()) {
      if (t_mask_mac) {
        // Replace real MAC with zeros
        mac_address->fill(0x00);
      } else {
        std::copy_n(reinterpret_cast<const uint8_t*>(adapter->PhysicalAddress),
                    mac_address->size(), mac_address->begin());
      }
    } else {
      mac_address.reset();
    }
// === END PATCH ===
