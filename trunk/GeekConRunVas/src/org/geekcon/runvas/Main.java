package org.geekcon.runvas;

import java.nio.FloatBuffer;

import javax.media.opengl.GL;
import javax.media.opengl.GLAutoDrawable;
import javax.media.opengl.GLCanvas;
import javax.media.opengl.GLCapabilities;
import javax.media.opengl.GLEventListener;
import javax.media.opengl.glu.GLU;

import org.eclipse.swt.SWT;
import org.eclipse.swt.awt.SWT_AWT;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

/* *******************************************
 * check out our demo at 
 * http://www.youtube.com/watch?v=XbQ0Qd3gHZM
 * ****************************************** 
 */


public class Main implements GLEventListener, Runnable
{

	private GameController game;

	public static void main(String[] args) 
	{
		
		Display display = new Display();
		
		Main main = new Main();
		main.runMain(display);
		display.dispose();
	}
	
	GLCanvas canvas;
	
	void runMain(Display display)
	{
		final Shell shell = new Shell(display);
		shell.setText("Snails3D");
		GridLayout gridLayout = new GridLayout();
		gridLayout.marginHeight = 0;
		gridLayout.marginWidth = 0;
	
		shell.setLayout(gridLayout);
		

		// this allows us to set particular properties for the GLCanvas
		GLCapabilities glCapabilities = new GLCapabilities();

		glCapabilities.setDoubleBuffered(true);
		glCapabilities.setHardwareAccelerated(true);

		// instantiate the canvas
		canvas = new GLCanvas(glCapabilities);

		// we can't use the default Composite because using the AWT bridge
		// requires that it have the property of SWT.EMBEDDED
		Composite composite = new Composite(shell, SWT.EMBEDDED);
		GridData ld = new GridData(GridData.FILL_BOTH);
		composite.setLayoutData(ld);
		
		// the grid layout allows you to add more widgets in the main window.
		
		// set the internal layout so our canvas fills the whole control
		FillLayout clayout = new FillLayout();
		composite.setLayout(clayout);

		// create the special frame bridge to AWT
		java.awt.Frame glFrame = SWT_AWT.new_Frame(composite);
		// we need the listener so we get the GL events
		canvas.addGLEventListener(this);

		// finally, add our canvas as a child of the frame
		glFrame.add(canvas);

		composite.setFocus();
		canvas.requestFocus();

		final Thread repaintThread = new Thread(this);

		composite.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent k) {
//			    System.out.println("pressed: " + k);
			    canvas.repaint();
			    
			    if(k.keyCode == java.awt.event.KeyEvent.VK_ESCAPE) {
			    	repaintThread.interrupt();
			    	shell.dispose();
			    }
			}
		});
		
		if(game==null) {
			game = new GameController();
		}

		composite.addKeyListener(game);
		
		canvas.addMouseMotionListener(game);
		canvas.addMouseListener(game);

		
		// show it all
		shell.open();

		repaintThread.start();

		// the event loop.
		while (!shell.isDisposed ()) {
			if (!display.readAndDispatch ()) display.sleep ();
		}
		
		repaintThread.interrupt();
	}


	public void init(GLAutoDrawable drawable) 
	{
	    GL gl = drawable.getGL();
	    gl.glShadeModel(GL.GL_SMOOTH);
	    gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	    gl.glClearDepth(1.0f);

	    gl.glEnable(GL.GL_BLEND);
	    gl.glBlendFunc(GL.GL_SRC_ALPHA,GL.GL_ONE_MINUS_SRC_ALPHA);

//	    gl.glEnable(GL.GL_DEPTH_TEST);
//	    gl.glDepthFunc(GL.GL_LEQUAL);
	    gl.glHint(GL.GL_PERSPECTIVE_CORRECTION_HINT, GL.GL_NICEST);
	    
	    gl.glMatrixMode(GL.GL_MODELVIEW);
	    gl.glLoadIdentity();	 
	    
	    timestamp = System.currentTimeMillis();
	    
	    //setup lighting
	    float[] a = {0.2f,0.2f,0.2f,1f};
	    gl.glLightfv(GL.GL_LIGHT1, GL.GL_AMBIENT, FloatBuffer.wrap(a));
	    float[] d = {1f,1f,1f,1f};
	    gl.glLightfv(GL.GL_LIGHT1, GL.GL_DIFFUSE, FloatBuffer.wrap(d));
	    float[] s = {.5f,.5f,.5f,1f};
	    gl.glLightfv(GL.GL_LIGHT1, GL.GL_SPECULAR, FloatBuffer.wrap(s));
	    float[] p = { -5f, 5f, 0f, 1f };
	    gl.glLightfv(GL.GL_LIGHT1, GL.GL_POSITION, FloatBuffer.wrap(p));
	    
	    gl.glEnable(GL.GL_LIGHT1);
	    gl.glEnable(GL.GL_LIGHTING);
	    
	    gl.glEnable(GL.GL_COLOR_MATERIAL);
	    gl.glColorMaterial ( GL.GL_FRONT_AND_BACK, GL.GL_AMBIENT_AND_DIFFUSE ) ;
	    
	    game.init(gl);
	}

	float color;
	long timestamp = 0;
	
	public void display(GLAutoDrawable drawable)
	{
	    GL gl = drawable.getGL();

	    gl.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT);
	    gl.glClearColor(0.1f, 0.1f, .3f, 0.0f);


//	    gl.glMatrixMode(GL.GL_PROJECTION);

	    long now = System.currentTimeMillis();
	    long diff = Math.max(now - timestamp,1l); 
	    timestamp = now;

	    game.drawGame(gl,drawable,diff);
	}


	public void reshape(GLAutoDrawable drawable, int x, int y, int width, int height)
	{
	    GL gl = drawable.getGL();
	    final GLU glu = new GLU();

	    gl.glViewport(0, 0, width, height);
	    gl.glMatrixMode(GL.GL_PROJECTION);
	    gl.glLoadIdentity();
	    glu.gluPerspective(45.0f, (double) width / (double) height, 0.1f, 1000.0f);
	    gl.glMatrixMode(GL.GL_MODELVIEW);

	}
	
	
	public void displayChanged(GLAutoDrawable arg0, boolean arg1, boolean arg2) {
		// should remain empty
	}

	public void run() {
		while(true) {
			canvas.repaint();
			try {
				Thread.sleep(25);
			} catch (InterruptedException e) {
				break;
			}
		}
	}
}
