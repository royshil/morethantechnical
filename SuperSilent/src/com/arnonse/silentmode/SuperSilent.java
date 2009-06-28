package com.arnonse.silentmode;

//import android.R;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.media.AudioManager;

import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;


public class SuperSilent extends Activity {
	private OnCheckedChangeListener m_BasicCheckListener;
	AudioManager manager = null;
	
    /** Called when the activity is first created. */
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cblayout);

        CheckBox cb = ((CheckBox)findViewById(R.id.issilent));

        manager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
        
        m_BasicCheckListener = new OnCheckedChangeListener(){
			public void onCheckedChanged(CompoundButton buttonView,
					boolean isChecked) {
				setSilent(isChecked);
				// A comment
			}
		};
		
		cb.setOnCheckedChangeListener(m_BasicCheckListener);
        

    }
    
    @SuppressWarnings("static-access")
	public void setSilent(boolean silent)
    {
    	if (silent)
    	{
            manager.setRingerMode(manager.RINGER_MODE_SILENT);
            manager.setVibrateSetting(manager.VIBRATE_TYPE_NOTIFICATION, manager.VIBRATE_SETTING_OFF);
            manager.setVibrateSetting(manager.VIBRATE_TYPE_RINGER, manager.VIBRATE_SETTING_OFF);    		
    	}
    	else
    	{
            manager.setRingerMode(manager.RINGER_MODE_NORMAL);
            manager.setVibrateSetting(manager.VIBRATE_TYPE_NOTIFICATION, manager.VIBRATE_SETTING_ON);
            manager.setVibrateSetting(manager.VIBRATE_TYPE_RINGER, manager.VIBRATE_SETTING_ON);    		
    	}
    }

}