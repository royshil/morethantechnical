package com.arnonse.pelesms;

public class SMSManager {
	
	private String cellnum = "";  // the username
	private String password = ""; // the password
	private String message = "";  // the SMS body
	private String prefix = "";   // target SMS prefix
	private String number = "";   // target SMS number
	private String returnMessage = "";
	
	public void SendSMS() {

	}


	public void setMessage(String msg)
	{
		message = msg;
	}
	
	public void setRecipiant(String nPrefix, String num)
	{
		prefix = nPrefix;
		number = num;
	}
	
	public String getMessage()
	{
		return returnMessage;
	}
	
	public void setCredentials(String user, String pass)
	{
		cellnum = user;
		password = pass;
	}
	
}
