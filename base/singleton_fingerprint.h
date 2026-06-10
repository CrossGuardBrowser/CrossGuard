#ifndef BASE_SINGLETON_FINGERPRINT_H_
#define BASE_SINGLETON_FINGERPRINT_H_

#include <string>
#include "base/base_export.h"
#include "base/debug/debugging_buildflags.h"
#include "third_party/blink/public/common/fingerprint/fingerprint.h"


namespace base {

class BASE_EXPORT SingletonFingerprint {
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

    // 旧 API 保留: 检查给定实例是否已 Init 且 fingerprint.init >= 2。
    static bool GetInit(SingletonFingerprint* singletonFingerprint){
        if (!singletonFingerprint) {
            return false;
        }
        if (!singletonFingerprint->GetInit()) {
            return false;
        }
        if (singletonFingerprint->GetFingerprint().init < 2) {
            return false;
        }
        return true;
    }

    static bool Init();
    static bool Init(const blink::fp::Fingerprint& inputFingerprint);

    // 查询单例是否已经创建过(不论是否 Init 成功)。
    // 新增,便于调用方在 ForCurrentProcess 前做轻量级判断。
    static bool HasInstance();

    // 返回单例指针;若尚未 Init() 则返回 nullptr 并发出 LOG(WARNING)。
    // 旧版会 CHECK 崩溃,新版改为安全降级,允许渲染进程在没有指纹配置时继续运行。
    static SingletonFingerprint* ForCurrentProcess();

    // 一站式安全访问:同时做 HasInstance + GetInit 校验,
    // 命中时把当前 Fingerprint 复制到 out 并返回 true。
    static bool TryGet(blink::fp::Fingerprint* out);

private:
    bool init_ = false;
    blink::fp::Fingerprint fingerprint_;
    static SingletonFingerprint* singletonFingerprint_;
};

}  // namespace base
#endif  // BASE_SINGLETON_FINGERPRINT_H_
