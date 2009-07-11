package com.arnonse.silentmode;

//import android.R;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

public class SuperSilent extends Activity {
	protected static final int START_TIME_PICKER_DIALOG = 0;
	private OnCheckedChangeListener m_BasicCheckListener;
	AudioManager manager = null;
	
    /** Called when the activity is first created. */
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cblayout);

        CheckBox cb = ((CheckBox)findViewById(R.id.widget58));

        manager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
        
        m_BasicCheckListener = new OnCheckedChangeListener(){
			public void onCheckedChanged(CompoundButton buttonView,
					boolean isChecked) {
				setSilent(isChecked);
				// A comment
			}
		};
		
		cb.setOnCheckedChangeListener(m_BasicCheckListener);
        
		Button myButton = (Button) findViewById(R.id.StartTimeBtn);
		myButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				showDialog(START_TIME_PICKER_DIALOG);
			}
		});
    }
	
	@Override
	protected Dialog onCreateDialog(int id) {
		final Dialog d;
		switch (id) {
		case START_TIME_PICKER_DIALOG:
			AlertDialog.Builder builder;

			Context mContext = getApplicationContext();
			LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);
			View layout = inflater.inflate(R.layout.timerdialog,null);

			builder = new AlertDialog.Builder(this);
			builder.setView(layout);
			d = builder.create();
		
			Button doneBtn = (Button)layout.findViewById(R.id.SetTimerDoneBtn);
			doneBtn.setOnClickListener(new OnClickListener(){
				@Override
				public void onClick(View v) {
					SuperSilent.this.dismissDialog(START_TIME_PICKER_DIALOG);
				}
			});
			
			break;
		default:
			d = null;
		}
		return d;
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