package org.geekcon.runvas.utils;

public class Edge {
	public Vertex a;
	public Vertex b;

	public Edge(Vertex vertexFrom, Vertex vertexTo) {
		a = vertexFrom;
		b = vertexTo;
	}

	/**
	 * get the distance of C from this edge, with the given direction
	 * assume direction is normalized
	 * @param C
	 * @param direction
	 * @return
	 */
	public float distanceFromEdge(Vector3D C, Vector3D direction) {
		Vector3D AC = a.getVector3D().minus(C);
		Vector3D AB = a.getVector3D().minus(C);
		float ABnorm = AB.getNorm();
		float r = AC.innerProduct(AB) / (ABnorm * ABnorm);
		float Px = a.x + r*(b.x - a.x);
		float Py = a.y + r*(b.y - a.y);
		float Pz = a.z + r*(b.z - a.z);
		Vector3D P = new Vector3D(Px,Py,Pz);
		Vector3D CP = P.minus(C);
		Vector3D CPnormalized = CP.getNormalized();
		float cosAlpha = CPnormalized.innerProduct(direction);
		return CP.getNorm() / cosAlpha;
	}

	public boolean intersectsRay(Vector3D location, Vector3D directionForeward) {
		Vector3D locationAndSome = location.add(directionForeward.multiply(.5f));
		Vector3D intersectPt = Utils.dist3D_Segment_to_Segment(a.getVector3D(), b.getVector3D(), location, locationAndSome);
		return (intersectPt != null);
	}

	public Vector3D findIntersectionPt(Vector3D location,Vector3D directionForeward) {
		Vector3D locationAndSome = location.add(directionForeward.multiply(.005f));
		return Utils.dist3D_Segment_to_Segment(a.getVector3D(), b.getVector3D(), location, locationAndSome);
	}

	public Vector3D getNormal() {
		return ((a.getNormal().add(b.getNormal())).multiply(0.5f)).getNormalized();
	}
}
