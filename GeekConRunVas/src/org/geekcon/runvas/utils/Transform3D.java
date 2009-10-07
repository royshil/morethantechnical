package org.geekcon.runvas.utils;

import java.util.Arrays;

public class Transform3D {
	public float[][] m;
	
	public Transform3D() {
		m = new float[4][];
		for (int i = 0; i < m.length; i++) {
			m[i] = new float[4];
			Arrays.fill(m[i], 0f);
			m[i][i] = 1; //set identity
		}
	}
	
	public Vector3D transform(Vector3D v) {
		Vector3D v1 = new Vector3D(Vector3D.origin);
		float f[] = {v.x,v.y,v.z,1f};
		v1.x = f[0]*m[0][0] + f[1]*m[0][1] + f[2]*m[0][2] + f[3]*m[0][3];
		v1.y = f[0]*m[1][0] + f[1]*m[1][1] + f[2]*m[1][2] + f[3]*m[1][3];
		v1.z = f[0]*m[2][0] + f[1]*m[2][1] + f[2]*m[2][2] + f[3]*m[2][3];
		return v1;
	}
	
	private void mul(float[][] m1) {
		Transform3D t = new Transform3D();
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				t.m[i][j] = m[i][0]*m1[0][j] + m[i][1]*m1[1][j] + m[i][2]*m1[2][j] + m[i][3]*m1[3][j];
			}
		}
		m = t.m;
	}
	
	public void translate(float x, float y, float z) {
		float[][] m1 = {{1f,0f,0f,x},
						{0f,1f,0f,y},
						{0f,0f,1f,z},
						{0f,0f,0f,1f}};
		mul(m1);
	}
	
	public void rotateX(float a) {
		float co = (float)Math.cos(a);
		float si = (float)Math.sin(a);
		float[][] m1 = {{1f,0f,0f,0f},
						{0f,co,-si,0f},
						{0f,si,co,0f},
						{0f,0f,0f,1f}};
		mul(m1);
	}
	public void rotateY(float a) {
		float co = (float)Math.cos(a);
		float si = (float)Math.sin(a);
		float[][] m1 = {{co,0f,si,0f},
						{0f,1f,0f,0f},
						{-si,0f,co,0f},
						{0f,0f,0f,1f}};
		mul(m1);
	}
	public void rotateZ(float a) {
		float co = (float)Math.cos(a);
		float si = (float)Math.sin(a);
		float[][] m1 = {{co,-si,0f,0f},
						{si,co,0f,0f},
						{0f,0f,1f,0f},
						{0f,0f,0f,1f}};
		mul(m1);
	}

	public void scale(float f) {
		float[][] m1 = {{f,0f,0f,0f},
						{0f,f,0f,0f},
						{0f,0f,f,0f},
						{0f,0f,0f,1f}};
		mul(m1);
	}
}
