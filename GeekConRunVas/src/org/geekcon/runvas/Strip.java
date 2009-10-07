package org.geekcon.runvas;

import java.util.ArrayList;

import org.geekcon.runvas.utils.Face;
import org.geekcon.runvas.utils.Vector3D;
import org.geekcon.runvas.utils.Vertex;

public class Strip extends DefaultController {
	public Strip() {
		ArrayList<Vertex> vs = m.getVertices();
		Vertex a = new Vertex(0,0,0); a.vcolor = new float[]{0,0,1,.5f}; a.setNormal(Vector3D.Xaxis);
		Vertex b = new Vertex(1,0,0); b.vcolor = new float[]{0,0,1,.5f}; b.setNormal(Vector3D.Xaxis);
		Vertex c = new Vertex(1,0,1); b.vcolor = new float[]{0,0,1,.5f}; c.setNormal(Vector3D.Xaxis);
		Vertex d = new Vertex(0,0,1); b.vcolor = new float[]{0,0,1,.5f}; d.setNormal(Vector3D.Xaxis);
		vs.add(a);vs.add(b);vs.add(c);vs.add(d);
		m.getQuadFaces().add(Face.createQuadFace(a, b, c, d, Vector3D.Xaxis, false));
		m.setLocation(new Vector3D(.5f,.5f,.1f));
		m.setScale(.5f);
	}
}
