package org.geekcon.runvas;

import java.util.ArrayList;
import java.util.List;

import org.geekcon.runvas.utils.Face;
import org.geekcon.runvas.utils.Utils;
import org.geekcon.runvas.utils.Vector3D;
import org.geekcon.runvas.utils.Vertex;

public class Strip extends DefaultController {
	private static final float DECAY_PER_SECOND_FACTOR = .999999999999999999f;
	private static final float HALF_DECAY_FACTOR = (1f - ((1f - DECAY_PER_SECOND_FACTOR) / 2f));

	List<VertexPair> pairs = new ArrayList<VertexPair>();
	
	Vector3D baseColor = new Vector3D(Vector3D.ones);

	public Strip() {
//		RunVasObject o = new RunVasObject();
//		float count = 50;
//		float halfCount = count / 2f;
//		double radius = Math.random() * 100.0;
//		int startX = (int)(Math.random() * 100.0);
//		int startY = (int)(Math.random() * 100.0);
//		baseColor.x = (float)Math.random();
//		baseColor.y = (float)Math.random();
//		baseColor.z = (float)Math.random();
//		for (float i = 0; i < count; i += 1f) {
//			float rel = i / count;
//			o.loc[0] = startX + (int)(Math.cos(rel) * radius);
//			o.loc[1] = startY + (int)(Math.sin(rel) * radius);
//			// o.speed[0] = (int)(halfCount - Math.abs(halfCount-i));
//			o.speed[0] = (int) (Math.sin(rel * Math.PI) * 10.0);
//			o.speed[1] = 0;
//			o.size = (int) (halfCount - Math.abs(halfCount - i));
//			addToStrip(o);
//			pairs.get(pairs.size() - 1).a.vcolor[3] = rel;
//		}

		m.setLocation(new Vector3D(-1f, -1f, .1f));
		m.setScale(.01f);
	}

	public void addToStrip(RunVasObject o) {
		int x = o.loc[0];
		int y = o.loc[1];
		Vector3D objSpeedV = new Vector3D(o.speed[0], o.speed[1], 0);
		// Vector3D objDirV = objSpeedV.getNormalized();
		// float angle = objDirV.innerProduct(Vector3D.Xaxis);

		float initialHeight = objSpeedV.getNorm();

		Vertex a = new Vertex(x, y, initialHeight);
		a.vcolor = new float[] { baseColor.x, baseColor.y, baseColor.z, 1f };
		a.setNormal(Vector3D.Xaxis);
		Vertex b = new Vertex(x, y, initialHeight + o.size);
		b.vcolor = a.vcolor;
		b.setNormal(Vector3D.Xaxis);
		pairs.add(new VertexPair(a, b));

		if (pairs.size() > 1) {
			VertexPair pm1 = pairs.get(pairs.size() - 2);
			VertexPair p = pairs.get(pairs.size() - 1);
			m.getQuadFaces().add(
					Face.createQuadFace(p.a, p.b, pm1.b, pm1.a, Vector3D.Xaxis,
							false));
		}
	}

	private long stripLife = 0l;
	private boolean stripIsDone;
	@Override
	public void advance(long timeElapsed) {
		super.advance(timeElapsed);
		stripLife += timeElapsed;
		float timeElapsedInSec = (float) timeElapsed / 1000f;
		float decayTimeFactor = 1f - timeElapsedInSec;

		if (pairs.size() > 1) {
			float[] fs = pairs.get(0).a.vcolor;
			if (Utils.floatEq(fs[3], 0f)) {
				pairs.remove(0);
				m.getQuadFaces().remove(0);
			}
			for (int i = 0; i < pairs.size(); i++) {
				fs = pairs.get(i).a.vcolor;
				fs[3] *= DECAY_PER_SECOND_FACTOR * decayTimeFactor;
			}
		} else {
			//check how much time has gone past the last update
			//and kill strip if necessary
			if(stripLife > 1000l) {
				//more than a second of no updates - kill strip
				stripIsDone = true;
			}
		}
	}
	
	@Override
	public boolean shouldBeRemoved() {
		return super.shouldBeRemoved() || stripIsDone;
	}

	class VertexPair {
		public Vertex a, b;

		public VertexPair(Vertex a, Vertex b) {
			super();
			this.a = a;
			this.b = b;
		}

	}
}
