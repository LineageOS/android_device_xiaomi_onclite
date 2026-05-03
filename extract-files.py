#!/bin/bash
#
# SPDX-FileCopyrightText: 2016 The CyanogenMod Project
# SPDX-FileCopyrightText: 2017-2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)
namespace_imports = [
	"hardware/qcom-caf/msm8953", 
	"hardware/qcom/wlan/legacy",
	"vendor/qcom/opensource/dataservices",
	"vendor/qcom/opensource/display",
	"hardware/xiaomi"
]


blob_fixups: blob_fixups_user_type = {
"vendor/lib/hw/camera.msm8953.so": blob_fixup().add_needed("libui_shim.so"),
"vendor/lib/lib_lowlight.so":blob_fixup().replace_needed("libstdc++.so","libstdc++_vendor.so"),
"vendor/lib64/hw/fingerprint.goodix.default.so":blob_fixup().replace_needed("libvendor.goodix.hardware.fingerprint@1.0.so", "vendor.goodix.hardware.fingerprint@1.0.so"),
"vendor/lib64/libvendor.goodix.hardware.fingerprint@1.0-service.so":blob_fixup().replace_needed("libvendor.goodix.hardware.fingerprint@1.0.so", "vendor.goodix.hardware.fingerprint@1.0.so")
}


module = ExtractUtilsModule(
    'onclite',
    'xiaomi',
    blob_fixups=blob_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
