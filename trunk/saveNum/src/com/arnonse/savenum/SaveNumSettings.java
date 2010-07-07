package com.arnonse.savenum;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;

public class SaveNumSettings extends PreferenceActivity{
	
	Preference donationPref;

	@Override 
	protected void onCreate(Bundle savedInstanceState) { 
		super.onCreate(savedInstanceState); 
		addPreferencesFromResource(R.xml.settings);
		donationPref = (Preference) findPreference("donation");
		
		String url = "http://www.morethantechnical.com/savenum/";
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri.parse(url));
		
		donationPref.setIntent(intent);
	}
	
	
}

