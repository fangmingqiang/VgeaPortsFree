//
//  kern_igdvmt.cpp
//  IntelGraphicsDVMTFixup
//
//  Copyright © 2017 lvs1974. All rights reserved.
//
//  This kext is made based on lvs1974's NvidiaGraphicsFixup.
//

#include <Headers/kern_api.hpp>
#include <Headers/kern_util.hpp>
#include <Library/LegacyIOService.h>

#include <mach/vm_map.h>
#include <IOKit/IORegistryEntry.h>

#include "kern_vega.hpp"



static const char *kextAppleGraphicsDevicePolicyId { "com.apple.driver.AppleGraphicsDevicePolicy" };
static const char *kextAGDPolicy[] {
    "/System/Library/Extensions/AppleGraphicsControl.kext/Contents/PlugIns/AppleGraphicsDevicePolicy.kext/Contents/MacOS/AppleGraphicsDevicePolicy"
};

static KernelPatcher::KextInfo kextList[] {
    { kextAppleGraphicsDevicePolicyId, kextAGDPolicy, 1, {}, {}, KernelPatcher::KextInfo::Unloaded },
};

static size_t kextListSize {arrsize(kextList)};

bool VEGA::init() {
	LiluAPI::Error error = lilu.onKextLoad(kextList, kextListSize,
	[](void *user, KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
		VEGA *Vega = static_cast<VEGA *>(user);
		Vega->processKext(patcher, index, address, size);
	}, this);
	
	if (error != LiluAPI::Error::NoError) {
		SYSLOG("Vega", "failed to register onPatcherLoad method %d", error);
		return false;
	}
	
	return true;
}

void VEGA::deinit() {
}

void VEGA::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
    if (progressState != ProcessingState::EverythingDone) {
        for (size_t i = 0; i < kextListSize; i++) {
            if (kextList[i].loadIndex == index) {
                if (!(progressState & ProcessingState::GraphicsFramebufferPatched) && !strcmp(kextList[i].id, kextAppleGraphicsDevicePolicyId)) {
                    DBGLOG("Vega", "found %s", kextAppleGraphicsDevicePolicyId);
                    if (getKernelVersion() == KernelVersion::HighSierra){
                        const uint8_t find[]    = { 0x64, 0x2D, 0x69, 0x64 };
                        const uint8_t replace[] = { 0x64, 0x2D, 0x69, 0x78 };
                        KextPatch kext_patch {
                            {&kextList[i], find, replace, sizeof(find), 0},
                            KernelVersion::HighSierra, KernelVersion::HighSierra
                        };
                        applyPatches(patcher, index, &kext_patch, 1);
                        progressState |= ProcessingState::GraphicsFramebufferPatched;
                        DBGLOG("Vega", "vega graphice");
                    }
                }
            }
        }
    }
	
    // Ignore all the errors for other processors
    patcher.clearError();
}

void VEGA::applyPatches(KernelPatcher &patcher, size_t index, const KextPatch *patches, size_t patchNum) {
    DBGLOG("Vega", "applying patches for %zu kext", index);
    for (size_t p = 0; p < patchNum; p++) {
        auto &patch = patches[p];
        if (patch.patch.kext->loadIndex == index) {
            if (patcher.compatibleKernel(patch.minKernel, patch.maxKernel)) {
                DBGLOG("Vega", "applying %zu patch for %zu kext", p, index);
                patcher.applyLookupPatch(&patch.patch);
                // Do not really care for the errors for now
                patcher.clearError();
            }
        }
    }
}

