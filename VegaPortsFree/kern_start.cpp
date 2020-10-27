//
//  kern_start.cpp
//  IntelGraphicsDVMTFixup
//
//  Copyright © 2017 lvs1974. All rights reserved.
//
//  This kext is made based on lvs1974's NvidiaGraphicsFixup.
//

#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>

#include "kern_vega.hpp"
static VEGA Vega;

static const char *bootargOff[] {
	"-vegaoff"
};

static const char *bootargDebug[] {
	"-vegadbg"
};

static const char *bootargBeta[] {
	"-vegabeta"
};

PluginConfiguration ADDPR(config) {
	xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal | LiluAPI::AllowInstallerRecovery,
	bootargOff,
	arrsize(bootargOff),
	bootargDebug,
	arrsize(bootargDebug),
	bootargBeta,
	arrsize(bootargBeta),
	KernelVersion::HighSierra,
	KernelVersion::HighSierra,
	[]() {
		Vega.init();
	}
};
