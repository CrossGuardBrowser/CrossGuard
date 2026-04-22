// CrossGuard patch: Hostname spoofing
// Applied to: net/base/network_interfaces.cc
//
// HOW TO APPLY:
// 1. Add include at the top of network_interfaces.cc:
//    #include "third_party/blink/public/common/fingerprint/singleton_fingerprint.h"
//
// 2. Replace GetHostName() implementation with the patched version below:

// === ORIGINAL CODE (to be replaced) ===
// std::string GetHostName() {
// #if BUILDFLAG(IS_WIN)
//   EnsureWinsockInit();
// #endif
//   char buffer[256];
//   int result = gethostname(buffer, sizeof(buffer));
//   if (result != 0) {
//     DVLOG(1) << "gethostname() failed with " << result;
//     buffer[0] = '\0';
//   }
//   return std::string(buffer);
// }

// === PATCHED CODE (replace with) ===
std::string GetHostName() {
  // CrossGuard: Check for hostname override from fingerprint config
  if (base::SingletonFingerprint::HasInstance()) {
    base::SingletonFingerprint* t_singletonFingerprint =
        base::SingletonFingerprint::ForCurrentProcess();
    if (base::SingletonFingerprint::GetInit(t_singletonFingerprint)) {
      blink::fp::Fingerprint t_fingerprint =
          t_singletonFingerprint->GetFingerprint();
      if (t_fingerprint.deviceName.type > 1 &&
          !t_fingerprint.deviceName.value.empty()) {
        return t_fingerprint.deviceName.value;
      }
    }
  }

#if BUILDFLAG(IS_WIN)
  EnsureWinsockInit();
#endif

  // Host names are limited to 255 bytes.
  char buffer[256];
  int result = gethostname(buffer, sizeof(buffer));
  if (result != 0) {
    DVLOG(1) << "gethostname() failed with " << result;
    buffer[0] = '\0';
  }
  return std::string(buffer);
}
// === END PATCH ===
