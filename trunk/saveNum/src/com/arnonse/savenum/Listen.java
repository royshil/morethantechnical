package com.arnonse.savenum;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
//import android.util.Log;

public class Listen extends BroadcastReceiver {
	private static CallStateListener phoneListener = null;
	private static boolean firstTime = true;
	@Override
	public void onReceive(final Context context, final Intent intent) {
		if (firstTime)
		{
			if (phoneListener==null)
				phoneListener = new CallStateListener(context);
			TelephonyManager telephony = (TelephonyManager) context
			.getSystemService(Context.TELEPHONY_SERVICE);

			telephony.listen(phoneListener, PhoneStateListener.LISTEN_CALL_STATE);
		}
		firstTime = false;
	}

}
