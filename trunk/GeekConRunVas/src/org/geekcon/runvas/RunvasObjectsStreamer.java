package org.geekcon.runvas;

import java.text.ParseException;

public abstract class RunvasObjectsStreamer extends Thread {

	public IRunvasObjectsController rnvsObjCtrlr;
	protected int m_countObj = 0;
	public boolean serverOn;
	
	public RunvasObjectsStreamer(String name) {
		super(name);
	}

	public void parse(String string) throws ParseException {
	        //23,[[#id:12,#size:10,#color:[172,234,12],#loc:[100,200], #speed:[20,30]],[#id:13,#size:10,#color:[172,234,12],#loc:[100,200], #speed:[20,30]]]
	    	try {
	    		
	        int count = Integer.parseInt(string.substring(0,string.indexOf(",")));
	        m_countObj += count;
	    	string = string.substring(string.indexOf(",")+1);
	    	int[] is = {0};
	        for (int i = 0; i < count; i++) {
	        	RunVasObject o = new RunVasObject();
	//        	System.out.println("object ["+i+"]: ");
				int id = Integer.parseInt(extractStringFromColonToComma(string,is));
				o.id = id;
//				o.id = 1;
	//			System.out.println("id: "+id);
				string = string.substring(is[0] + 1);
				int size = Integer.parseInt(extractStringFromColonToComma(string,is));
				o.size = size;
	//			System.out.println("size: " + size);
				string = string.substring(is[0] + 1);
				String colorStr = extractStringFromAtoB(string,is,"( "," )");
	//			System.out.println("color: " + colorStr);
				
				extractIntArrayOfSize(o.color, colorStr, 3);
				
				string = string.substring(is[0] + 2);
				String locStr = extractStringFromAtoB(string,is,"point(",")");
	//			System.out.println("loc: " + locStr);
	
				extractIntArrayOfSize(o.loc, locStr, 2);
				
				string = string.substring(is[0] + 2);
				String speedStr = extractStringFromAtoB(string,is,"[","]");
	//			System.out.println("speed: " + speedStr);
	
				extractFloatArrayOfSize(o.speed, speedStr, 2);
	
				
				string = string.substring(is[0] + 2);
				String rectStr = extractStringFromAtoB(string,is,"(",")");
	//			System.out.println("rect: " + rectStr);
	
				extractIntArrayOfSize(o.rect, rectStr, 4);
	
				string = string.substring(is[0] + 3);
				
				if(rnvsObjCtrlr != null) {
//					if(o.id==4) {
					rnvsObjCtrlr.incomingRunvasObject(o);
//					}
				}
			}
	        
	    	}catch (StringIndexOutOfBoundsException e) {
	    		throw new ParseException("string index out of bounds",0);
	    	}
		}

	private String extractIntArrayOfSize(int[] is, String colorStr, int size) {
		String separator = ", ";
		for(int j=0;j<size-1;j++) {
			String istr = colorStr.substring(0, colorStr.indexOf(separator));
			is[j] = Integer.parseInt(istr);
			colorStr = colorStr.substring(colorStr.indexOf(separator)+separator.length());
		}
		is[size-1] = Integer.parseInt(colorStr);
		return colorStr;
	}

	private String extractFloatArrayOfSize(float[] is, String colorStr,
			int size) {
				String separator = ", ";
				for(int j=0;j<size-1;j++) {
					String istr = colorStr.substring(0, colorStr.indexOf(separator));
					is[j] = Float.parseFloat(istr);
					colorStr = colorStr.substring(colorStr.indexOf(separator)+separator.length());
				}
				is[size-1] = Float.parseFloat(colorStr);
				return colorStr;
			}

	private String extractStringFromColonToComma(String string, int[] is) {
		String colon = ": ";
		String comma = ",";
		return extractStringFromAtoB(string, is, colon, comma);
	}

	private String extractStringFromAtoB(String string, int[] is,
			String A, String B) {
				int start = string.indexOf(A)+A.length();
				int end = string.indexOf(B, start);
				is[0] = end;
				String result = string.substring(start,end);
				return result;
			}

	public IRunvasObjectsController getRnvsObjCtrlr() {
		return rnvsObjCtrlr;
	}

	public void setRnvsObjCtrlr(IRunvasObjectsController rnvsObjCtrlr) {
		this.rnvsObjCtrlr = rnvsObjCtrlr;
	}

	public boolean isServerOn() {
		return serverOn;
	}

	public void setServerOn(boolean serverOn) {
		this.serverOn = serverOn;
	}

}