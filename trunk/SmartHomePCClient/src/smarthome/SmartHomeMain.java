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

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;

public class SmartHomeMain implements ISmartHomeGUI {
	
	public CommSmartHomeListener smartHomeListener;
	protected boolean m_shouldCancel = false;
	private SmartHomeWidget m_shell;
	private Composite[] m_stationsComposites;
	private int[] m_stationsTemp;
	private Canvas[] m_stationsCanvas;

	public SmartHomeMain(SmartHomeWidget shell) {
		smartHomeListener = new CommSmartHomeListener(this);

		m_shell = shell;
		
		m_stationsComposites = new Composite[] {m_shell.station1_cmpst,
				m_shell.station1_cmpst1,m_shell.station1_cmpst2};
		m_stationsTemp = new int[] {0,0,0};
		m_stationsCanvas = new Canvas[] {m_shell.canvas,m_shell.canvas1,m_shell.canvas2};
		
		redColor = new Color(Display.getDefault(), new RGB(255,0,0));
		greenColor = new Color(Display.getDefault(), new RGB(0,255,0));
		defaultBackground = Display.getDefault().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND);

		initWidget(shell);
	}

	public static void main(java.lang.String[] args) {
		final Display display = new Display();
		final SmartHomeWidget shell = new SmartHomeWidget();
		shell.createSShell();
		final Shell getsShell = shell.getsShell();
		getsShell.setImage(new Image(display, "icon.bmp"));
				
		final SmartHomeMain main = new SmartHomeMain(shell);
		simulatorThread = new Thread(new SmartHomeSimulator(main));
//		simulatorThread.start();
		
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
			
		simulatorThread.interrupt();
		display.dispose();
	}

	public Color redColor;//new Color(Display.getDefault(), new RGB(255,0,0));  //  @jve:decl-index=0:
	public Color greenColor;//new Color(Display.getDefault(), new RGB(255,0,0));  //  @jve:decl-index=0:
	private Color defaultBackground;
	private static Thread simulatorThread;

	/**
	 * initialize the widget appearance
	 * @param shell
	 */
	private void initWidget(final SmartHomeWidget shell) {
		final Shell getsShell = shell.getsShell();
		getsShell.setBackgroundMode(SWT.INHERIT_FORCE);	
		shell.master_group.setBackgroundMode(SWT.INHERIT_NONE);

		for (int i = 0; i < 3; i++) stationDown(i);

		shell.button_connect.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				if(SmartHomeMain.this.isM_shouldCancel()) {
					SmartHomeMain.this.setM_shouldCancel(false);
					shell.button_connect.setText("Connect");
					shell.textArea.append("Trying to close thread.\n");
					shell.combo.setEnabled(true);
					if(shell.combo.getItem(shell.combo.getSelectionIndex()).equals("Simulator")) {
						simulatorThread.interrupt();
					} else {
						try {
							SmartHomeMain.this.smartHomeListener.stopListener();
						} catch (InterruptedException e1) {
							e1.printStackTrace();
						}
					}
					shell.master_group.setBackground(defaultBackground);
					shell.combo.setBackground(Display.getCurrent().getSystemColor(SWT.COLOR_LIST_BACKGROUND));
					shell.text_baudRate.setBackground(Display.getCurrent().getSystemColor(SWT.COLOR_WHITE));
				} else {
					shell.button_connect.setText("Cancel");
					SmartHomeMain.this.setM_shouldCancel(true);
					shell.combo.setEnabled(false);
					if(shell.combo.getItem(shell.combo.getSelectionIndex()).equals("Simulator")) {
						simulatorThread.start();
					} else {
						try {
							SmartHomeMain.this.smartHomeListener.startListener();
						} catch (Exception e1) {
							shell.textArea.append(e1.getLocalizedMessage());
						}
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
		
		final Button[] btns = new Button[] { shell.radioButton_station1,
			shell.radioButton_station2, shell.radioButton_station3
		};
		Listener listener = new Listener () {
			public void handleEvent (Event e) {
//				Control [] children = getsShell.getChildren ();
				for (int i=0; i<3; i++) {
//					Control child = children [i];
					Button btn = btns[i];
					if (e.widget != btn) { // (btn.getStyle () & SWT.TOGGLE) != 0) {
						btn.setSelection (false);
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
		shell.combo.add("Simulator");
		shell.combo.select(0);
		
		PaintListener pl = new PaintListener() {
			@Override
			public void paintControl(PaintEvent e) {
				int id = (Integer)e.widget.getData("name");
				e.gc.setBackground(Display.getCurrent().getSystemColor(SWT.COLOR_BLACK));
				e.gc.fillRectangle(0, 0, e.width, e.height);

				e.gc.setBackground(Display.getCurrent().getSystemColor(SWT.COLOR_CYAN));
				int h = (int)Math.round((double)e.height / 100.0 * (double)m_stationsTemp[id]);
				e.gc.fillRectangle(0, e.height - h - 1, e.width - 2, h);
				
				Font font = new Font(Display.getCurrent(),"Arial",14,SWT.BOLD | SWT.ITALIC); 
				e.gc.setForeground(Display.getCurrent().getSystemColor(SWT.COLOR_BLUE)); 
				e.gc.setFont(font);
				if(m_stationsTemp[id] == 0) {
					e.gc.drawString("N/A",(int)((double)e.width/2.0-17.0),(int)((double)e.height/2.0-14.0),true);
				} else {
					e.gc.drawString(String.valueOf(m_stationsTemp[id]),(int)((double)e.width/2.0-14.0),(int)((double)e.height/2.0-14.0),true);
				}
			}
		};
		for (int i = 0; i < 3; i++) {
			m_stationsCanvas[i].addPaintListener(pl);
		}
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
	public void stationDown(final int index) {
		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				m_stationsComposites[index].setBackground(redColor);
				m_stationsComposites[index].setEnabled(false);
			}
		});
	}

	@Override
	public void stationUp(final int index) {
		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				m_stationsComposites[index].setBackground(greenColor);
				m_stationsComposites[index].setEnabled(true);
			}
		});
	}

	public boolean isM_shouldCancel() {
		return m_shouldCancel;
	}
	public void setM_shouldCancel(boolean mShouldCancel) {
		m_shouldCancel = mShouldCancel;
	}

	@Override
	public void stationTemp(final int index, final int temp) {
		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				m_stationsTemp[index] = temp;
				m_stationsCanvas[index].redraw();
			}
		});
	}

	@Override
	public void master_online() {
		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				m_shell.master_group.setBackground(greenColor);
			}
		});
	}

	@Override
	public void showError(String title, String description) {
		Display.getDefault().asyncExec(new Runnable() {
			public void run() {
				m_shell.master_group.setBackground(redColor);
			}
		});		
	}
}
