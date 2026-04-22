
#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_MEDIA_STREAM_TRACK_IMPL_FP_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_MEDIA_STREAM_TRACK_IMPL_FP_H_

#include "third_party/blink/public/common/fingerprint/singleton_fingerprint.h"
#include "third_party/blink/public/common/fingerprint/fingerprint.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"


#include "third_party/blink/renderer/bindings/modules/v8/v8_media_track_settings.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_media_track_capabilities.h"
          
WTF::String fpLabel(WTF::String label,bool isAudio){

  base::SingletonFingerprint* t_singletonFingerprint = base::SingletonFingerprint::ForCurrentProcess();
  blink::fp::Fingerprint  t_fingerprint = t_singletonFingerprint->GetFingerprint();
  if(base::SingletonFingerprint::GetInit(t_singletonFingerprint) && t_fingerprint.mediaEquipment.type > 1 ){
      std::vector< blink::fp::MediaEquipmentInfo> list=t_fingerprint.mediaEquipment.list;
      for(unsigned i=0;i<list.size();i++){
        blink::fp::MediaEquipmentInfo t_equipmentInfo=list[i];
        if((isAudio&&t_equipmentInfo.type==0)||(!isAudio&&t_equipmentInfo.type==1)){
           return String::FromUTF8(t_equipmentInfo.label);
        }
      }
  }
  return label;
}

void fpGetSettings(blink::MediaTrackSettings* settings,bool isAudio,bool groupIdNotNull){
  base::SingletonFingerprint* t_singletonFingerprint = base::SingletonFingerprint::ForCurrentProcess();
  blink::fp::Fingerprint  t_fingerprint = t_singletonFingerprint->GetFingerprint();
  if(base::SingletonFingerprint::GetInit(t_singletonFingerprint) && t_fingerprint.mediaEquipment.type > 1 ){
      std::vector< blink::fp::MediaEquipmentInfo> list=t_fingerprint.mediaEquipment.list;
      for(unsigned i=0;i<list.size();i++){
        blink::fp::MediaEquipmentInfo t_equipmentInfo=list[i];
        if((isAudio&&t_equipmentInfo.type==0)||(!isAudio&&t_equipmentInfo.type==1)){
           settings->setDeviceId(String::FromUTF8(t_equipmentInfo.deviceId));
          if(groupIdNotNull){
              settings->setGroupId(String::FromUTF8(t_equipmentInfo.groupId));
          }
          return ;
        }
      }   
  }

}


void fpGetCapabilities(blink::MediaTrackCapabilities* capabilities,bool isAudio,bool groupIdNotNull){
  base::SingletonFingerprint* t_singletonFingerprint = base::SingletonFingerprint::ForCurrentProcess();
  blink::fp::Fingerprint  t_fingerprint = t_singletonFingerprint->GetFingerprint();
  if(base::SingletonFingerprint::GetInit(t_singletonFingerprint) && t_fingerprint.mediaEquipment.type > 1 ){
      std::vector< blink::fp::MediaEquipmentInfo> list=t_fingerprint.mediaEquipment.list;
      for(unsigned i=0;i<list.size();i++){
        blink::fp::MediaEquipmentInfo t_equipmentInfo=list[i];
        if((isAudio&&t_equipmentInfo.type==0)||(!isAudio&&t_equipmentInfo.type==1)){
           capabilities->setDeviceId(String::FromUTF8(t_equipmentInfo.deviceId));
          if(groupIdNotNull){
              capabilities->setGroupId(String::FromUTF8(t_equipmentInfo.groupId));
          }
          return ;
        }
      }
  }

}



#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_MEDIA_STREAM_TRACK_IMPL_FP_H_
