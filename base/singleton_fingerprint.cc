#include "base/singleton_fingerprint.h"

#include <json/json.h>

#include "base/check_op.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "third_party/blink/public/common/fingerprint/fingerprint.h"
#include "third_party/blink/public/common/user_agent/user_agent_metadata.h"


namespace base {

SingletonFingerprint* SingletonFingerprint::singletonFingerprint_ = nullptr;

namespace {

// 实例持有改用 base::NoDestructor 避免裸 new 泄漏。
// 第一次 Init 时构造,之后所有调用复用同一对象。
SingletonFingerprint* GetOrCreateInstance() {
    static base::NoDestructor<SingletonFingerprint> instance;
    return instance.get();
}

}  // namespace

SingletonFingerprint::SingletonFingerprint() = default;
SingletonFingerprint::SingletonFingerprint(const SingletonFingerprint& other) = default;
SingletonFingerprint& SingletonFingerprint::operator=(const SingletonFingerprint& other) = default;
SingletonFingerprint::~SingletonFingerprint() = default;

// static
bool SingletonFingerprint::Init() {
    if (singletonFingerprint_) {
        return false;
    }
    singletonFingerprint_ = GetOrCreateInstance();
    blink::fp::Fingerprint fingerprint;
    fingerprint.init = 0;
    singletonFingerprint_->fingerprint_ = fingerprint;
    singletonFingerprint_->init_ = true;
    return true;
}

// static
bool SingletonFingerprint::Init(const blink::fp::Fingerprint& inputFingerprint) {
    if (!singletonFingerprint_) {
        singletonFingerprint_ = GetOrCreateInstance();
    }
    singletonFingerprint_->fingerprint_ = inputFingerprint;
    singletonFingerprint_->init_ = true;
    return true;
}

// static
bool SingletonFingerprint::HasInstance() {
    return singletonFingerprint_ != nullptr;
}

// static
SingletonFingerprint* SingletonFingerprint::ForCurrentProcess() {
    if (!singletonFingerprint_) {
        // 历史版本会 CHECK 崩溃;改为返回 nullptr 让调用方安全降级。
        // 第一次出现时打印警告,后续静默以免刷屏。
        static bool warned = false;
        if (!warned) {
            warned = true;
            LOG(WARNING) << "SingletonFingerprint::ForCurrentProcess() before Init(); "
                            "renderer will run without fingerprint spoofing.";
        }
        return nullptr;
    }
    return singletonFingerprint_;
}

// static
bool SingletonFingerprint::TryGet(blink::fp::Fingerprint* out) {
    if (!out) {
        return false;
    }
    SingletonFingerprint* sfp = ForCurrentProcess();
    if (!GetInit(sfp)) {
        return false;
    }
    *out = sfp->GetFingerprint();
    return true;
}

}  // namespace base
