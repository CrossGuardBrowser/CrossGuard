#ifndef COMPONENTS_LANGUAGE_CORE_BROWSER_FP_LANGUAGE_H_
#define COMPONENTS_LANGUAGE_CORE_BROWSER_FP_LANGUAGE_H_

#include "third_party/blink/public/common/fingerprint/fingerprint.h"
#include "base/singleton_fingerprint.h"

inline void setLlanguage(PrefService* user_prefs) {
    // --lang 已在 chrome_main.cc（FetchHttpResponse 之后）按 interfaceLanguage 注入，
    // 此处仅同步 Accept-Language 偏好，影响 HTTP Accept-Language 头与
    // navigator.languages 的后续条目（navigator.language 主值由 --lang 决定）。
    base::SingletonFingerprint* t_singletonFingerprint = base::SingletonFingerprint::ForCurrentProcess();
    blink::fp::Fingerprint  t_fingerprint = t_singletonFingerprint->GetFingerprint();
        if(base::SingletonFingerprint::GetInit(t_singletonFingerprint) && t_fingerprint.language.type>1){
            std::string languages;
            for (const std::string& t_language : t_fingerprint.language.languages) {
                languages.append(t_language);
                languages.append(",");
            }
            if (!languages.empty()) {
                languages = languages.substr(0, languages.length() - 1);
                user_prefs->SetString(language::prefs::kAcceptLanguages,languages);
            }
        }

}
#endif // COMPONENTS_LANGUAGE_CORE_BROWSER_FP_LANGUAGE_H_