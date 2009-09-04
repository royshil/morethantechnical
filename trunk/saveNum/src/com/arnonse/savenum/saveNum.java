package com.arnonse.savenum;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.text.ClipboardManager;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
//import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class saveNum extends Activity {

	EditText phnNum;
	Button btnAdd;
	Button btnDial;
	private boolean monitorChange = true;
	private static boolean iHaveTurnedOnSpeakerphone = false;
	SharedPreferences prefs;
	AudioManager audioman;
	TelephonyManager telephoneman;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Initialize preferences manager
		prefs = PreferenceManager.getDefaultSharedPreferences(this);
		audioman = (AudioManager) getSystemService(AUDIO_SERVICE);
		telephoneman = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
		
		setContentView(R.layout.main);
		setTheme(android.R.style.Theme_Dialog);
		ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		phnNum = (EditText) findViewById(R.id.phnNum);
		phnNum.setText(clipboard.getText());

		btnAdd = (Button) findViewById(R.id.btnAdd);
		btnDial = (Button) findViewById(R.id.btnDial);

		if (clipboard.getText().toString().equals("")) {
			btnAdd.setText(getString(R.string.btnSetText));
		}
		adjustInputType();

		phnNum.addTextChangedListener(new TextWatcher() {

			public void afterTextChanged(Editable arg0) {
				// TODO Auto-generated method stub

			}

			public void beforeTextChanged(CharSequence arg0, int arg1,
					int arg2, int arg3) {
				// TODO Auto-generated method stub

			}

			public void onTextChanged(CharSequence arg0, int arg1, int arg2,
					int arg3) {
				if (monitorChange) {
					monitorChange = false;
					btnAdd.setText(getString(R.string.btnSetText));
				}

			}

		});

		btnAdd.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (btnAdd.getText().equals(getString(R.string.btnClear))) {
					ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
					clipboard.setText("");
					phnNum.setText("");
					btnAdd.setText(getString(R.string.btnSetText));
				} else if (btnAdd.getText().equals(
						getString(R.string.btnSetText))) {
					ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
					clipboard.setText(phnNum.getText());
					showToast(getString(R.string.copied));
					
					InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);   
					imm.hideSoftInputFromWindow(phnNum.getWindowToken(), 0);  

					finish();
				}
			}

		});

		btnDial.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				performDial();
			}

		});
	}

	public void performDial() {
		if (!phnNum.getText().toString().equals(""))
			startActivity(new Intent(Intent.ACTION_CALL, Uri.parse("tel:" + phnNum.getText())));
		else showToast("Nothing to dial");
	}

	public void showToast(String s) {
		Toast.makeText(getBaseContext(), s, Toast.LENGTH_SHORT).show();
	}
	

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		startActivity(new Intent(this, SaveNumSettings.class));
		return super.onPrepareOptionsMenu(menu);
	}


	@Override
	protected void onResume() {
		super.onResume();
		adjustInputType();
	}
	
	private void adjustInputType()
	{
		boolean allowFullString = prefs.getBoolean("allowString", false);
		if (allowFullString)
		{
			phnNum.setInputType(InputType.TYPE_CLASS_TEXT);
		}
		else
		{
			phnNum.setInputType(InputType.TYPE_CLASS_PHONE);
		}
	}

	@Override
	protected void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		if (ShouldIStartSpeakerphone())
		{
			audioman.setSpeakerphoneOn(true);
			iHaveTurnedOnSpeakerphone=true;
		}
	}

	private boolean ShouldIStartSpeakerphone() {
		return 
		( 		prefs.getBoolean("speaker", false) &&
				!audioman.isSpeakerphoneOn() && 
				(telephoneman.getCallState() == TelephonyManager.CALL_STATE_OFFHOOK)
		);
	}

	@Override
	protected void onStop() {
		super.onStop();
		if (iHaveTurnedOnSpeakerphone)
		{
			audioman.setSpeakerphoneOn(false);
			iHaveTurnedOnSpeakerphone=false;
		}
	}

	
}
