package org.geekcon.runvas;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.text.ParseException;

public class RunvasObjectFileReader extends RunvasObjectsStreamer {

	private static final int BUF_SIZE = 1024;

	public RunvasObjectFileReader() {
		super("RunvasObjectFileReader");
	}

	@Override
	public void run() {
		FileReader fr =null;
		try {
			 fr = new FileReader("output_geekcon3653.txt");
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return;
		}
		
		char buf[] = new char[BUF_SIZE];
//		boolean notDone = true;
		StringBuilder sb = new StringBuilder();
		while(serverOn) {
			try {
				int sbIndexOfPipe = sb.indexOf("|");
				if(sbIndexOfPipe < 0) {
					//no more full objects, read fromfile again
					int read = fr.read(buf, 0, BUF_SIZE);
					if(read<0) break;
					sb.append(new String(buf));
				}				
				
				String s = sb.toString();
				int indexOfPipe = s.indexOf('|');
				String obj = s.substring(0, indexOfPipe);
				
				sb = new StringBuilder(sb.substring(indexOfPipe+1));
				
				if(obj.length()>0) {
					parse(obj);
					Thread.sleep(30);
				}
			} catch (IOException e) {
				e.printStackTrace();
			} catch (ParseException e) {
				e.printStackTrace();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}
