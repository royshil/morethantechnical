package org.geekcon.runvas;

import java.io.BufferedReader;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.text.ParseException;

public class UDPServerThread extends RunvasObjectsStreamer {

    protected DatagramSocket socket = null;
    protected BufferedReader in = null;

    public UDPServerThread() throws IOException {
    	this("UDPServerThread");
    }

    public UDPServerThread(String name) throws IOException {
        super(name);
        socket = new DatagramSocket(4450);
    }

    private long m_time = 0l;
    private long m_lastReportTime = 0l;
	private int m_countBytes = 0;
	private int m_countPackets = 0;
    
    public void run() {
		System.out.println("Runvas server online.");

		byte[] buf = new byte[2048];
		m_time = m_lastReportTime = System.currentTimeMillis();
		try {
			socket.setSoTimeout(1000);
		} catch (SocketException e1) {
			e1.printStackTrace();
		}
        while (serverOn) {
            try {
                DatagramPacket packet = new DatagramPacket(buf, buf.length);
                socket.receive(packet);
                
                m_countPackets++;
                m_countBytes += packet.getLength();
                
                String string = new String(buf,"US-ASCII");
//				System.out.println(string);

				parse(string);
				
				m_time = System.currentTimeMillis();
				
				if(Math.abs(m_time - m_lastReportTime) > 10000l) {
					m_lastReportTime = m_time;
					System.out.println("UDP server report: read " + m_countObj + " objects, from " + m_countBytes + " bytes in " + m_countPackets + " packets");
					m_countBytes = m_countObj = m_countPackets = 0;
				}

            } catch (SocketTimeoutException e) {
            	;//nop
			} catch (IOException e) {
                e.printStackTrace();
            } catch (ParseException e) {
				e.printStackTrace();
			}
        }
        socket.close();
        System.out.println("Runvas server shutdown.");
    }
}
