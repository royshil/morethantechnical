package smarthome;

public interface ISmartHomeMaster {
	/**
	 * test the communication with the station
	 * @param index
	 */
	void testStation(int index);
	/**
	 * poll the station for it's data
	 * @param index
	 */
	void poll(int index);
	/**
	 * toggle a switch on the station
	 * @param index
	 * @param on_off
	 */
	void toggle(int index, int switchNum, boolean on_off);
}
