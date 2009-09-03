package com.arnonse.savenum;
import android.os.Bundle;
import android.preference.PreferenceActivity;


public class SaveNumSettings extends PreferenceActivity{

	@Override 
	protected void onCreate(Bundle savedInstanceState) { 
		super.onCreate(savedInstanceState); 
		addPreferencesFromResource(R.xml.settings);
	}
	
	
}

