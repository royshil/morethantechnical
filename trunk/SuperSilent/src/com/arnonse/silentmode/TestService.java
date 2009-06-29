package com.arnonse.silentmode;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;


public class TestService extends Service{

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	private void _startService()
	{

	}
	
	@Override
	public void onCreate()
	{
		super.onCreate();
		_startService();
	}
	
	@Override
	public void onDestroy() {
	  super.onDestroy();

	  _shutdownService();

	  
	}

	private void _shutdownService() {
	}

}

