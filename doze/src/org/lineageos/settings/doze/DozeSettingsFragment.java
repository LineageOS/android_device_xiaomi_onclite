/*
 * SPDX-FileCopyrightText: 2015 The CyanogenMod Project
 * SPDX-FileCopyrightText: 2017-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.settings.doze;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.widget.CompoundButton;

import androidx.preference.PreferenceFragment;
import androidx.preference.SwitchPreferenceCompat;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;

import com.android.settingslib.widget.MainSwitchPreference;

public class DozeSettingsFragment extends PreferenceFragment implements
        Preference.OnPreferenceChangeListener, CompoundButton.OnCheckedChangeListener {

    private MainSwitchPreference mSwitchBar;

    private SwitchPreferenceCompat mWakeOnGesturePreference;
    private SwitchPreferenceCompat mPickUpPreference;
    private SwitchPreferenceCompat mHandwavePreference;
    private SwitchPreferenceCompat mPocketPreference;

    private Handler mHandler = new Handler();

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.doze_settings);

        SharedPreferences prefs = getActivity().getSharedPreferences("doze_settings",
                Activity.MODE_PRIVATE);
        if (savedInstanceState == null && !prefs.getBoolean("first_help_shown", false)) {
            showHelp();
        }

        boolean dozeEnabled = DozeUtils.isDozeEnabled(getActivity());

        mSwitchBar = (MainSwitchPreference) findPreference(DozeUtils.DOZE_ENABLE);
        mSwitchBar.addOnSwitchChangeListener(this);
        mSwitchBar.setChecked(dozeEnabled);

        mWakeOnGesturePreference = (SwitchPreferenceCompat) findPreference(DozeUtils.WAKE_ON_GESTURE_KEY);
        mWakeOnGesturePreference.setEnabled(dozeEnabled);
        mWakeOnGesturePreference.setOnPreferenceChangeListener(this);

        PreferenceCategory proximitySensorCategory =
                (PreferenceCategory) getPreferenceScreen().findPreference(DozeUtils.CATEG_PROX_SENSOR);

        mPickUpPreference = (SwitchPreferenceCompat) findPreference(DozeUtils.GESTURE_PICK_UP_KEY);
        mPickUpPreference.setEnabled(dozeEnabled);
        mPickUpPreference.setOnPreferenceChangeListener(this);

        mHandwavePreference = (SwitchPreferenceCompat) findPreference(DozeUtils.GESTURE_HAND_WAVE_KEY);
        mHandwavePreference.setEnabled(dozeEnabled);
        mHandwavePreference.setOnPreferenceChangeListener(this);

        mPocketPreference = (SwitchPreferenceCompat) findPreference(DozeUtils.GESTURE_POCKET_KEY);
        mPocketPreference.setEnabled(dozeEnabled);
        mPocketPreference.setOnPreferenceChangeListener(this);

        // Hide proximity sensor related features if the device doesn't support them
        if (!DozeUtils.getProxCheckBeforePulse(getActivity())) {
            getPreferenceScreen().removePreference(proximitySensorCategory);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        mHandler.post(() -> DozeUtils.checkDozeService(getActivity()));

        return true;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        DozeUtils.enableDoze(getActivity(), isChecked);
        DozeUtils.checkDozeService(getActivity());

        mSwitchBar.setChecked(isChecked);

        mWakeOnGesturePreference.setEnabled(isChecked);
        mPickUpPreference.setEnabled(isChecked);
        mHandwavePreference.setEnabled(isChecked);
        mPocketPreference.setEnabled(isChecked);
    }

    private void showHelp() {
        AlertDialog fragment = new AlertDialog.Builder(getActivity())
                .setTitle(R.string.doze_settings_help_title)
                .setMessage(R.string.doze_settings_help_text)
                .setPositiveButton(R.string.dialog_ok,
                        (dialog, which) -> {
                            getActivity()
                                    .getSharedPreferences("doze_settings", Activity.MODE_PRIVATE)
                                    .edit()
                                    .putBoolean("first_help_shown", true)
                                    .commit();
                            dialog.cancel();
                        })
                .create();
        fragment.show();
    }
}
