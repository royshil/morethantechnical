package smarthome;

public class SmartHomeSimulator implements Runnable {

	ISmartHomeGUI gui;
	
	public SmartHomeSimulator(ISmartHomeGUI g) {
		gui = g;
	}
	
	@Override
	public void run() {
		gui.logMessage("Simulator running");
		
		boolean[] upstations = new boolean[3];
		for (int i = 0; i < 3; i++) {
			upstations[i] = (Math.random()>0.5)?true:false;
			if(upstations[i]) gui.stationUp(i);
		}
		
		while(true) {
			int randSt = (int)Math.floor(Math.random()*3.0);
			while(!upstations[randSt]) {
				randSt = (int)Math.floor(Math.random()*3.0);
			}
//			gui.stationUp(randI);
			int randTemp = 30 + (int)Math.floor(Math.random()*5.0);
			gui.stationTemp(randSt, randTemp);
			gui.logMessage("Station " + randSt + " temp = " + randTemp);
			try {
				Thread.sleep(2500);
			} catch (InterruptedException e) {
//				e.printStackTrace();
				break;
			}
		}
		gui.logMessage("Simulator done");
	}
}
