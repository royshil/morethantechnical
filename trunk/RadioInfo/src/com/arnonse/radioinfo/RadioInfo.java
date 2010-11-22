package com.arnonse.radioinfo;

import java.io.IOException;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;

public class RadioInfo extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent myInt = new Intent(Intent.ACTION_MAIN)
	        .addCategory(Intent.CATEGORY_LAUNCHER)
	        .setComponent(new ComponentName("com.android.settings", "com.android.settings.RadioInfo")); 
        startActivity(myInt);
        
        
        try {
        	SystemClock.sleep(100);
			//Process process = 
        	Runtime.getRuntime().exec("su");  
        	Runtime.getRuntime().exec("input keyevent 82");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Log.e("Error!",e.getMessage());
		}//

        
        
         
        
        // keyevent 82
        // keyevent 20 20 20 
       //  keyevent 21 21 21
        // keyevent 23
        
        finish();
    }
}