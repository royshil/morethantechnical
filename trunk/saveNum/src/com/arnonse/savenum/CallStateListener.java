package com.arnonse.savenum;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

public class CallStateListener extends PhoneStateListener {

	Context ctx = null;

	CallStateListener(Context ctx) {
		super();
		this.ctx = ctx;

	}

	// Thread t = new Thread(new Runnable(){
	// public void run(){
	// try {
	// Thread.sleep(1000*60);
	// } catch (InterruptedException e) {
	// // TODO Auto-generated catch block
	// e.printStackTrace();
	// }
	// clearNotification();
	// }
	// });

	public void onCallStateChanged(int state, String incomingNumber) {
		switch (state) {
		case TelephonyManager.CALL_STATE_IDLE:

			// t.start();
			clearNotification();
			// Log.d("DEBUG", "IDLE");
			break;
		case TelephonyManager.CALL_STATE_OFFHOOK:

			// t.stop();
			activateNotification();

			break;
		case TelephonyManager.CALL_STATE_RINGING:
			// Log.d("DEBUG", "RINGING");
			break;
		}
	}

	public void activateNotification() {
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

		mManager.notify(APP_ID, notification);

	}

	public void clearNotification() {
		String ns = Context.NOTIFICATION_SERVICE;

		NotificationManager mNotificationManager = (NotificationManager) ctx
				.getSystemService(ns);

		mNotificationManager.cancel(0);
	}

}
