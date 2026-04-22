#ifndef THIRD_PARTY_BLINK_PUBLIC_COMMON_FINGERPRINT_SINGLETON_FINGERPRINT_H_
#define THIRD_PARTY_BLINK_PUBLIC_COMMON_FINGERPRINT_SINGLETON_FINGERPRINT_H_

#include <string>

#include "third_party/blink/public/common/fingerprint/fingerprint.h"
#include "third_party/blink/public/common/common_export.h"


namespace base {

class BLINK_COMMON_EXPORT SingletonFingerprint {
public:
    SingletonFingerprint();
    SingletonFingerprint(const SingletonFingerprint& other);
    SingletonFingerprint& operator=(const SingletonFingerprint& other);
    ~SingletonFingerprint();

    bool GetInit() const { return init_; }
    const blink::fp::Fingerprint& GetFingerprint() const {
        return fingerprint_;
    }

    bool setFontDefaultInfo(int index, std::string name){
        fingerprint_.font.defaultId = index;
        fingerprint_.font.defaultName = name;
        return true;
    }

    static bool GetInit(SingletonFingerprint* singletonFingerprint){
        if(!singletonFingerprint->GetInit()){
            return false;
        }
        if(singletonFingerprint->GetFingerprint().init<2){
            return false;
        }
        return true;

    }
    static bool Init();
    static bool Init(const blink::fp::Fingerprint& inputFingerprint);
    static SingletonFingerprint* ForCurrentProcess();
    static bool HasInstance() { return singletonFingerprint_ != nullptr; }


private:
    bool init_ = false;
    blink::fp::Fingerprint fingerprint_;
    static SingletonFingerprint* singletonFingerprint_;
};
}  // namespace base
#endif  // THIRD_PARTY_BLINK_PUBLIC_COMMON_FINGERPRINT_SINGLETON_FINGERPRINT_H_
