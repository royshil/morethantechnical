/**
Copyright (c) 2010, Roy S.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package smarthome;

public class SmartHomeSimulator implements Runnable {

	ISmartHomeGUI gui;
	
	public SmartHomeSimulator(ISmartHomeGUI g) {
		gui = g;
	}
	
	@Override
	public void run() {
		gui.logMessage("Simulator running");
		
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e1) {
//			e1.printStackTrace();
			return;
		}
		gui.master_online();
		
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
		
		for (int i = 0; i < 3; i++) {
			if(upstations[i]) gui.stationDown(i);
		}
		gui.logMessage("Simulator done");
	}
}
