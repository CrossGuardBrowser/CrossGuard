#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_WEBGPU_GPU_ADAPTER_FP_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_WEBGPU_GPU_ADAPTER_FP_H_

#include "third_party/blink/public/common/fingerprint/singleton_fingerprint.h"
#include "third_party/blink/public/common/fingerprint/fingerprint.h"

#include "third_party/blink/renderer/modules/webgpu/gpu_adapter_info.h"
#include "third_party/blink/renderer/modules/webgpu/dawn_enum_conversions.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

blink::GPUAdapterInfo* fpRequestAdapterInfo(blink::GPUAdapterInfo* adapter_info, bool flag) {
    if (!base::SingletonFingerprint::HasInstance()) return adapter_info;
    base::SingletonFingerprint* t_singletonFingerprint = base::SingletonFingerprint::ForCurrentProcess();
    if (!t_singletonFingerprint) return adapter_info;
    blink::fp::Fingerprint t_fingerprint = t_singletonFingerprint->GetFingerprint();
    if (base::SingletonFingerprint::GetInit(t_singletonFingerprint) && t_fingerprint.webGLDevice.type > 1) {
        blink::GPUAdapterInfo* T_adapter_info;

        if (flag) {
            // Chromium 138: Full constructor with all adapter info
            // Note: adapter_info->backend() and type() are already Strings
            T_adapter_info = blink::MakeGarbageCollected<blink::GPUAdapterInfo>(
                String::FromUTF8(t_fingerprint.webGLDevice.gpuVendors),
                String::FromUTF8(t_fingerprint.webGLDevice.gpuArchitecture),
                adapter_info->subgroupMinSize(),
                adapter_info->subgroupMaxSize(),
                adapter_info->isFallbackAdapter(),
                adapter_info->device(),
                adapter_info->description(),
                adapter_info->driver(),
                adapter_info->backend(),
                adapter_info->type(),
                adapter_info->d3dShaderModel(),
                adapter_info->vkDriverVersion(),
                adapter_info->powerPreference());
        } else {
            // Chromium 138: Basic constructor with essential info
            T_adapter_info = blink::MakeGarbageCollected<blink::GPUAdapterInfo>(
                String::FromUTF8(t_fingerprint.webGLDevice.gpuVendors),
                String::FromUTF8(t_fingerprint.webGLDevice.gpuArchitecture),
                adapter_info->subgroupMinSize(),
                adapter_info->subgroupMaxSize(),
                adapter_info->isFallbackAdapter());
        }
        return T_adapter_info;
    }

    return adapter_info;
}

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_WEBGPU_GPU_ADAPTER_FP_H_
