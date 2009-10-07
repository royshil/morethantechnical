package org.geekcon.runvas.utils;

import java.nio.FloatBuffer;
import java.util.HashSet;
import java.util.Set;

public class Vertex {
	public float x;
	public float y;
	public float z;
	Vector3D normal;
	public float vcolor[] = null;
	public VertexData vertex = new VertexData();
	
	public enum Color {
		WHITE,
		GRAY,
		BLACK
	};
	
	public Color color = Color.WHITE;
	
	public Vector3D getNormal() {
		return normal;
	}

	public void setNormal(Vector3D normal) {
		this.normal = normal;
	}
	
	public Vertex(float x, float y, float z) {
		super();
		this.x = x;
		this.y = y;
		this.z = z;
	}
	
	public Vector3D getVector3D() {
		return new Vector3D(x,y,z);
	}

	public boolean isIsolated() {
		return (vertex.half == null);
	}
	
	public void calcNormal() {
		Edge e = vertex.half.edge;
		Vector3D outVec = null;
		Vector3D thisVec = this.getVector3D();
		if(e.a == this) {
			outVec = e.b.getVector3D().minus(thisVec);
		} else {
			outVec = e.a.getVector3D().minus(thisVec);
		}
		
		Set<Edge> ring = get1RingOfEdges();
		ring.remove(e);
		float min = Float.MAX_VALUE;
		Vertex minVtx = null;
		for (Edge r : ring) {
			Vector3D outVec1 = null;
			Vertex opposite = null;
			if(r.a == this) {
				outVec1 = r.b.getVector3D().minus(thisVec);
				opposite = r.b;
			} else {
				outVec1 = r.a.getVector3D().minus(thisVec);
				opposite = r.a;
			}
			float absInnerP = Math.abs(outVec.innerProduct(outVec1));
			if(absInnerP < min) {
				minVtx = opposite;
				min = absInnerP;
			}
		}
		Vector3D outVec1 = minVtx.getVector3D().minus(thisVec);
		Vector3D newNormal = outVec.crossProduct(outVec1).getNormalized();
		
//		if(normal.innerProduct(newNormal)<0) newNormal.multiply(-1f);
		
		setNormal(normal.projectOnto(newNormal).getNormalized());
	}

	public Set<Edge> get1RingOfEdges() {
		Set<Edge> outSet = null;
		if (vertex != null) {
			outSet = new HashSet<Edge>();
			HalfEdge begin = vertex.half;
			HalfEdge half  = begin;
			do {
				outSet.add(half.edge);
				half = half.pair.next;
			} while (half != begin);
		}
		return outSet;
	}
	
	public Set<Edge> get2RingOfEdges() {
		Set<Edge> ring = get1RingOfEdges();
		Set<Edge> twoRing = new HashSet<Edge>();
		for (Edge edge : ring) {
			twoRing.add(edge);
			twoRing.addAll(edge.a.get1RingOfEdges());
			twoRing.addAll(edge.b.get1RingOfEdges());
		}
		return twoRing;
	}
	
	public FloatBuffer getFB() {
		float[] f = { this.x, this.y, this.z, 1f };
		return FloatBuffer.wrap(f);
	}
	
	@Override
	public String toString() {
		return "[" + x + ", " + y + ", " + z + "]";
	}
}
