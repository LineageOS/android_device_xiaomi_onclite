/*
 * SPDX-FileCopyrightText: 2015 The CyanogenMod Project
 * SPDX-FileCopyrightText: 2017 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.settings.doze;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootCompletedReceiver extends BroadcastReceiver {

    private static final boolean DEBUG = false;
    private static final String TAG = "XiaomiDoze";

    @Override
    public void onReceive(final Context context, Intent intent) {
        if (DozeUtils.isDozeEnabled(context) && DozeUtils.sensorsEnabled(context)) {
            if (DEBUG) Log.d(TAG, "Starting service");
            DozeUtils.checkDozeService(context);
        }
    }

}
