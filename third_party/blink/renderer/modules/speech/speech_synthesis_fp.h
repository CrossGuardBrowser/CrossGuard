#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_SPEECH_SPEECH_SYNTHESIS_VOICE_FP_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_SPEECH_SPEECH_SYNTHESIS_VOICE_FP_H_

#include "third_party/blink/public/common/fingerprint/singleton_fingerprint.h"
#include "third_party/blink/public/common/fingerprint/fingerprint.h"

#include "third_party/blink/public/mojom/speech/speech_synthesis.mojom-blink-forward.h"
#include "third_party/blink/renderer/modules/speech/speech_synthesis_voice.h"
#include "third_party/blink/renderer/platform/heap/collection_support/heap_vector.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/platform/heap/member.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"


bool fpGetVoices(
        blink::HeapVector<blink::Member<blink::SpeechSynthesisVoice>>& voices,
        blink::HeapVector<blink::Member<blink::SpeechSynthesisVoice>>& fpVoices){


    base::SingletonFingerprint* t_singletonFingerprint = base::SingletonFingerprint::ForCurrentProcess();
    blink::fp::Fingerprint  t_fingerprint = t_singletonFingerprint->GetFingerprint();
    if(base::SingletonFingerprint::GetInit(t_singletonFingerprint) && t_fingerprint.speechVoices.type > 1 ){
            if(fpVoices.size() > 0){
                return true;
            }
            std::vector<blink::fp::SpeechVoicesInfo> list =t_fingerprint.speechVoices.list;
            for (std::size_t i = 0; i < list.size(); ++i) {
                const blink::fp::SpeechVoicesInfo& info = list[i];
                
                blink::mojom::blink::SpeechSynthesisVoicePtr prt0= blink::mojom::blink::SpeechSynthesisVoice::New();
                prt0->voice_uri=String::FromUTF8(info.voiceUri);
                prt0->name = String::FromUTF8(info.name);
                prt0->lang =String::FromUTF8(info.lang);
                prt0->is_local_service = info.isLocalService;
                prt0->is_default = info.isDefault;
                fpVoices.push_back(blink::MakeGarbageCollected<blink::SpeechSynthesisVoice>(std::move(prt0)));
            }
            return true;
    }
    return false;
}

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_SPEECH_SPEECH_SYNTHESIS_VOICE_FP_H_