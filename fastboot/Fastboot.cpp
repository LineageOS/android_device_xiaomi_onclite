/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/file.h>
#include <android-base/properties.h>
#include <unordered_map>

#include "Fastboot.h"

using aidl::android::hardware::fastboot::FileSystemType;
using android::base::GetProperty;
using android::base::ReadFileToString;
using ndk::ScopedAStatus;

const std::unordered_map<std::string, FileSystemType> kPartitionTypeMap = {
        // Data partitions
        {"cache", FileSystemType::F2FS},
        {"userdata", FileSystemType::F2FS},

        // System partitions
        {"product", FileSystemType::EXT4},
        {"system", FileSystemType::EXT4},
        {"system_ext", FileSystemType::EXT4},
        {"odm", FileSystemType::EXT4},
        {"vendor", FileSystemType::EXT4},

        // Possible metadata partition
        {"logdump", FileSystemType::EXT4},
        {"oem", FileSystemType::EXT4},
};

const std::string kVariantPaths[] = {
        "/sys/class/huaqin/interface/hw_info/pcba_config",  // onclite
        "/sys/xiaomi-msm8953-mach/wingtech_board_id",       // Mi8953 - Wingtech ODM
};

const std::string kVariantProps[] = {
        // Mi8937 - Longcheer ODM
        "ro.boot.hwcountry",
        "ro.boot.product.region",
};

namespace aidl {
namespace android {
namespace hardware {
namespace fastboot {

ScopedAStatus Fastboot::getPartitionType(const std::string& in_partitionName,
                                         FileSystemType* _aidl_return) {
    if (in_partitionName.empty()) {
        return ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_ARGUMENT,
                                                           "Invalid partition name");
    }
    for (const auto& [part_name, part_fstype] : kPartitionTypeMap) {
        if (part_name == in_partitionName) {
            *_aidl_return = part_fstype;
            goto out;
        }
    }
    *_aidl_return = FileSystemType::RAW;
out:
    return ScopedAStatus::ok();
}

ScopedAStatus Fastboot::doOemCommand(const std::string& in_oemCmd, std::string* _aidl_return) {
    *_aidl_return = "";
    if (in_oemCmd.empty()) {
        return ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_ARGUMENT, "Invalid command");
    }
    return ScopedAStatus::fromExceptionCodeWithMessage(EX_UNSUPPORTED_OPERATION,
                                                       "Command not supported");
}

ScopedAStatus Fastboot::getVariant(std::string* _aidl_return) {
    std::string variant;
    for (const auto& variant_path : kVariantPaths) {
        if (ReadFileToString(variant_path, &variant, true)) {
            *_aidl_return = variant;
            goto out;
        }
    }
    for (const auto& variant_prop : kVariantProps) {
        variant = GetProperty(variant_prop, "");
        if (!variant.empty()) {
            *_aidl_return = variant;
            goto out;
        }
    }
    *_aidl_return = "Unknown";
out:
    return ScopedAStatus::ok();
}

ScopedAStatus Fastboot::getOffModeChargeState(bool* _aidl_return) {
    *_aidl_return = true;
    return ScopedAStatus::ok();
}

ScopedAStatus Fastboot::getBatteryVoltageFlashingThreshold(int32_t* _aidl_return) {
    *_aidl_return = 0;
    return ScopedAStatus::ok();
}

ScopedAStatus Fastboot::doOemSpecificErase() {
    return ScopedAStatus::fromExceptionCodeWithMessage(EX_UNSUPPORTED_OPERATION,
                                                       "Command not supported");
}

}  // namespace fastboot
}  // namespace hardware
}  // namespace android
}  // namespace aidl
