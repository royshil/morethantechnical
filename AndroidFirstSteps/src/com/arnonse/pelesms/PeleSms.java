package com.arnonse.pelesms;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.view.Menu;

public class PeleSms extends Activity {

	Button btnSendSms;
	EditText messageBody;
	EditText smsTarget;
	String prefix;
	String number;
	private boolean getcontact = false;
	SharedPreferences prefs;
	SMSManager mySMSManager;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Check if there is an active network around
		if (!anyNetworkActive()) {
			showToast(getString(R.string.nonet));
			
			finish();
		}
		
		
		// Check if the program was called from the contact manager
		String param = getIntent().getDataString();

		// If so, check if this is a valid number
		if (param != null) {
			if (param.contains(":")) {
				param = param.split(":")[1];
			}
		}

		setContentView(R.layout.pelesms);
		
		// Initialize preferences manager
		prefs = PreferenceManager.getDefaultSharedPreferences(this);

		// Assign objecs from layout
		messageBody = (EditText) findViewById(R.id.smsBody);
		smsTarget = (EditText) findViewById(R.id.smsRec);
		btnSendSms = (Button) findViewById(R.id.btnSend);

		// If there is a valid number got from the contact manager, set the TO 
		// edit Text to this number
		if (param != null)
			smsTarget.setText(param.replace("-", ""));

		
		btnSendSms.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				mySMSManager = new SMSManager();
				// Validate username and password
				if (!validateCredentials())
					return;
				
				// Clear previos notification, if exist
				clearNotification();
				
				// Validate those valuses
				String validation = validateSms();
				
				if (validation.equals("")) { // Say the validation passed
					showToast("SMSing " + prefix + "-" + number);
					
					// Set the values for sms to be sent
					mySMSManager.setMessage(messageBody.getText().toString());
					mySMSManager.setRecipiant(prefix, number);

					// Send the sms with a thread
					new Thread(new Runnable() {
						public void run() {
							try {
								mySMSManager.SendSMS();
								notifyString("SMS Sent", "peleSms", mySMSManager
										.getMessage());
							} catch (Exception e) {
								notifyString("ERROR", "ERROR", e.getMessage());
							}
							mySMSManager = null;
						}
					}).start();
				} else
					// Validation failed
					showToast(validation);
			}
		});

	}

	/**
	 * Validation credentials are valid
	 * @return boolean
	 */
	protected boolean validateCredentials() {
		String user = prefs.getString("cellnum", "");
		String pass = prefs.getString("password", "");

		// Password should not be empty
		if (pass.equals("")) {
			showToast(getString(R.string.invalidpwd));
			return false;
		}
		//  Set the credentials
		mySMSManager.setCredentials(user, pass);
		return true;

	}

	/**
	 * Create the options menu
	 */
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(getString(R.string.menuconfig));
		menu.add(getString(R.string.menuclear));
		menu.add(getString(R.string.menucontact));
		return super.onCreateOptionsMenu(menu);
	}

	
	/**
	 * Act on menu item selection
	 */
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		if (item.getTitle().equals(getString(R.string.menuconfig)))
			// Show config screen
			startActivity(new Intent(this, Prefs.class));
		if (item.getTitle().equals(getString(R.string.menuclear))) {
			// Clear all fields on screen
			messageBody.setText("");
			smsTarget.setText("");
		}
		if (item.getTitle().equals(getString(R.string.menucontact))) {
			getcontact = true; // for sync issues
			// Delete the contactnum variable
			prefs.edit().putString("contactnum", "").commit();
			// Show contact selection screen
			startActivity(new Intent(this, ContactList.class));
		}
		return (true);
	}

	@Override
	protected void onResume() {
		super.onResume();
		// If i'm back from contact selection
		if (getcontact) {
			// No longer needed
			String newNumber = prefs.getString("contactnum", "");
			if (!newNumber.equals(""))
				// Assign the recieved number
				smsTarget.setText(newNumber);
			prefs.edit().putString("contactnum", "").commit();
		}
		getcontact = false;
	}

	/**
	 * SMS validation
	 * 1. Number - Extract prefix and number 
	 * 2. Message - Make sure not over 115 chars
	 */
	public String validateSms() {
		// length not over 115. Also now handled in the layout itself
		if (messageBody.getText().toString().length() > 115)
			return getString(R.string.errmsglong);
		// Non-blank message
		if (messageBody.getText().toString().length() == 0)
			return getString(R.string.errmsgblank);
		String smsNum = smsTarget.getText().toString();
		// If the number contains '-' it should be 11 digits long
		if (smsNum.contains("-") && smsNum.length() != 11)
			return getString(R.string.errnum);
		// If the numebr doesn't contain '-' it should be 10 digits long
		if (!smsNum.contains("-") && smsNum.length() != 10)
			return getString(R.string.errnum);

		// Assign the number and prefix
		if (smsNum.contains("-")) {
			prefix = smsNum.split("-")[0];
			number = smsNum.split("-")[1];

		} else {
			prefix = smsNum.substring(0, 3);
			number = smsNum.substring(3, 10);
		}
		// Validation passed
		return "";

	}
	
	/**
	 * Create notifictaion on notification-area
	 * @param sTickerText
	 * @param sContentTitle
	 * @param sContentText
	 */

	public void notifyString(String sTickerText, String sContentTitle,
			String sContentText) {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
		int icon = R.drawable.notification_icon;
		CharSequence tickerText = sTickerText;
		long when = System.currentTimeMillis();

		Notification notification = new Notification(icon, tickerText, when);
		Context context = getApplicationContext();
		CharSequence contentTitle = sContentTitle;
		CharSequence contentText = sContentText;
		Intent notificationIntent = new Intent(this, PeleSms.class);
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
				notificationIntent, 0);

		notification.setLatestEventInfo(context, contentTitle, contentText,
				contentIntent);
		final int HELLO_ID = 1;

		mNotificationManager.notify(HELLO_ID, notification);
	}
	
	/**
	 * Clear notification
	 */
	public void clearNotification() {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
		mNotificationManager.cancel(1);
	}
	
	/**
	 * Show toast popup
	 * @param s
	 */
	public void showToast(String s) {
		Toast.makeText(getBaseContext(), s, Toast.LENGTH_SHORT).show();
	}

	/**
	 * Check if there is an active network
	 * @return
	 */
	private boolean anyNetworkActive() {
		TelephonyManager tm = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
		WifiManager wm = (WifiManager) getSystemService(Context.WIFI_SERVICE);
		WifiInfo wi = wm.getConnectionInfo();
		return !((wi == null || WifiInfo.getDetailedStateOf(wi.getSupplicantState()) == DetailedState.IDLE) && 
				tm.getDataState() != TelephonyManager.DATA_CONNECTED);
	}

	
}
