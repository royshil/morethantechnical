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

import gnu.io.CommPortIdentifier;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.UnsupportedCommOperationException;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A smart home listener that works with serial COM ports
 * @author rshilkr
 *
 */
public class CommSmartHomeListener extends AbstractSmartHomeListener implements Runnable, ISmartHomeMaster {
	private SerialPort serialPort;
	private InputStream mInputFromPort;
	private OutputStream mOutputToPort;
	private Thread listenerThread;
	private boolean m_running;

	private String m_comPort;
	private int m_baudRate;
	private boolean m_doReset = false;
	
	private SmartHomeMasterParser parser;

	public void startListener() throws NoSuchPortException, PortInUseException, UnsupportedCommOperationException {
		m_comPort = m_gui.getCOMPort();
		m_baudRate = m_gui.getBaudRate();
		m_running = true;
		listenerThread.start();
	}

	public void stopListener() throws InterruptedException {
		m_running = false;
		if(listenerThread.isAlive()) {
			logToTextArea("trying to shut down listener thread");
			listenerThread.join(1000);
			while(listenerThread.isAlive()) {
				logToTextArea("Thread not dead yet - interrupt");
				listenerThread.interrupt();
				listenerThread.join(2000);
			}
			logToTextArea("Thread dead");
		}
	}

	public CommSmartHomeListener(ISmartHomeGUI g) {
		super(g);
//		m_shell = shell;
		listenerThread = new Thread(this);
		
		parser = new SmartHomeMasterParser(g);
	}

	private boolean init(String comPort, int baudRate) throws NoSuchPortException, PortInUseException,
			UnsupportedCommOperationException, InterruptedException {
		System.out.println("init()");
		logToTextArea("Getting port identifier for " + comPort + ".. please wait");
		CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(comPort);
		
		if (portIdentifier.isCurrentlyOwned()) {
			logToTextArea("Port in use!");
			return false;
		}

		logToTextArea("Try to open port... "+comPort+"/"+baudRate+"/8N1");
		serialPort = (SerialPort) portIdentifier.open("ListPortClass", baudRate);
		
		serialPort.setSerialPortParams(baudRate, SerialPort.DATABITS_8,
				SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

		try {
			mInputFromPort = serialPort.getInputStream();
			mOutputToPort = serialPort.getOutputStream();
		} catch (IOException e) {
			e.printStackTrace();
			logToTextArea(e.getLocalizedMessage());
			return false;
		}
		logToTextArea("Set DTR, Clear RTS");
		serialPort.setRTS(false);
		serialPort.setDTR(true);
		
		Thread.sleep(500);
		
		logToTextArea("Clear DTR");
		serialPort.setDTR(false);

		logToTextArea("Port open");
		return true;
	}

	private void startComListner() throws InterruptedException {
//		BufferedReader br = new BufferedReader(new InputStreamReader(mInputFromPort));
//		InputStreamReader br = new InputStreamReader(mInputFromPort);
//
//		char[] cbuf = new char[128];
//		CharBuffer cb = CharBuffer.wrap(cbuf);
//		byte[] bbuf = new byte[128];
//		ByteBuffer bb = ByteBuffer.wrap(bbuf);
		StringBuilder sb = new StringBuilder();
		while(m_running) {
			Thread.sleep(1);
//			int read;
			try {
//				read = br.read(cb);
//				read = mInputFromPort.read(bbuf, 0, bbuf.length);
				int c = mInputFromPort.read();
				if (c < 0) {
					continue;
				}
//				System.out.println(new String(bbuf,0,read));
//				for (int i = 0; i < bbuf.length; i++) {
					if(c == '\n') {
						if(sb.length() > 0) {
							//now sb contains a line
							String line = sb.toString().trim();

							if(line.startsWith("pc")) {
								logToTextArea(line);
								
								parser.parseMasterCommand(line);
							}
							
							sb = new StringBuilder();
						}
					} else if (c == '\r'){
						//ignore CRs
					} else {
						sb.append((char)c);
					}
//				}
			} catch (IOException e) {
				;
			}
			
			if(m_doReset) {
				logToTextArea("Set DTR, Clear RTS");
				serialPort.setDTR(true);
				
				Thread.sleep(500);
				
				logToTextArea("Clear DTR");
				serialPort.setDTR(false);
				
				m_doReset = false;
			}
		}

		try {
//			br.close();
			mInputFromPort.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void run() {
		logToTextArea("Listener thread running");
		try {
			if(!init(m_comPort,m_baudRate)) {
				logToTextArea("Error opening listener port");
			} else {
				startComListner();
			}
		} catch (final Exception e) {
			e.printStackTrace();
			logToTextArea(e.getLocalizedMessage());
		} finally {
			logToTextArea("Closing port");
			serialPort.close();
			logToTextArea("Port closed.");
		}
	}

	private void logToTextArea(final String string) {
		m_gui.logMessage(string);
	}
	
	public void setM_running(boolean mRunning) {
		m_running = mRunning;
	}
	public boolean isM_doReset() {
		return m_doReset;
	}

	public void setM_doReset(boolean mDoReset) {
		m_doReset = mDoReset;
	}

	static final String pollTemplate = "m=poll_%d";
	static final String testTemplate = "m=test_%d";
	static final String toggleTemplate = "m=toggle_%d_%d_%d";
	
	@Override
	public void poll(int index) {
		byte[] buf = String.format(pollTemplate, index).getBytes();
		writeToPort(buf);
	}

	@Override
	public void testStation(int index) {
		byte[] buf = String.format(testTemplate, index).getBytes();
		writeToPort(buf);
	}

	@Override
	public void toggle(int index, int switchNum, boolean onOff) {
		byte[] buf = String.format(toggleTemplate, index,switchNum,onOff?1:0).getBytes();
		writeToPort(buf);
	}

	private void writeToPort(byte[] buf) {
		try {
			mOutputToPort.write(buf);
		} catch (IOException e) {
			logToTextArea("ERROR: " + e.getLocalizedMessage());
		}
	}
}
