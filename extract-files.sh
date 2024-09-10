#!/bin/bash
#
# SPDX-FileCopyrightText: 2016 The CyanogenMod Project
# SPDX-FileCopyrightText: 2017-2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Required!
export DEVICE=onclite
export VENDOR=xiaomi

export DEVICE_BRINGUP_YEAR=2019

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

SECTION=
KANG=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"
                shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
    vendor/lib/hw/camera.msm8953.so)
        [ "$2" = "" ] && return 0
        grep -q "libui_shim.so" "${2}" || "${PATCHELF}" --add-needed "libui_shim.so" "${2}"
        ;;
    vendor/lib/lib_lowlight.so)
        [ "$2" = "" ] && return 0
        "${PATCHELF}" --replace-needed "libstdc++.so" "libstdc++_vendor.so" "${2}"
        ;;
    vendor/lib64/hw/fingerprint.goodix.default.so | vendor/lib64/libvendor.goodix.hardware.fingerprint@1.0-service.so)
        [ "$2" = "" ] && return 0
        "${PATCHELF_0_17_2}" --replace-needed "libvendor.goodix.hardware.fingerprint@1.0.so" "vendor.goodix.hardware.fingerprint@1.0.so" "${2}"
        ;;
    vendor/lib*/libwvhidl.so)
        [ "$2" = "" ] && return 0
        grep -q libcrypto_shim.so "${2}" || "${PATCHELF}" --add-needed "libcrypto_shim.so" "${2}"
        ;;
        *)
            return 1
            ;;
    esac
    return 0
}

function blob_fixup_dry() {
    blob_fixup "$1" ""
}

setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" \
        "${KANG}" --section "${SECTION}"

"${MY_DIR}/setup-makefiles.sh"
