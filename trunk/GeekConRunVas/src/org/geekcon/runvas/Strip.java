package org.geekcon.runvas;

import java.util.ArrayList;
import java.util.List;

import org.geekcon.runvas.utils.Face;
import org.geekcon.runvas.utils.Vector3D;
import org.geekcon.runvas.utils.Vertex;

public class Strip extends DefaultController {
	private static final float DECAY_PER_SECOND_FACTOR = .999999999999999999f;
	private static final float HALF_DECAY_FACTOR = (1f - ((1f - DECAY_PER_SECOND_FACTOR) / 2f));

	List<VertexPair> pairs = new ArrayList<VertexPair>();
	
	public Strip() {
		RunVasObject o = new RunVasObject();
		float count = 10;
		float halfCount = count/2f;
		for(float i=0;i<count;i+=1f) {
			o.loc[0] = (int)(i);
			o.loc[1] = 0;
//			o.speed[0] = (int)(halfCount - Math.abs(halfCount-i));
			o.speed[0] = (int)(Math.sin(i/count * Math.PI) * count);
			o.speed[1] = 0;
			o.size = (int)(halfCount - Math.abs(halfCount-i));
			addToStrip(o);
			pairs.get(pairs.size()-1).a.vcolor[3] = i/count;
		}		

		m.setLocation(new Vector3D(.5f,.5f,.1f));
		m.setScale(.01f);
	}

	public void addToStrip(RunVasObject o) {
		int x = o.loc[0]; 
		int y = o.loc[1];
		Vector3D objSpeedV = new Vector3D(o.speed[0],o.speed[1],0);
//		Vector3D objDirV = objSpeedV.getNormalized();
//		float angle = objDirV.innerProduct(Vector3D.Xaxis);
		
		float initialHeight = objSpeedV.getNorm();
		
		Vertex a = new Vertex(x,y,initialHeight); 
		a.vcolor = new float[]{0f,0f,1f,1f}; 
		a.setNormal(Vector3D.Xaxis);
		Vertex b = new Vertex(x,y,initialHeight + o.size); 
		b.vcolor = a.vcolor; 
		b.setNormal(Vector3D.Xaxis);
		pairs.add(new VertexPair(a,b));
		
		if(pairs.size() > 1) {
		VertexPair pm1 = pairs.get(pairs.size()-2);
		VertexPair p = pairs.get(pairs.size()-1);
		m.getQuadFaces().add(
			Face.createQuadFace(
					p.a,p.b,pm1.b,pm1.a,
					Vector3D.Xaxis,false)
		);
		}
	}
	
	@Override
	public void advance(long timeElapsed) {
		super.advance(timeElapsed);
		float timeElapsedInSec = 1f - (float)timeElapsed / 1000f;
		
		for (int i = 1; i < pairs.size(); i++) {
			float[] fs = pairs.get(i-1).a.vcolor;
			fs[3] *= DECAY_PER_SECOND_FACTOR * timeElapsedInSec;
			fs = pairs.get(i).a.vcolor;
			fs[3] *= HALF_DECAY_FACTOR * timeElapsedInSec;
		}
	}
	
	class VertexPair {
		public Vertex a,b;

		public VertexPair(Vertex a, Vertex b) {
			super();
			this.a = a;
			this.b = b;
		}
		
	}
}
