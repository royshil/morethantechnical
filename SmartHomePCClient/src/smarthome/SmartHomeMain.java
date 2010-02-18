package smarthome;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;

public class SmartHomeMain implements ISmartHomeGUI {
	
	public CommSmartHomeListener smartHomeListener;
	protected boolean m_shouldCancel = false;
	private SmartHomeWidget m_shell;
	private Composite[] m_stations;

	public SmartHomeMain(SmartHomeWidget shell) {
		smartHomeListener = new CommSmartHomeListener(this);

		m_shell = shell;
		
		m_stations = new Composite[] {m_shell.station1_cmpst,
				m_shell.station1_cmpst1,m_shell.station1_cmpst2};
		
		redColor = new Color(Display.getDefault(), new RGB(255,0,0));
		greenColor = new Color(Display.getDefault(), new RGB(0,255,0));

		initWidget(shell);
	}

	public static void main(java.lang.String[] args) {
		final Display display = new Display();
		final SmartHomeWidget shell = new SmartHomeWidget();
		shell.createSShell();
		final Shell getsShell = shell.getsShell();
				
		final SmartHomeMain main = new SmartHomeMain(shell);
		
		getsShell.open();

		while (!getsShell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
		
		if(main != null) {
			try {
				main.smartHomeListener.stopListener();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
			
		display.dispose();
	}

	public Color redColor;//new Color(Display.getDefault(), new RGB(255,0,0));  //  @jve:decl-index=0:
	public Color greenColor;//new Color(Display.getDefault(), new RGB(255,0,0));  //  @jve:decl-index=0:

	/**
	 * initialize the widget appearance
	 * @param shell
	 */
	private void initWidget(final SmartHomeWidget shell) {
		final Shell getsShell = shell.getsShell();
		getsShell.setBackgroundMode(SWT.INHERIT_FORCE);		

		for (int i = 0; i < 3; i++) stationDown(i);

		shell.button_connect.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				if(SmartHomeMain.this.isM_shouldCancel()) {
					SmartHomeMain.this.setM_shouldCancel(false);
					shell.button_connect.setText("Connect");
					shell.textArea.append("Trying to close thread.\n");
					try {
						SmartHomeMain.this.smartHomeListener.stopListener();
					} catch (InterruptedException e1) {
						e1.printStackTrace();
					}
				} else {
					shell.button_connect.setText("Cancel");
					SmartHomeMain.this.setM_shouldCancel(true);
					try {
						SmartHomeMain.this.smartHomeListener.startListener();
					} catch (Exception e1) {
						shell.textArea.append(e1.getLocalizedMessage());
					}
				}
			}
		});
		
		shell.button_R.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				SmartHomeMain.this.smartHomeListener.setM_doReset(true);
			}
		});
		
		Listener listener = new Listener () {
			public void handleEvent (Event e) {
				Control [] children = getsShell.getChildren ();
				for (int i=0; i<children.length; i++) {
					Control child = children [i];
					if (e.widget != child && child instanceof Button && (child.getStyle () & SWT.TOGGLE) != 0) {
						((Button) child).setSelection (false);
					}
				}
				((Button) e.widget).setSelection (true);
			}
		};
		shell.radioButton_station1.addListener(SWT.Selection, listener);
		shell.radioButton_station2.addListener(SWT.Selection, listener);
		shell.radioButton_station3.addListener(SWT.Selection, listener);
		
		shell.button_turnON.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				SmartHomeMain.this.stationUp(1);
			}
		});
		
//		shell.button_connect.setEnabled(false);
//		shell.combo.setEnabled(false);
//		shell.button_R.setEnabled(false);
//		Thread t = new Thread(new Runnable() {
//			@Override
//			public void run() {
//				Display.getDefault().asyncExec(new Runnable() {
//				       public void run() {
//				    	   shell.textArea.append("querying OS for com ports... may take a while\n");
//				       }
//				    }
//				);
//				
//				final Enumeration<?> en = CommPortIdentifier.getPortIdentifiers();
//				
//				while(en.hasMoreElements()) {
//					Display.getDefault().asyncExec(new Runnable() {
//					       public void run() {
//									shell.combo.add(en.toString());
//					       }
//					    }
//					);
//				}
//				Display.getDefault().asyncExec(new Runnable() {
//				       public void run() {
//							shell.textArea.append("DONE query\n");
//							shell.button_connect.setEnabled(true);
//							shell.combo.setEnabled(true);
//							shell.button_R.setEnabled(true);
//				       }
//				}
//				);
//			}
//		});
//		t.start();
		
		for (int i = 1; i < 21; i++) {
			shell.combo.add("COM"+i);
		}
		shell.combo.select(0);
	}

	@Override
	public int getBaudRate() {
		return Integer.parseInt(m_shell.text_baudRate.getText());
	}

	@Override
	public String getCOMPort() {
		return m_shell.combo.getItem(m_shell.combo.getSelectionIndex());
	}

	@Override
	public void logMessage(final String s) {
		Display.getDefault().asyncExec(new Runnable() {
		       public void run() {
		    	   m_shell.textArea.append(s + "\n");
		       }
	    	}
		);
	}

	@Override
	public void stationDown(int index) {
		m_stations[index].setBackground(redColor);
		m_stations[index].setEnabled(false);
	}

	@Override
	public void stationUp(int index) {
		m_stations[index].setBackground(greenColor);
		m_stations[index].setEnabled(true);
	}

	public boolean isM_shouldCancel() {
		return m_shouldCancel;
	}
	public void setM_shouldCancel(boolean mShouldCancel) {
		m_shouldCancel = mShouldCancel;
	}
}
