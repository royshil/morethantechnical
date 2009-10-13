package org.geekcon.runvas.tests;

import java.io.IOException;

import org.geekcon.runvas.RunvasObjectsStreamer;
import org.geekcon.runvas.UDPServerThread;

import junit.framework.TestCase;

public class TestUDPServerThread extends TestCase {
	public void testParse() {
		try {
			RunvasObjectsStreamer t;
			t = new UDPServerThread();
			t.parse("2,[[#id:12,#size:10,#color:[172,234,12],#loc:[100,200], #speed:[20,30],#rect:[10,10,10,10]],[#id:13,#size:10,#color:[172,234,12],#loc:[100,200], #speed:[20,30],#rect:[10,10,10,10]]]");
			assertTrue(true);
		} catch (Exception e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}
	
	public void testServer() {
//		try {
/*			UDPServerThread serverThread = new UDPServerThread();
			serverThread.start();
			
			Thread.sleep(10000);
			
			serverThread.serverOn = false;
			serverThread.interrupt();
			serverThread.join();
			
*/			assertTrue(true);
//		} catch (IOException e) {
//			e.printStackTrace();
//			assertTrue(false);
//		} catch (InterruptedException e) {
//			e.printStackTrace();
//			assertTrue(false);
//		}
	}
}
