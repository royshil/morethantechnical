package org.geekcon.runvas.utils;

import java.nio.FloatBuffer;

import org.geekcon.runvas.GameModel;

public class Vector3D {
	public static final Vector3D origin = new Vector3D(0.0f, 0.0f, 0.0f);
	public static final Vector3D Xaxis = new Vector3D(1.0f, 0.0f, 0.0f);
	public static final Vector3D Yaxis = new Vector3D(0.0f, 1.0f, 0.0f);
	public static final Vector3D Zaxis = new Vector3D(0.0f, 0.0f, 1.0f);
	public static final Vector3D ones = new Vector3D(1.0f, 1.0f, 1.0f);
	public static final Vector3D minusInfinity = new Vector3D(Float.MIN_VALUE,
			Float.MIN_VALUE, Float.MIN_VALUE);
	public static final Vector3D plusInfinity = new Vector3D(Float.MAX_VALUE,
			Float.MAX_VALUE, Float.MAX_VALUE);
	
	public static final float EPSILON = 0.00001f;
	
	public float x;
	public float y;
	public float z;

	public Vector3D(Vector3D v) {
		this.x = v.x;
		this.y = v.y;
		this.z = v.z;
	}

	public Vector3D(float x, float y, float z) {
		super();
		this.x = x;
		this.y = y;
		this.z = z;
	}

	/**
	 * return the origin: (0,0,0)
	 * 
	 * @return
	 */
	public static Vector3D getOrigin() {
		// if(origin == null) origin = new Vector3D(0.0,0.0,0.0);
		return origin;
	}

	/**
	 * Get this vector's norm
	 * 
	 * @return
	 */
	public float getNorm() {
		return (float) Math.sqrt(x * x + y * y + z * z);
	}

	/**
	 * Get a normalized vector
	 * 
	 * @return
	 */
	public Vector3D getNormalized() {
		float n = getNorm();
		return new Vector3D(x / n, y / n, z / n);
	}

	/**
	 * multiply this vector by this scalar
	 * 
	 * @param s
	 * @return
	 */
	public Vector3D multiply(float s) {
		return new Vector3D(x * s, y * s, z * s);
	}

	/**
	 * get the cross product of this vector with v: (this X v)
	 * 
	 * @param v
	 * @return
	 */
	public Vector3D crossProduct(Vector3D v) {
		return new Vector3D(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y
				* v.x);
	}

	/**
	 * get the addition of this vector and v
	 * 
	 * @param v
	 * @return this + v
	 */
	public Vector3D add(Vector3D v) {
		return new Vector3D(x + v.x, y + v.y, z + v.z);
	}

	/**
	 * returns the inner product of this vector and v: (this * v)
	 * 
	 * @param v
	 * @return
	 */
	public float innerProduct(Vector3D v) {
		return this.x * v.x + this.y * v.y + this.z * v.z;
	}

	/**
	 * returns the subtraction of v from this vector
	 * 
	 * @param v
	 * @return this - v
	 */
	public Vector3D minus(Vector3D v) {
		return new Vector3D(this.x - v.x, this.y - v.y, this.z - v.z);
	}

	/**
	 * returns the piecewise multiplication of this vector and v: this .* v =
	 * (this.x * v.x, this.y * v.y, this.z * v.z)
	 * 
	 * @param v
	 * @return
	 */
	public Vector3D piecewiseMultiply(Vector3D v) {
		return new Vector3D(this.x * v.x, this.y * v.y, this.z * v.z);
	}

	/**
	 * returns the inverse vector
	 * 
	 * @return
	 */
	public Vector3D inverseVector() {
		return this.multiply(-1.0f);
	}

	public Vector3D projectOnto(Vector3D v) {
		return v.multiply((float) this.innerProduct(v)
				/ (float) v.innerProduct(v));
	}

	@Override
	public boolean equals(Object obj) {
		if (obj.getClass() != this.getClass())
			return false;
		Vector3D v = (Vector3D) obj;
		float xDiff = this.x - v.x;
		if((xDiff > 0 && xDiff > EPSILON) || (xDiff < 0 && xDiff < EPSILON))
			return false;
		
		float yDiff = this.y - v.y;
		if((yDiff > 0 && (yDiff) > EPSILON) || (yDiff < 0 && yDiff < EPSILON))
			return false;
		
		float zDiff = this.z - v.z;
		if((zDiff > 0 && (zDiff) > EPSILON) || (zDiff < 0 && zDiff < EPSILON))
			return false;
		
		return true;
	}

	public FloatBuffer getFB() {
		float[] f = { this.x, this.y, this.z, 1f };
		return FloatBuffer.wrap(f);
	}

	@Override
	public String toString() {
		return super.toString() + " Vector(" + x + "," + y + "," + z + ")";
	}

	public void setTo(Vector3D v) {
		this.x = v.x;
		this.y = v.y;
		this.z = v.z;
	}

	public static void angleVectorToDirectionVector(Vector3D angles, Vector3D left, Vector3D up, Vector3D forward) {
		// /////////////////////////////////////////////////////////////////////////////
		// convert Euler angles(x,y,z) to axes(left, up, forward)
		// Each column of the rotation matrix represents left, up and forward
		// axis.
		// The order of rotation is Roll->Yaw->Pitch (Rx*Ry*Rz)
		// Rx: rotation about X-axis, pitch
		// Ry: rotation about Y-axis, yaw(heading)
		// Rz: rotation about Z-axis, roll
		// Rx Ry Rz
		// |1 0 0| | Cy 0 Sy| |Cz -Sz 0| | CyCz -CySz Sy |
		// |0 Cx -Sx|*| 0 1 0|*|Sz Cz 0| = | SxSyCz+CxSz -SxSySz+CxCz -SxCy|
		// |0 Sx Cx| |-Sy 0 Cy| | 0 0 1| |-CxSyCz+SxSz CxSySz+SxCz CxCy|
		// /////////////////////////////////////////////////////////////////////////////

		float DEG2RAD = 3.141593f / 180;
		double sx, sy, sz, cx, cy, cz, theta;

		// rotation angle about X-axis (pitch)
		theta = angles.x * DEG2RAD;
		sx = Math.sin(theta);
		cx = Math.cos(theta);

		// rotation angle about Y-axis (yaw)
		theta = angles.y * DEG2RAD;
		sy = Math.sin(theta);
		cy = Math.cos(theta);

		// rotation angle about Z-axis (roll)
		theta = angles.z * DEG2RAD;
		sz = Math.sin(theta);
		cz = Math.cos(theta);

		// determine left axis
		left.x = (float)(cy * cz);
		left.y = (float) (sx * sy * cz + cx * sz);
		left.z = (float) (-cx * sy * cz + sx * sz);

		// determine up axis
		up.x = (float) (-cy * sz);
		up.y = (float) (-sx * sy * sz + cx * cz);
		up.z = (float) (cx * sy * sz + sx * cz);

		// determine forward axis
		forward.x = (float) sy;
		forward.y = (float) (-sx * cy);
		forward.z = (float) (cx * cy);
	}

	public Vector3D projectOnFace(Face face) {
		Vertex a = face.vertices[0];
		Vertex b = face.vertices[1];
		Vertex c = face.vertices[2];
		Vector3D aVec = new Vector3D(a.x, a.y, a.z);
		Vector3D bVec = new Vector3D(b.x, b.y, b.z);
		Vector3D cVec = new Vector3D(c.x, c.y, c.z);
		return projectOnFace(aVec, bVec, cVec);
	}
	
	public Vector3D projectOnFace(Vector3D aVec, Vector3D bVec, Vector3D cVec) {
		Vector3D bMinusA = bVec.minus(aVec).getNormalized();
		Vector3D cMinusA = cVec.minus(aVec).getNormalized();
		float vInnerProductBminusA = this.innerProduct(bMinusA);
		float vInnerProductCminusA = this.innerProduct(cMinusA);
		return (bMinusA.multiply(vInnerProductBminusA).add(cMinusA.multiply(vInnerProductCminusA))).getNormalized();
	}
	
	/**
	 * this function moves a vector towards b vector by scale of diffInSec
	 * we use this to make smooth transitions of the camera from one point to another
	 * 
	 * alg:
	 * a = a + (a - b) * diffInSec * speed
	 * 
	 * @param diffInSec
	 * @param a
	 * @param b
	 */
	public static void moveAVecToBVecByDiff(float diffInSec, Vector3D a, Vector3D b) {
		float speed = GameModel.CAMERA_SPEED;
		moveAtoBByDiffAndSpeed(a, b, diffInSec, speed);
	}

	public static void moveAtoBByDiffAndSpeed(Vector3D a, Vector3D b,
			float diffInSec, float speed) {
		if(!a.equals(b)) {
			Vector3D goToVector = b.minus(a);
			Vector3D tmpA = a.add(goToVector.multiply(diffInSec * speed));
			a.setTo(tmpA); //change A vector in-place
		}
	}
}
