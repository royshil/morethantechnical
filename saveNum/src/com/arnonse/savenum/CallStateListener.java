package com.arnonse.savenum;

import java.util.Timer;
import java.util.TimerTask;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
//import android.util.Log;

public class CallStateListener extends PhoneStateListener {

	Context ctx = null;
	SharedPreferences prefs;
	Timer t = null;

	CallStateListener(Context ctx) {
		super();
		this.ctx = ctx;
		prefs = PreferenceManager.getDefaultSharedPreferences(ctx);
	}

	public void onCallStateChanged(int state, String incomingNumber) {
		switch (state) {
		case TelephonyManager.CALL_STATE_IDLE:
			//Log.d("DEBUG", "CALL STATE IDLE ****************");
			handleRemoveNotification();
			break;
		case TelephonyManager.CALL_STATE_OFFHOOK:
			//Log.d("DEBUG", "CALL STATE ACTIVE ***************");
			activateNotification();
			break;
		case TelephonyManager.CALL_STATE_RINGING:
			break;
		}
	}

	public void activateNotification() {

		cancelTimer();

		final int APP_ID = 0;

		NotificationManager mManager = (NotificationManager) ctx
				.getSystemService(Context.NOTIFICATION_SERVICE);
		Notification notification = new Notification(R.drawable.icon, null,
				System.currentTimeMillis());

		Intent notificationIntent = new Intent(ctx, saveNum.class);
		PendingIntent contentIntent = PendingIntent.getActivity(ctx, 0,
				notificationIntent, 0);

		notification.setLatestEventInfo(ctx, "Number Saver",
				"Click to open application", contentIntent);
		notification.flags = Notification.FLAG_ONGOING_EVENT;

		mManager.notify(APP_ID, notification);

	}

	private void cancelTimer() {
		if (t != null) {
			t.cancel();
			t.purge();
			t = null;
		}
	}

	private void handleRemoveNotification() {

		cancelTimer();

		int timeout = Integer.parseInt(prefs.getString("suspendTime", "0")) * 1000;
		if (timeout == 0)
			clearNotification();
		else {

			t = new Timer(true);
			t.schedule(new TimerTask() {
				@Override
				public void run() {
					clearNotification();
				}
			}, timeout);
		}
	}

	public void clearNotification() {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) ctx
				.getSystemService(ns);
		mNotificationManager.cancel(0);
		// pickedup=false;
	}

}
