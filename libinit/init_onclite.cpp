/*
 * SPDX-FileCopyrightText: 2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"

// Define variants
#define VARIANT_ONE "Redmi 7"
#define VARIANT_TWO "Redmi Y3"

using android::base::GetProperty;

static const std::string ro_props_sources[] = {
    "",
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
    "vendor_dlkm."
};

void property_override(const std::string& name, const std::string& value, bool add = true) {
    auto pi = const_cast<prop_info*>(__system_property_find(name.c_str()));

    if (pi != nullptr) {
        __system_property_update(pi, value.c_str(), value.size());
    } else if (add) {
        __system_property_add(name.c_str(), name.size(), value.c_str(), value.size());
    }
}

void set_model_props(const std::string &model) {
    // Set device model
    for (const std::string &source : ro_props_sources) {
        std::string prop = "ro.product." + source + "model";
        property_override(prop, model);
    }
    // Set bluetooth name
    property_override("bluetooth.device.default_name", model);
}

void set_device_model() {
    // Get the hwversion prop
    const std::string hwversion = GetProperty("ro.boot.hwversion", "");
    // Only Redmi Y3 has hwversion 1.19.0.
    // So set the device model based on it.
    if (hwversion == "1.19.0") {
      set_model_props(VARIANT_TWO);
    } else {
      set_model_props(VARIANT_ONE);
    }
}

void vendor_load_properties() {
    set_device_model();
}
