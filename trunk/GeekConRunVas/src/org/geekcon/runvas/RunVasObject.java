package org.geekcon.runvas;

import org.geekcon.runvas.utils.Vector3D;

public class RunVasObject {
	public int id;
	public int size;
	public int color[] = {0,0,0};
	public int loc[] = {0,0};
	public float speed[] = {0f,0f};
	public int rect[] = {0,0,0,0};
	
	@Override
	public String toString() {
		return "RunVasObject: [id="+id+
//				",size="+size+
//				",color="+color[0]+"-"+color[1]+"-"+color[2]+
				",loc="+loc[0]+"-"+loc[1]+"]";
//				",speed="+speed[0]+"-"+speed[1]+
//				",rect="+rect[0]+"-"+rect[1]+"-"+rect[2]+"-"+rect[3]+"]";
	}
}
