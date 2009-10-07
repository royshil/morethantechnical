package org.geekcon.runvas.utils;

import java.nio.FloatBuffer;

import javax.media.opengl.GL;

public class Face {
	public Vertex[] vertices;
	Vector3D normal;
	int count;
	FaceData half;
	float[] texCoords;
	
	public void setTexCoords(float[] texCoords) {
		this.texCoords = texCoords;
	}

	private Face() {}
	
	public static Face createTriangleFace(Vertex a, Vertex b, Vertex c, Vector3D normal, boolean calcHalfEdge) {
		Face f = new Face();
		f.vertices = new Vertex[3];
		f.count = 3;
		f.vertices[0] = a;
		f.vertices[1] = b;
		f.vertices[2] = c;
		f.normal = normal;
		if (calcHalfEdge == true) {
			HalfEdge.calcHalfEdges(a, b);
			HalfEdge.calcHalfEdges(b, c);
			HalfEdge.calcHalfEdges(c, a);
		}
		return f;
	}
	
	public static Face createQuadFace(Vertex a, Vertex b, Vertex c, Vertex d, Vector3D normal, boolean calcHalfEdge) {
		Face f = new Face();
		f.vertices = new Vertex[4];
		f.count = 4;
		f.vertices[0] = a;
		f.vertices[1] = b;
		f.vertices[2] = c;
		f.vertices[3] = d;
		f.normal = normal;
		if (calcHalfEdge == true) {
			HalfEdge.calcHalfEdges(a, b);
			HalfEdge.calcHalfEdges(b, c);
			HalfEdge.calcHalfEdges(c, d);
			HalfEdge.calcHalfEdges(d, a);
		}
		return f;
	}
	
	public void glDrawWireframe(GL gl) {
		for (int i = 0; i < vertices.length; i++) {
//			gl.glNormal3f(normal.x, normal.y, normal.z);
			gl.glVertex3f(vertices[i].x,vertices[i].y,vertices[i].z);
			int nextI = (i+1)%vertices.length;
			gl.glVertex3f(vertices[nextI].x,vertices[nextI].y,vertices[nextI].z);
		}
	}
	
	public void glDraw(GL gl) {
//		if(this.count == 3) {
//			gl.glBegin(GL.GL_TRIANGLES);
//		} else {
//			gl.glBegin(GL.GL_QUADS);
//		}
		
		for (int i = 0; i < vertices.length; i++) {
			if(texCoords != null)
			{
				gl.glTexCoord2f(texCoords[i*2], texCoords[i*2+1]);
			}
			Vertex v = vertices[i];
			if(v.vcolor != null && v.vcolor.length == 4) {
				gl.glColor4fv(FloatBuffer.wrap(v.vcolor));
			}
			gl.glNormal3f(v.normal.x, v.normal.y, v.normal.z);
			gl.glVertex3f(v.x,v.y,v.z);
		}
//		gl.glEnd();
//		gl.glPushAttrib(GL.GL_CURRENT_BIT);
//		gl.glBegin(GL.GL_LINES);
//		Vector3D locPlusY = this.getCenterPoint();
//		gl.glColor3f(0f, 0f, 1f);//blue
//		gl.glVertex3fv(locPlusY.getFB());
//		Vector3D second = locPlusY.add(vertices[0].normal.multiply(1f/20f));
//		gl.glVertex3fv(second.getFB());
//		gl.glEnd();
//		gl.glPopAttrib();
	}

	public Vector3D projectVectonOntoFace(Vector3D direction) {
		// TODO Auto-generated method stub
		return null;
	}

//	public Face getNextFace(Edge exitEdge) {
//		// TODO Auto-generated method stub
//		return null;
//	}
//
//	public Edge getExitEdge(Vector3D direction) {
//		// TODO Auto-generated method stub
//		return null;
//	}

	public Vector3D getCenterPoint() {
		float x = 0,y = 0,z = 0;
		for (int i = 0; i < count; i++) {
			x += vertices[i].x;
			y += vertices[i].y;
			z += vertices[i].z;
		}
		return new Vector3D(x,y,z).multiply(1f/(float)count);
	}

	public Vector3D getNormal() {
		return normal;
	}

	public void setNormal(Vector3D normal) {
		this.normal = normal;
	}
}
