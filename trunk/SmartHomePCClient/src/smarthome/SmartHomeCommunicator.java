package smarthome;

import app.Com;
import app.Parameters;

/**
 * 
 * using http://code.google.com/p/giovynetserialport/
 * 
 * @author rshilkr
 * 
 */
public class SmartHomeCommunicator implements Runnable {

	public static void main(String[] args) throws Exception {
		// *** Open port COM1
		Parameters param = new Parameters();
		param.setPort("COM19");
		param.setBaudRate("38400");
		Com com1 = new Com(param);

		// *** Send four characters every 400 milli seconds
		for (int i = 0; i < 4; i++) {
			Thread.sleep(400);
			com1.sendSingleData('G');
		}

		// *** Receives characters every 400 milli seconds
		for (int i = 0; i < 4; i++) {
			Thread.sleep(400);
			System.out.println((int)com1.receiveSingleChar());
		}

		// *** Close COM1
		com1.close();
	}

	public SmartHomeCommunicator() {
		// TODO Auto-generated constructor stub
	}

	@Override
	public void run() {
		// TODO Auto-generated method stub

	}

}
