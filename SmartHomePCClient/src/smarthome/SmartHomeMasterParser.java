package smarthome;

import java.io.Console;
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
