package com.arnonse.savenum;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

public class Listen extends BroadcastReceiver {

	@Override
	public void onReceive(final Context context, final Intent intent) {

		CallStateListener phoneListener = new CallStateListener(context);
		TelephonyManager telephony = (TelephonyManager) context
				.getSystemService(Context.TELEPHONY_SERVICE);

		telephony.listen(phoneListener, PhoneStateListener.LISTEN_CALL_STATE);

	}

}
