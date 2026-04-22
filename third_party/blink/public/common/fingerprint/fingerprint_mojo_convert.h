#ifndef THIRD_PARTY_BLINK_PUBLIC_COMMON_FINGERPRINT_FINGERPRINT_MOJO_CONVERT_H_
#define THIRD_PARTY_BLINK_PUBLIC_COMMON_FINGERPRINT_FINGERPRINT_MOJO_CONVERT_H_

#include "third_party/blink/public/common/fingerprint/fingerprint.h"
#include "third_party/blink/public/mojom/fingerprint/fingerprint.mojom.h"

namespace blink {
namespace fp {

inline mojom::ConsistencyPtr ToMojo(const Consistency& v) {
  return mojom::Consistency::New(v.type, v.urlList);
}

inline mojom::UAPtr ToMojo(const UA& v) {
  return mojom::UA::New(v.type, v.userAgent);
}

inline mojom::UaMetadataPtr ToMojo(const UaMetadata& v) {
  return mojom::UaMetadata::New(v.type, v.userAgentMetadata);
}

inline mojom::WebRTCPtr ToMojo(const WebRTC& v) {
  return mojom::WebRTC::New(v.type, v.privateIp, v.publicIp);
}

inline mojom::TimeZonePtr ToMojo(const TimeZone& v) {
  return mojom::TimeZone::New(v.type, v.gmt);
}

inline mojom::LocationPtr ToMojo(const Location& v) {
  return mojom::Location::New(v.type, v.permissions, v.latitude, v.longitude, v.accuracy);
}

inline mojom::LanguagePtr ToMojo(const Language& v) {
  return mojom::Language::New(v.type, v.interfaceLanguage, v.languages);
}

inline mojom::ResolutionPtr ToMojo(const Resolution& v) {
  return mojom::Resolution::New(v.type, v.windowWidth, v.windowHeight, v.monitorWidth, v.monitorHeight);
}

inline mojom::FontInfoPtr ToMojo(const FontInfo& v) {
  return mojom::FontInfo::New(v.id, v.width, v.height,
      v.actualBoundingBoxAscent, v.actualBoundingBoxDescent,
      v.actualBoundingBoxLeft, v.actualBoundingBoxRight,
      v.fontBoundingBoxAscent, v.fontBoundingBoxDescent,
      v.filePaths);
}

inline mojom::FontPtr ToMojo(const Font& v) {
  std::vector<std::pair<std::string, mojom::FontInfoPtr>> font_map_entries;
  for (auto& [k, val] : v.fontMap) {
    font_map_entries.push_back(std::make_pair(k, ToMojo(val)));
  }
  base::flat_map<std::string, mojom::FontInfoPtr> font_map(std::move(font_map_entries));

  std::vector<std::pair<int32_t, std::string>> font_id_entries;
  for (auto& [k, val] : v.fontIdMap) {
    font_id_entries.push_back(std::make_pair(static_cast<int32_t>(k), val));
  }
  base::flat_map<int32_t, std::string> font_id_map(std::move(font_id_entries));

  return mojom::Font::New(v.type, v.maxId, v.defaultId, v.defaultName, v.defaultPaths,
      std::move(font_map), std::move(font_id_map));
}

inline mojom::ColoredPointPtr ToMojo(const ColoredPoint& v) {
  return mojom::ColoredPoint::New(v.row, v.column, v.red, v.green, v.blue, v.alpha);
}

inline mojom::CanvasPtr ToMojo(const Canvas& v) {
  std::vector<mojom::ColoredPointPtr> points;
  for (const auto& p : v.coloredPointList) {
    points.push_back(ToMojo(p));
  }
  return mojom::Canvas::New(v.type, std::move(points));
}

inline mojom::WebGLDevicePtr ToMojo(const WebGLDevice& v) {
  return mojom::WebGLDevice::New(v.type, v.vendors, v.renderer, v.gpuVendors, v.gpuArchitecture);
}

inline mojom::AudioContextPtr ToMojo(const AudioContext& v) {
  return mojom::AudioContext::New(v.type, v.noise);
}

inline mojom::MediaEquipmentInfoPtr ToMojo(const MediaEquipmentInfo& v) {
  return mojom::MediaEquipmentInfo::New(v.type, v.label, v.deviceId, v.groupId);
}

inline mojom::MediaEquipmentPtr ToMojo(const MediaEquipment& v) {
  std::vector<mojom::MediaEquipmentInfoPtr> infos;
  for (const auto& i : v.list) {
    infos.push_back(ToMojo(i));
  }
  return mojom::MediaEquipment::New(v.type, std::move(infos));
}

inline mojom::ClientRectsPtr ToMojo(const ClientRects& v) {
  return mojom::ClientRects::New(v.type, v.x, v.y, v.width, v.height);
}

inline mojom::SpeechVoicesInfoPtr ToMojo(const SpeechVoicesInfo& v) {
  return mojom::SpeechVoicesInfo::New(v.voiceUri, v.name, v.lang, v.isLocalService, v.isDefault);
}

inline mojom::SpeechVoicesPtr ToMojo(const SpeechVoices& v) {
  std::vector<mojom::SpeechVoicesInfoPtr> infos;
  for (const auto& i : v.list) {
    infos.push_back(ToMojo(i));
  }
  return mojom::SpeechVoices::New(v.type, std::move(infos));
}

inline mojom::ResourceInfoPtr ToMojo(const ResourceInfo& v) {
  return mojom::ResourceInfo::New(v.type, v.cpu, v.memory);
}

inline mojom::DoNotTrackPtr ToMojo(const DoNotTrack& v) {
  return mojom::DoNotTrack::New(v.type, v.flag);
}

inline mojom::OpenPortPtr ToMojo(const OpenPort& v) {
  std::vector<int32_t> ports;
  for (int p : v.openPort) {
    ports.push_back(static_cast<int32_t>(p));
  }
  return mojom::OpenPort::New(v.type, std::move(ports), v.url);
}

inline mojom::DeviceNamePtr ToMojo(const DeviceName& v) {
  return mojom::DeviceName::New(v.type, v.value);
}

inline mojom::MacAddressPtr ToMojo(const MacAddress& v) {
  return mojom::MacAddress::New(v.type);
}

inline mojom::SSLFingerprintPtr ToMojo(const SSLFingerprint& v) {
  return mojom::SSLFingerprint::New(v.type, v.profile);
}

inline mojom::HardwareAccelerationPtr ToMojo(const HardwareAcceleration& v) {
  return mojom::HardwareAcceleration::New(v.type);
}

inline mojom::FingerprintPtr FingerprintToMojo(const Fingerprint& fp) {
  return mojom::Fingerprint::New(
      fp.init,
      ToMojo(fp.consistency),
      ToMojo(fp.ua),
      ToMojo(fp.uaMetadata),
      ToMojo(fp.timeZone),
      ToMojo(fp.webRTC),
      ToMojo(fp.location),
      ToMojo(fp.language),
      ToMojo(fp.resolution),
      ToMojo(fp.font),
      ToMojo(fp.canvas),
      ToMojo(fp.webGL),
      ToMojo(fp.gupGL),
      ToMojo(fp.webGLDevice),
      ToMojo(fp.audioContext),
      ToMojo(fp.mediaEquipment),
      ToMojo(fp.clientRects),
      ToMojo(fp.speechVoices),
      ToMojo(fp.resourceInfo),
      ToMojo(fp.doNotTrack),
      ToMojo(fp.openPort),
      ToMojo(fp.deviceName),
      ToMojo(fp.macAddress),
      ToMojo(fp.sslFingerprint),
      ToMojo(fp.hardwareAcceleration));
}

// Convert from Mojo FingerprintPtr back to C++ Fingerprint struct
inline Fingerprint FingerprintFromMojo(const mojom::FingerprintPtr& mojo_fp) {
  Fingerprint fp;
  fp.init = mojo_fp->init;

  if (mojo_fp->consistency) {
    fp.consistency.type = mojo_fp->consistency->type;
    fp.consistency.urlList = mojo_fp->consistency->urlList;
  }
  if (mojo_fp->ua) {
    fp.ua.type = mojo_fp->ua->type;
    fp.ua.userAgent = mojo_fp->ua->userAgent;
  }
  if (mojo_fp->uaMetadata) {
    fp.uaMetadata.type = mojo_fp->uaMetadata->type;
    fp.uaMetadata.userAgentMetadata = mojo_fp->uaMetadata->userAgentMetadata;
  }
  if (mojo_fp->timeZone) {
    fp.timeZone.type = mojo_fp->timeZone->type;
    fp.timeZone.gmt = mojo_fp->timeZone->gmt;
  }
  if (mojo_fp->webRTC) {
    fp.webRTC.type = mojo_fp->webRTC->type;
    fp.webRTC.privateIp = mojo_fp->webRTC->privateIp;
    fp.webRTC.publicIp = mojo_fp->webRTC->publicIp;
  }
  if (mojo_fp->location) {
    fp.location.type = mojo_fp->location->type;
    fp.location.permissions = mojo_fp->location->permissions;
    fp.location.latitude = mojo_fp->location->latitude;
    fp.location.longitude = mojo_fp->location->longitude;
    fp.location.accuracy = mojo_fp->location->accuracy;
  }
  if (mojo_fp->language) {
    fp.language.type = mojo_fp->language->type;
    fp.language.interfaceLanguage = mojo_fp->language->interfaceLanguage;
    fp.language.languages = mojo_fp->language->languages;
  }
  if (mojo_fp->resolution) {
    fp.resolution.type = mojo_fp->resolution->type;
    fp.resolution.windowWidth = mojo_fp->resolution->windowWidth;
    fp.resolution.windowHeight = mojo_fp->resolution->windowHeight;
    fp.resolution.monitorWidth = mojo_fp->resolution->monitorWidth;
    fp.resolution.monitorHeight = mojo_fp->resolution->monitorHeight;
  }
  if (mojo_fp->font) {
    fp.font.type = mojo_fp->font->type;
    fp.font.maxId = mojo_fp->font->maxId;
    fp.font.defaultId = mojo_fp->font->defaultId;
    fp.font.defaultName = mojo_fp->font->defaultName;
    fp.font.defaultPaths = mojo_fp->font->defaultPaths;
    for (auto& [k, v] : mojo_fp->font->fontMap) {
      if (v) {
        FontInfo fi;
        fi.id = v->id;
        fi.width = v->width;
        fi.height = v->height;
        fi.actualBoundingBoxAscent = v->actualBoundingBoxAscent;
        fi.actualBoundingBoxDescent = v->actualBoundingBoxDescent;
        fi.actualBoundingBoxLeft = v->actualBoundingBoxLeft;
        fi.actualBoundingBoxRight = v->actualBoundingBoxRight;
        fi.fontBoundingBoxAscent = v->fontBoundingBoxAscent;
        fi.fontBoundingBoxDescent = v->fontBoundingBoxDescent;
        fi.filePaths = v->filePaths;
        fp.font.fontMap.insert(std::make_pair(k, fi));
      }
    }
    for (auto& [k, v] : mojo_fp->font->fontIdMap) {
      fp.font.fontIdMap.insert(std::make_pair(static_cast<int>(k), v));
    }
  }
  if (mojo_fp->canvas) {
    fp.canvas.type = mojo_fp->canvas->type;
    for (const auto& p : mojo_fp->canvas->coloredPointList) {
      if (p) {
        ColoredPoint cp;
        cp.row = p->row;
        cp.column = p->column;
        cp.red = p->red;
        cp.green = p->green;
        cp.blue = p->blue;
        cp.alpha = p->alpha;
        fp.canvas.coloredPointList.push_back(cp);
      }
    }
  }
  if (mojo_fp->webGL) {
    fp.webGL.type = mojo_fp->webGL->type;
    for (const auto& p : mojo_fp->webGL->coloredPointList) {
      if (p) {
        ColoredPoint cp;
        cp.row = p->row;
        cp.column = p->column;
        cp.red = p->red;
        cp.green = p->green;
        cp.blue = p->blue;
        cp.alpha = p->alpha;
        fp.webGL.coloredPointList.push_back(cp);
      }
    }
  }
  if (mojo_fp->gupGL) {
    fp.gupGL.type = mojo_fp->gupGL->type;
    for (const auto& p : mojo_fp->gupGL->coloredPointList) {
      if (p) {
        ColoredPoint cp;
        cp.row = p->row;
        cp.column = p->column;
        cp.red = p->red;
        cp.green = p->green;
        cp.blue = p->blue;
        cp.alpha = p->alpha;
        fp.gupGL.coloredPointList.push_back(cp);
      }
    }
  }
  if (mojo_fp->webGLDevice) {
    fp.webGLDevice.type = mojo_fp->webGLDevice->type;
    fp.webGLDevice.vendors = mojo_fp->webGLDevice->vendors;
    fp.webGLDevice.renderer = mojo_fp->webGLDevice->renderer;
    fp.webGLDevice.gpuVendors = mojo_fp->webGLDevice->gpuVendors;
    fp.webGLDevice.gpuArchitecture = mojo_fp->webGLDevice->gpuArchitecture;
  }
  if (mojo_fp->audioContext) {
    fp.audioContext.type = mojo_fp->audioContext->type;
    fp.audioContext.noise = mojo_fp->audioContext->noise;
  }
  if (mojo_fp->mediaEquipment) {
    fp.mediaEquipment.type = mojo_fp->mediaEquipment->type;
    for (const auto& i : mojo_fp->mediaEquipment->list) {
      if (i) {
        MediaEquipmentInfo mei;
        mei.type = i->type;
        mei.label = i->label;
        mei.deviceId = i->deviceId;
        mei.groupId = i->groupId;
        fp.mediaEquipment.list.push_back(mei);
      }
    }
  }
  if (mojo_fp->clientRects) {
    fp.clientRects.type = mojo_fp->clientRects->type;
    fp.clientRects.x = mojo_fp->clientRects->x;
    fp.clientRects.y = mojo_fp->clientRects->y;
    fp.clientRects.width = mojo_fp->clientRects->width;
    fp.clientRects.height = mojo_fp->clientRects->height;
  }
  if (mojo_fp->speechVoices) {
    fp.speechVoices.type = mojo_fp->speechVoices->type;
    for (const auto& i : mojo_fp->speechVoices->list) {
      if (i) {
        SpeechVoicesInfo svi;
        svi.voiceUri = i->voiceUri;
        svi.name = i->name;
        svi.lang = i->lang;
        svi.isLocalService = i->isLocalService;
        svi.isDefault = i->isDefault;
        fp.speechVoices.list.push_back(svi);
      }
    }
  }
  if (mojo_fp->resourceInfo) {
    fp.resourceInfo.type = mojo_fp->resourceInfo->type;
    fp.resourceInfo.cpu = mojo_fp->resourceInfo->cpu;
    fp.resourceInfo.memory = mojo_fp->resourceInfo->memory;
  }
  if (mojo_fp->doNotTrack) {
    fp.doNotTrack.type = mojo_fp->doNotTrack->type;
    fp.doNotTrack.flag = mojo_fp->doNotTrack->flag;
  }
  if (mojo_fp->openPort) {
    fp.openPort.type = mojo_fp->openPort->type;
    for (int32_t p : mojo_fp->openPort->openPort) {
      fp.openPort.openPort.push_back(static_cast<int>(p));
    }
    fp.openPort.url = mojo_fp->openPort->url;
  }
  if (mojo_fp->deviceName) {
    fp.deviceName.type = mojo_fp->deviceName->type;
    fp.deviceName.value = mojo_fp->deviceName->value;
  }
  if (mojo_fp->macAddress) {
    fp.macAddress.type = mojo_fp->macAddress->type;
  }
  if (mojo_fp->sslFingerprint) {
    fp.sslFingerprint.type = mojo_fp->sslFingerprint->type;
    fp.sslFingerprint.profile = mojo_fp->sslFingerprint->profile;
  }
  if (mojo_fp->hardwareAcceleration) {
    fp.hardwareAcceleration.type = mojo_fp->hardwareAcceleration->type;
  }

  return fp;
}

}  // namespace fp
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_PUBLIC_COMMON_FINGERPRINT_FINGERPRINT_MOJO_CONVERT_H_
