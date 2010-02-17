package smarthome;

import gnu.io.CommPortIdentifier;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.UnsupportedCommOperationException;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

public class SmartHomeMain implements Runnable {
	private SerialPort serialPort;
	private InputStream mInputFromPort;
	private Thread listenerThread;
	private boolean m_running;

	public static void main(java.lang.String[] args) {
		final Display display = new Display();
		final SmartHomeWidget shell = new SmartHomeWidget();
		shell.createSShell();
		final Shell getsShell = shell.getsShell();
		
//		final int time = 500;
//		Runnable timer = new Runnable() {
//			public void run() {
//				if (getsShell.isDisposed())
//					return;
//				Point point = display.getCursorLocation();
//				Rectangle rect = getsShell.getBounds();
//				if (rect.contains(point)) {
//					shell.textArea.append("IN");
//				} else {
//					shell.textArea.append("OUT");
//				}
//				display.timerExec(time, this);
//			}
//		};
//		display.timerExec(time, timer);

		shell.station1_cmpst.setBackground(new Color(display, new RGB(255,0,0)));
		shell.station1_cmpst1.setBackground(new Color(display, new RGB(255,0,0)));
		shell.station1_cmpst2.setBackground(new Color(display, new RGB(255,0,0)));
		shell.station1_cmpst.setEnabled(false);
		shell.station1_cmpst1.setEnabled(false);
		shell.station1_cmpst2.setEnabled(false);
		
		getsShell.open();

		//TODO: any errors should be displayed on GUI
		SmartHomeMain main = null;
		try {
			main = new SmartHomeMain(shell);
//			main.startListener();
		} catch (NoSuchPortException e) {
			shell.textArea.append(e.getLocalizedMessage());
//			e.printStackTrace();
		} catch (PortInUseException e) {
			shell.textArea.append(e.getLocalizedMessage());
//			e.printStackTrace();
		} catch (UnsupportedCommOperationException e) {
			shell.textArea.append(e.getLocalizedMessage());
//			e.printStackTrace();
		}

		while (!getsShell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
		
		if(main != null) {
			try {
				main.stopListener();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
			
		display.dispose();
	}

	private void startListener() {
		m_running = true;
		listenerThread.start();
	}

	private void stopListener() throws InterruptedException {
		m_running = false;
		listenerThread.join();
		serialPort.close();
	}

	private SmartHomeWidget m_shell;
	public SmartHomeMain(SmartHomeWidget shell) throws NoSuchPortException, PortInUseException, UnsupportedCommOperationException {
		m_shell = shell;
		init();
		listenerThread = new Thread(this);
	}

	private boolean init() throws NoSuchPortException, PortInUseException,
			UnsupportedCommOperationException {
		CommPortIdentifier portIdentifier = CommPortIdentifier
				.getPortIdentifier("COM4");
		if (portIdentifier.isCurrentlyOwned()) {
			m_shell.textArea.append("Port in use!");
			return false;
		}

		m_shell.textArea.append("name=" + portIdentifier.getName());

		serialPort = (SerialPort) portIdentifier.open("ListPortClass", 38400);

		int b = serialPort.getBaudRate();

		m_shell.textArea.append("baud=" + Integer.toString(b));

		serialPort.setSerialPortParams(38400, SerialPort.DATABITS_8,
				SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

		try {
			mInputFromPort = serialPort.getInputStream();
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		serialPort.setDTR(false);
		serialPort.setRTS(false);

		return true;
	}

	private void startComListner() throws InterruptedException {
		BufferedReader br = new BufferedReader(new InputStreamReader(
				mInputFromPort));

		char[] cbuf = new char[128];
		while(m_running) {
			Thread.sleep(30);
			int read;
			try {
				read = br.read(cbuf, 0, 128);
				if (read == 0) {
					continue;
				}
				final String string = new String(cbuf, 0, read);
				System.out.print(string);
				Display.getDefault().asyncExec(new Runnable() {
		               public void run() {
		            	   m_shell.textArea.append(string);
		               }
		            }
				);
			} catch (IOException e) {
				;
			}
		}

		try {
			br.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void run() {
		try {
			startComListner();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}
