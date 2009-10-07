package org.geekcon.runvas;

import java.io.*;
import java.net.*;
import java.util.*;

public class UDPServerThread extends Thread {

    protected DatagramSocket socket = null;
    protected BufferedReader in = null;
    protected boolean moreQuotes = true;

    public UDPServerThread() throws IOException {
	this("QuoteServerThread");
    }

    public UDPServerThread(String name) throws IOException {
        super(name);
        socket = new DatagramSocket(4445);

//        try {
//            in = new BufferedReader(new FileReader("one-liners.txt"));
//        } catch (FileNotFoundException e) {
//            System.err.println("Could not open quote file. Serving time instead.");
//        }
    }

    public void run() {

        while (moreQuotes) {
            try {
                byte[] buf = new byte[256];

                    // receive request
                DatagramPacket packet = new DatagramPacket(buf, buf.length);
                socket.receive(packet);
                
                System.out.println(new String(buf,"US-ASCII"));

                //TODO: parse 
                //[[#id:12,#size:10,#color:[172,234,12],#loc:[100,200], #speed:[20,30]],[#id:13,#size:10,#color:[172,234,12],#loc:[100,200], #speed:[20,30]]]
            } catch (IOException e) {
                e.printStackTrace();
		moreQuotes = false;
            }
        }
        socket.close();
    }

    protected String getNextQuote() {
        String returnValue = null;
        try {
            if ((returnValue = in.readLine()) == null) {
                in.close();
		moreQuotes = false;
                returnValue = "No more quotes. Goodbye.";
            }
        } catch (IOException e) {
            returnValue = "IOException occurred in server.";
        }
        return returnValue;
    }
}
