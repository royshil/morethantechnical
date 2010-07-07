package com.arnonse.savenum;

import android.app.Activity;
import android.app.NotificationManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.BaseColumns;
import android.provider.CallLog;
import android.provider.Contacts.People;
import android.provider.Contacts.Intents.Insert;
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

	private EditText phnNum;
	private Button btnAdd;
	private Button btnDial;
	private Button btnAddContact;
	private Button btnSendText;
	private Button btnImportLast;
	private boolean monitorChange = true;
	private static boolean iHaveTurnedOnSpeakerphone = false;
	private SharedPreferences prefs;
	private AudioManager audioman;
	private TelephonyManager telephoneman;
	private ClipboardManager clipboard;

	private void initVars() {
		prefs = PreferenceManager.getDefaultSharedPreferences(this);
		audioman = (AudioManager) getSystemService(AUDIO_SERVICE);
		telephoneman = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
		clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		phnNum = (EditText) findViewById(R.id.phnNum);
		btnAdd = (Button) findViewById(R.id.btnAdd);
		btnDial = (Button) findViewById(R.id.btnDial);
		btnAddContact = (Button) findViewById(R.id.btnAddContact);
		btnSendText = (Button) findViewById(R.id.btnSendText);
		btnImportLast = (Button) findViewById(R.id.btnImportLast);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);
		setTheme(android.R.style.Theme_Dialog);

		initVars();

		if (!prefs.getBoolean("useClipboard", true)) {
			phnNum.setText(prefs.getString("savedNum", ""));
		} else {
			phnNum.setText(clipboard.getText());
		}

		if ((clipboard.getText().toString().equals("") && (prefs.getBoolean(
				"useClipboard", true)))
				|| ((prefs.getString("savedNum", "").equals("")) && (!prefs
						.getBoolean("useClipboard", true)))) {
			btnAdd.setText(getString(R.string.btnSetText));
		}

		adjustInputType();

		initListeners();

	}

	private void initListeners() {
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
					SetText("");
					phnNum.setText("");

					btnAdd.setText(getString(R.string.btnSetText));
				} else if (btnAdd.getText().equals(
						getString(R.string.btnSetText))) {

					SetText(phnNum.getText().toString());
					showToast(getString(R.string.copied));

					InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
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

		btnAddContact.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (!phnNum.getText().toString().equals(""))
					AddContact(phnNum.getText().toString());
			}
		});
		
		btnSendText.setOnClickListener(new View.OnClickListener() {
			public void onClick(View arg0) {
				if (!phnNum.getText().toString().equals(""))
				{
				Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.fromParts("smsto", 
						phnNum.getText().toString(), null));
				startActivity(intent);
				}
				else
				{
					showToast("No number to send");
				}
				
			}
		});
		
		btnImportLast.setOnClickListener(new View.OnClickListener() {
			public void onClick(View arg0) {
				String number = "";
				if (prefs.getString("currentNumber", "").equals(""))
				{
					String[] projection = new String[] { BaseColumns._ID,
							CallLog.Calls.NUMBER, CallLog.Calls.TYPE };
					ContentResolver resolver = getContentResolver();
					Cursor cur = resolver.query(CallLog.Calls.CONTENT_URI, projection,
							null, null, CallLog.Calls.DEFAULT_SORT_ORDER);
					int numberColumn = cur.getColumnIndex(CallLog.Calls.NUMBER);
					if (!cur.moveToNext()) {
						cur.close();
					}
					if (cur.getCount()!=0)
						number = cur.getString(numberColumn);
				}
				else
				{
					number = prefs.getString("currentNumber", "");
				}
				
				if (!number.equals(""))
					phnNum.setText(number);
			}
		});

	}

	public void performDial() {
		if (!phnNum.getText().toString().equals(""))
			startActivity(new Intent(Intent.ACTION_CALL, Uri.parse("tel:"
					+ phnNum.getText())));
		else
			showToast("Nothing to dial");
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

	private void adjustInputType() {
		boolean allowFullString = prefs.getBoolean("allowString", false);
		if (allowFullString) {
			phnNum.setInputType(InputType.TYPE_CLASS_TEXT);
		} else {
			phnNum.setInputType(InputType.TYPE_CLASS_PHONE);
		}
	}

	@Override
	protected void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		if (ShouldIStartSpeakerphone()) {
			audioman.setSpeakerphoneOn(true);
			iHaveTurnedOnSpeakerphone = true;
		}
		
		if (IShouldRemoveNotification())
		{
			String ns = Context.NOTIFICATION_SERVICE;
			NotificationManager mNotificationManager = (NotificationManager)getSystemService(ns);
			mNotificationManager.cancel(0);
		}
		
	}

	private boolean IShouldRemoveNotification() {
		return (telephoneman.getCallState()==TelephonyManager.CALL_STATE_IDLE);
	}

	private boolean ShouldIStartSpeakerphone() {
		return (prefs.getBoolean("speaker", false)
				&& !audioman.isSpeakerphoneOn() && !audioman.isBluetoothScoOn() && (telephoneman
				.getCallState() == TelephonyManager.CALL_STATE_OFFHOOK));
	}

	@Override
	protected void onStop() {
		super.onStop();
		if (iHaveTurnedOnSpeakerphone) {
			audioman.setSpeakerphoneOn(false);
			iHaveTurnedOnSpeakerphone = false;
		}

		if (prefs.getBoolean("autosave", false) && !phnNum.getText().equals("")) {
			SetText(phnNum.getText().toString());
		}
	}

	private void AddContact(String number) {
		Intent createIntent = new Intent(Intent.ACTION_INSERT_OR_EDIT);
		createIntent.setType(People.CONTENT_ITEM_TYPE);
		createIntent.putExtra(Insert.PHONE, number);
		startActivity(createIntent);
	}

	private void SetText(String Text) {
		if (prefs.getBoolean("useClipboard", true)) {
			ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
			clipboard.setText(Text);
		} else {
			prefs.edit().putString("savedNum", Text).commit();
		}

	}

}
