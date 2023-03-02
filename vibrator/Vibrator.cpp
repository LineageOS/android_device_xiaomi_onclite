/*
 * Copyright (C) 2019 The Android Open Source Project
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

#define LOG_TAG "VibratorService"

#include <android-base/file.h>
#include <log/log.h>

#include "Vibrator.h"

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

static const std::string kLedVibDeviceDir = "/sys/class/leds/vibrator/";
static const std::string kLedVibDeviceActivateFile = kLedVibDeviceDir + "activate";
static const std::string kLedVibDeviceDurationFile = kLedVibDeviceDir + "duration";
static const std::string kLedVibDeviceStateFile = kLedVibDeviceDir + "state";
static const std::string kLedVibDeviceVmaxMvFile = kLedVibDeviceDir + "vmax_mv";

static constexpr uint32_t QPNP_VIB_LDO_VMIN_MV = 1504;
static constexpr uint32_t QPNP_VIB_LDO_VMAX_MV = 3544;
static constexpr uint32_t MV_ADDITION_MAX = QPNP_VIB_LDO_VMAX_MV - QPNP_VIB_LDO_VMIN_MV;

Vibrator::Vibrator() {}

// Methods from ::android::hardware::vibrator::V1_0::IVibrator follow.

Return<Status> Vibrator::on(uint32_t timeoutMs) {
    mHasEffect = false;
    return enable(true, timeoutMs);
}

Return<Status> Vibrator::off() {
    if (mHasEffect)
        return Status::OK;
    else
        return enable(false, 0);
}

Return<bool> Vibrator::supportsAmplitudeControl() {
    return true;
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    if (amplitude < 0) {
        return Status::BAD_VALUE;
    }

    uint32_t mv_addition = amplitude * MV_ADDITION_MAX / 0xFF;
    uint32_t mv = QPNP_VIB_LDO_VMIN_MV + mv_addition;
    if (!android::base::WriteStringToFile(std::to_string(mv), kLedVibDeviceVmaxMvFile)) {
        ALOGE("Failed to set amplitude!");
        return Status::UNKNOWN_ERROR;
    }

    ALOGI("Amplitude: %u -> %u, mv = %u, mv_addition = %u\n",
            mAmplitude, amplitude, mv, mv_addition);
    mAmplitude = amplitude;
    return Status::OK;
}

Return<void> Vibrator::perform(V1_0::Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    return perform<decltype(effect)>(effect, strength, _hidl_cb);
}

// Methods from ::android::hardware::vibrator::V1_1::IVibrator follow.

Return<void> Vibrator::perform_1_1(V1_1::Effect_1_1 effect, EffectStrength strength,
                                   perform_cb _hidl_cb) {
    return perform<decltype(effect)>(effect, strength, _hidl_cb);
}

// Methods from ::android::hardware::vibrator::V1_2::IVibrator follow.

Return<void> Vibrator::perform_1_2(V1_2::Effect effect, EffectStrength strength,
                                   perform_cb _hidl_cb) {
    return perform<decltype(effect)>(effect, strength, _hidl_cb);
}

// Methods from ::android::hardware::vibrator::V1_3::IVibrator follow.

Return<bool> Vibrator::supportsExternalControl() {
    return true;
}

Return<Status> Vibrator::setExternalControl(bool enabled) {
    ALOGI("ExternalControl: %s -> %s\n", mExternalControl ? "true" : "false",
            enabled ? "true" : "false");
    mExternalControl = enabled;
    return Status::OK;
}

Return<void> Vibrator::perform_1_3(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    return perform<decltype(effect)>(effect, strength, _hidl_cb);
}

// Private methods follow.

Return<void> Vibrator::perform(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    uint8_t amplitude;
    uint32_t ms;
    Status status = Status::OK;

    ALOGI("Perform: Effect %s\n", effectToName(effect).c_str());
    mHasEffect = true;

    amplitude = strengthToAmplitude(strength, &status);
    if (status != Status::OK) {
        _hidl_cb(status, 0);
        return Void();
    }
    setAmplitude(amplitude);

    ms = effectToMs(effect, &status);
    ALOGI("ms = %u", ms);
    if (status != Status::OK) {
        _hidl_cb(status, 0);
        return Void();
    }
    status = enable(true, ms);

    _hidl_cb(status, ms);

    return Void();
}

template <typename T>
Return<void> Vibrator::perform(T effect, EffectStrength strength, perform_cb _hidl_cb) {
    auto validRange = hidl_enum_range<T>();
    if (effect < *validRange.begin() || effect > *std::prev(validRange.end())) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }
    return perform(static_cast<Effect>(effect), strength, _hidl_cb);
}

Status Vibrator::enable(bool enabled, uint32_t ms) {
    if (mExternalControl) {
        ALOGW("Enabling/disabling while the vibrator is externally controlled is unsupported!");
        return Status::UNSUPPORTED_OPERATION;
    } else {
        if (!android::base::WriteStringToFile(enabled ? "1" : "0", kLedVibDeviceStateFile) ||
            !android::base::WriteStringToFile(std::to_string(ms), kLedVibDeviceDurationFile) ||
            !android::base::WriteStringToFile(enabled ? "1" : "0", kLedVibDeviceActivateFile)) {
            ALOGE("Failed to enable vibration!");
            return Status::UNKNOWN_ERROR;
        }
        return Status::OK;
    }
}

const std::string Vibrator::effectToName(Effect effect) {
    return toString(effect);
}

uint32_t Vibrator::effectToMs(Effect effect, Status* status) {
    switch (effect) {
        case Effect::CLICK:
            return 10;
        case Effect::DOUBLE_CLICK:
            return 15;
        case Effect::TICK:
        case Effect::TEXTURE_TICK:
            return 5;
        case Effect::THUD:
            return 5;
        case Effect::POP:
            return 5;
        case Effect::HEAVY_CLICK:
            return 10;
        case Effect::RINGTONE_1:
            return 30000;
        case Effect::RINGTONE_2:
            return 30000;
        case Effect::RINGTONE_3:
            return 30000;
        case Effect::RINGTONE_4:
            return 30000;
        case Effect::RINGTONE_5:
            return 30000;
        case Effect::RINGTONE_6:
            return 30000;
        case Effect::RINGTONE_7:
            return 30000;
        case Effect::RINGTONE_8:
            return 30000;
        case Effect::RINGTONE_9:
            return 30000;
        case Effect::RINGTONE_10:
            return 30000;
        case Effect::RINGTONE_11:
            return 30000;
        case Effect::RINGTONE_12:
            return 30000;
        case Effect::RINGTONE_13:
            return 30000;
        case Effect::RINGTONE_14:
            return 30000;
        case Effect::RINGTONE_15:
            return 30000;
    }
    *status = Status::UNSUPPORTED_OPERATION;
    return 0;
}

uint8_t Vibrator::strengthToAmplitude(EffectStrength strength, Status* status) {
    switch (strength) {
        case EffectStrength::LIGHT:
            return 63;
        case EffectStrength::MEDIUM:
            return 159;
        case EffectStrength::STRONG:
            return 255;
    }
    *status = Status::UNSUPPORTED_OPERATION;
    return 0;
}

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
