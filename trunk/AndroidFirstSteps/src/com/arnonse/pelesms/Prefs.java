package com.arnonse.pelesms;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.PreferenceActivity;


public class Prefs extends PreferenceActivity implements OnSharedPreferenceChangeListener {
	
	@Override 
	protected void onCreate(Bundle savedInstanceState) { 
		super.onCreate(savedInstanceState); 
		addPreferencesFromResource(R.xml.prefs);
	}


	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
			String key) {
	}

	

}

