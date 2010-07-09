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

import java.util.StringTokenizer;

public class SmartHomeMasterParser {
	private static final String PARSER_UP = "up=";
	private static final String PARSER_MASTER_ONLINE = "master_online";
	private static final String PARSER_TEMP = "temp=";

	ISmartHomeGUI m_gui;
	
	public SmartHomeMasterParser(ISmartHomeGUI g) {
		m_gui = g;
	}
	
	/**
	 * parse the "pc=" response from the master
	 * it contains information to be shown on the pc client
	 * @param line
	 */
	public void parseMasterCommand(String line) {
		line = line.substring(3);
		if(line.equals(PARSER_MASTER_ONLINE)) {
			//master module is online
			//example: pc=master_online
			m_gui.master_online();
		} else if(line.startsWith(PARSER_TEMP))	{
			//temperature reading:
			//example: pc=temp=2 31
			line = line.substring(PARSER_TEMP.length());
			StringTokenizer tokenizer = new StringTokenizer(line," ");
			int station = Integer.parseInt(tokenizer.nextToken());
			int temperature = Integer.parseInt(tokenizer.nextToken());
			
			m_gui.stationTemp(station, temperature);
		} else if(line.startsWith(PARSER_UP)) {
			//station is up
			//example: pc=up=2
			line = line.substring(PARSER_UP.length());
			int station = Integer.parseInt(line);
			
			m_gui.stationUp(station);
		} else if(line.startsWith("e=")) {
			//error
			//example: pc=e=Some error
			line = line.substring("e=".length());
			
			m_gui.showError("Master cannot initialize",line);
		}
	}
}
