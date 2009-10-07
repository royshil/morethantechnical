package org.geekcon.runvas.utils;

import java.nio.FloatBuffer;

import javax.media.opengl.GL;

/**
 * Utility functions class
 * @author rshilkr
 *
 */
public class Utils {
	public static final float OneEightyOverPI = 180f/(float)Math.PI;
//	public static void main(String[] args) {
//		Vector3D S1P0 = new Vector3D(Vector3D.origin);
//		Vector3D S1P1 = new Vector3D(1f,0f,0f);
//		
//		Vector3D S2P0 = new Vector3D(0f,0f,1f);
////		Vector3D dir = (new Vector3D(2f,2f,2f)).minus(S2P0).getNormalized();
////		Vector3D dir = (new Vector3D(0f,1f,0f)).minus(S2P0).getNormalized();
//		Vector3D dir = (new Vector3D(.5f,-.1f,0f)).minus(S2P0).getNormalized();
//		Vector3D S2P1 = S2P0.add(dir.multiply(0.2f));
//		
//		Vector3D f = dist3D_Segment_to_Segment(S1P1, S1P0, S2P1, S2P0);
//		System.out.println(f);
//	}
	
	public static void ViewOrtho(GL gl, int x, int y)							// Set Up An Ortho View
	{
		gl.glMatrixMode(GL.GL_PROJECTION);					// Select Projection
		gl.glPushMatrix();							// Push The Matrix
		gl.glLoadIdentity();						// Reset The Matrix
		gl.glOrtho( 0, x , y , 0, -1, 1 );				// Select Ortho Mode
		gl.glMatrixMode(GL.GL_MODELVIEW);					// Select Modelview Matrix
		gl.glPushMatrix();							// Push The Matrix
		gl.glLoadIdentity();						// Reset The Matrix
	}

	public static void ViewPerspective(GL gl)							// Set Up A Perspective View
	{
		gl.glMatrixMode( GL.GL_PROJECTION );					// Select Projection
		gl.glPopMatrix();							// Pop The Matrix
		gl.glMatrixMode( GL.GL_MODELVIEW );					// Select Modelview
		gl.glPopMatrix();							// Pop The Matrix
	}

	public static boolean floatEq(float f1, float f2) {
		float d = f1 - f2;
		return (d <= Vector3D.EPSILON && d >= -Vector3D.EPSILON);
	}
	
	/**
	 * find the distance between 2 line segments
	 * @param S1P1 1st point of segment 
	 * @param S1P0
	 * @param S2P1
	 * @param S2P0
	 * @return
	 */
	public static Vector3D dist3D_Segment_to_Segment( Vector3D S1P1, Vector3D S1P0, Vector3D S2P1, Vector3D S2P0)
	{
	    Vector3D   u = S1P1.minus(S1P0);
	    Vector3D   v = S2P1.minus(S2P0);
	    Vector3D   w = S1P0.minus(S2P0);
	    float    a = u.innerProduct(u);        // always >= 0
	    float    b = u.innerProduct(v);
	    float    c = v.innerProduct(v);        // always >= 0
	    float    d = u.innerProduct(w);
	    float    e = v.innerProduct(w);
	    float    D = a*c - b*b;       // always >= 0
	    float    sc, sN, sD = D;      // sc = sN / sD, default sD = D >= 0
	    /*float    tc, tN, tD = D;*/      // tc = tN / tD, default tD = D >= 0

	    // compute the line parameters of the two closest points
//	    if (D < Vector3D.EPSILON) { // the lines are almost parallel
//	        sN = 0f;        // force using point P0 on segment S1
//	        sD = 1f;        // to prevent possible division by 0.0 later
//	        tN = e;
//	        tD = c;
//	    }
//	    else {                // get the closest points on the infinite lines
	        sN = (b*e - c*d);
//	        tN = (a*e - b*d);
	        if (sN < 0f) {       // sc < 0 => the s=0 edge is visible
//	            sN = 0f;
//	            tN = e;
//	            tD = c;
	            return null;
	        }
	        else if (sN > sD) {  // sc > 1 => the s=1 edge is visible
//	            sN = sD;
//	            tN = e + b;
//	            tD = c;
	            return null;
	        }
//	    }

//	    if (tN < 0f) {           // tc < 0 => the t=0 edge is visible
//	        tN = 0f;
//	        // recompute sc for this edge
//	        if (-d < 0f)
//	            sN = 0f;
//	        else if (-d > a)
//	            sN = sD;
//	        else {
//	            sN = -d;
//	            sD = a;
//	        }
//	    }
//	    else if (tN > tD) {      // tc > 1 => the t=1 edge is visible
//	        tN = tD;
//	        // recompute sc for this edge
//	        if ((-d + b) < 0.0)
//	            sN = 0;
//	        else if ((-d + b) > a)
//	            sN = sD;
//	        else {
//	            sN = (-d + b);
//	            sD = a;
//	        }
//	    }
	    // finally do the division to get sc and tc
	    sc = (Math.abs(sN) < Vector3D.EPSILON ? 0f : sN / sD);
//	    tc = (Math.abs(tN) < Vector3D.EPSILON ? 0f : tN / tD);

	    // get the difference of the two closest points
//	    Vector3D   dP = S2P0.add(v.multiply(tc));  // = S1(sc) - S2(tc)//.add(u.multiply(sc));//
//	    if(Utils.floatEq(sc, 0f)) { return null;}

	    return S1P0.add(u.multiply(sc));//.getNorm();   // return the closest distance
//	    return S2P0.add(v.multiply(tc));
	}

	/**
	 * multLookAt -- Create a matrix to make an object, such as
	 * a camera, "look at" another object or location, from
	 * a specified position.
	 * 
	 * Parameters:
	 * eye[x|y|z] Desired location of the camera object
	 * at[x|y|z]  Location for the camera to look at
	 * up[x|y|z]  Up vector of the camera
	 * 
	 * Algorithm:
	 * The desired transformation is obtained with this 4x4 matrix:
	 * 		|  [xaxis] 0  |
	 *      |    [up]  0  |
	 *      |   [-at]  0  |
	 *      |   [eye]  1  |
	 * Where 'xaxis', 'up' and 'at' are the new X, Y, and Z axes of
	 * the transforned object, respectively, and 'eye' is the input
	 * new location of the transformed object.
	 * 
	 * Assumptions:
	 *  The camera geometry is defined to be facing
	 *  the negative Z axis.
	 *  
	 * Usage:
	 *  multLookAt creates a matrix and multiplies it onto the
	 *  current matrix stack. Typical usage would be as follows:
	 *  
	 *  glMatrixMode (GL_MODELVIEW);
	 *  // Define the usual view transformation here using
	 *  //   gluLookAt or whatever.
	 *  glPushMatrix();
	 *  multLookAt (orig[0], orig[1], orig[2],
	 *          at[0], at[1], at[2],
	 *          up[0], up[1], up[2]);
	 *  // Define "camera" object geometry here
	 *  glPopMatrix();
	 *  
	 *  Warning: Results become undefined as (at-eye) approaches
	 *  coincidence with (up).
	 *  
	 *  from: http://www.opengl.org/resources/faq/technical/lookat.cpp
	 */
	public static void multLookAt (/*float eyex, float eyey, float eyez,
            float atx, float aty, float atz,
            float upx, float upy, float upz,*/
			Vector3D origin,
			Vector3D forward,
			Vector3D up,
            GL gl)
	{
		float m[] = new float[16];
//		Float xaxis = m[0], up = m[4], at = m[8];
		
		// Compute our new look at vector, which will be
		//   the new negative Z axis of our transformed object.
		forward = forward.getNormalized();
		
		// Make a useable copy of the current up vector.
//		up[0] = upx; up[1] = upy; up[2] = upz;
		
		// Cross product of the new look at vector and the current
		//   up vector will produce a vector which is the new
		//   positive X axis of our transformed object.
		Vector3D xaxis = forward.crossProduct(up).getNormalized();
		m[0] = xaxis.x;
		m[1] = xaxis.y;
		m[2] = xaxis.z;
//		cross (xaxis, at, up);
//		normalize (xaxis);
		
		// Calculate the new up vector, which will be the
		//   positive Y axis of our transformed object. Note
		//   that it will lie in the same plane as the new
		//   look at vector and the old up vector.
		up = xaxis.crossProduct(forward);
		m[4] = up.x;
		m[5] = up.y;
		m[6] = up.z;
//		cross (up, xaxis, at);
		
		// Account for the fact that the geometry will be defined to
		//   point along the negative Z axis.
//		scale (at, -1.f);
		forward = forward.multiply(-1f);
		m[8] = forward.x; 
		m[9] = forward.y; 
		m[10] = forward.z;
		
		// Fill out the rest of the 4x4 matrix
		m[3] = 0.f;     // xaxis is m[0..2]
		m[7] = 0.f;     // up is m[4..6]
		m[11] = 0.f;    // -at is m[8..10]
		m[12] = origin.x; 
		m[13] = origin.y; 
		m[14] = origin.z;
		m[15] = 1.f;
		
		// Multiply onto current matrix stack.
		gl.glMultMatrixf(FloatBuffer.wrap(m));
	}
	
	public static void setupLight(GL gl, float[] a, float[] d, float[] s,
			float[] p, int lightNum) {
		gl.glLightfv(lightNum, GL.GL_AMBIENT, FloatBuffer.wrap(a));
		gl.glLightfv(lightNum, GL.GL_DIFFUSE, FloatBuffer.wrap(d));
		gl.glLightfv(lightNum, GL.GL_SPECULAR, FloatBuffer.wrap(s));
		gl.glLightfv(lightNum, GL.GL_POSITION, FloatBuffer.wrap(p));
		gl.glEnable(lightNum);
	}

	public static void tintRenderingView(GL gl, float width, float height) {
		gl.glEnable(GL.GL_BLEND);
	    gl.glBlendFunc(GL.GL_SRC_ALPHA,
				GL.GL_ONE_MINUS_SRC_ALPHA);

		gl.glShadeModel(GL.GL_FLAT);
	    gl.glBegin(GL.GL_QUADS);
	    gl.glColor4f(.1f, .1f, .1f,.5f);
	    gl.glVertex2f(0f,0f);
	    gl.glVertex2f(width,0f);
	    gl.glVertex2f(width,height);
	    gl.glVertex2f(0f,height);
	    gl.glEnd();
		
		gl.glDisable(GL.GL_BLEND);
	}
	
//	public static void fixObjLocationToEdge(DynamicObjectModel m, Edge e) {
//		//avreage location of A & B
//		Vector3D middleOfEdge = e.a.getVector3D().add(e.b.getVector3D()).multiply(.5f);
//		m.setLocation(m.getTransformToMeshLocation().transform(middleOfEdge));
//		
//		//avreage normals of A & B
//		m.setDirectionUp(e.a.getNormal().add(e.b.getNormal()).multiply(.5f));
//		
//		m.setLocationInMeshCoords(middleOfEdge);
//	}
	
//	public static void makeADentInTheMesh(Vertex vtx, float amount, IWorldMeshHandler worldMeshHandler) {
//		//Make a dent in the mesh...
//		Set<Edge> es = vtx.get2RingOfEdges();
//		for (Edge ed : es) {
//			Vector3D v = new Vector3D(ed.a.x,ed.a.y,ed.a.z);
//			v = v.minus(ed.a.getNormal().multiply(amount));
//			ed.a.x = v.x; ed.a.y = v.y; ed.a.z = v.z;
//			v = new Vector3D(ed.b.x,ed.b.y,ed.b.z);
//			v = v.minus(ed.b.getNormal().multiply(amount));
//			ed.b.x = v.x; ed.b.y = v.y; ed.b.z = v.z;
//			
//			if(ed.playerOnEdge != null) {
//				ed.playerOnEdge.getModel().fixLocation();
//			}
//			if(ed.packageOnEdge != null) {
//				Utils.fixObjLocationToEdge(ed.packageOnEdge.getModel(), ed);
//			}
//			if(ed.treeOnEdge != null) {
//				Utils.fixObjLocationToEdge(ed.treeOnEdge.getModel(), ed);
//			}
//		}
//		
//		//Recalculate normals
//		for (Edge ed : es) {
//			ed.a.calcNormal();
//			ed.b.calcNormal();
//		}
//		
//		//reset the display list of the world mesh
//		worldMeshHandler.getWorld().getRenderer().setResetList();
//	}
}
