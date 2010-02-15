package smarthome;

import java.awt.BorderLayout;

import org.eclipse.swt.*;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

public class SmartHomeMain {
	public static void main(java.lang.String[] args) {
		final Display display = new Display ();
	    final SmartHomeWidget shell = new SmartHomeWidget();
	    shell.createSShell();
	    final Shell getsShell = shell.getsShell();
	    
	    final int time = 500;
	    Runnable timer = new Runnable () {
			public void run () {
				if (getsShell.isDisposed()) return;
				Point point = display.getCursorLocation ();
				Rectangle rect = getsShell.getBounds ();
				if (rect.contains (point)) {
					shell.textArea.append("IN");
				} else {
					shell.textArea.append("OUT");
				}
				display.timerExec (time, this);
			}
		};
		display.timerExec (time, timer);
	    
		getsShell.open ();
	    while (!getsShell.isDisposed ()) {
	        if (!display.readAndDispatch ()) display.sleep ();
	    }
	    display.dispose ();
	}
	
	public SmartHomeMain() {
		Runnable r = new Runnable() {
			public void run() {
				
			}
		};
	}
}
