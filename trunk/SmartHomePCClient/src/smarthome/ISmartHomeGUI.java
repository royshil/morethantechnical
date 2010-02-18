package smarthome;

public interface ISmartHomeGUI {
	/**
	 * This station is now live
	 * @param index
	 */
	void stationUp(int index);
	/**
	 * This station is down
	 * @param index
	 */
	void stationDown(int index);
	/**
	 * Log a message to the screen
	 * @param s
	 */
	void logMessage(String s);
	
	/**
	 * get the prefred COM port from the user
	 * @return
	 */
	String getCOMPort();
	/**
	 * get the baud rate from the user
	 * @return
	 */
	int getBaudRate();
}
