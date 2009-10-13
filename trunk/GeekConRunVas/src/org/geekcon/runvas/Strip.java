package org.geekcon.runvas;

import java.awt.Point;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.media.opengl.GL;
import javax.media.opengl.GLException;

import org.geekcon.runvas.utils.Face;
import org.geekcon.runvas.utils.Utils;
import org.geekcon.runvas.utils.Vector3D;
import org.geekcon.runvas.utils.Vertex;

import com.sun.opengl.util.texture.Texture;
import com.sun.opengl.util.texture.TextureCoords;
import com.sun.opengl.util.texture.TextureIO;

public class Strip extends DefaultController {
	private static final String STRIP_TEXTURE_FILE_NAME = "blue.png";
	private static final String SHADE_FILE_NAME = "shade.png";
	private static final float DECAY_TIME_REDUCE_FACTOR = 1.1f;
	private static final float ANGLE_FACTOR = 15f;
	private static final float DECAY_PER_SECOND_FACTOR = .999999999999999999f;
//	private static final float HALF_DECAY_FACTOR = (1f - ((1f - DECAY_PER_SECOND_FACTOR) / 2f));
//	private static final float SPEED_DOWNSCALE_FACTOR = 1f;
	private static final float INITIAL_SIZE = 40;
	private static final float INITIAL_HEIGHT = 10;
	private static final float SHADE_RECT_SIZE = 20f;
	private static final long STRIP_OLD = 1000;

	List<VertexPair> pairs = new ArrayList<VertexPair>();
	List<Face> shadeFaces = new ArrayList<Face>();
	
	Vector3D baseColor = new Vector3D(1f,0f,0f);
	private Texture m_tex = null;
	private static Texture m_shadeTex = null;
	
	private Point m_lastLoc = null;

	public Strip() {
		m.setLocation(new Vector3D(-1f, -1f, .1f));
		m_lastLoc = new Point(0,0);
		m.setScale(1f/400f);
		
		double d = Math.random() * 5.0;
		if(d<1.0) {
			baseColor = new Vector3D(1f,0f,0f);
		} else if(d>=1.0 && d<2.0) {
			baseColor = new Vector3D(0f,1f,0f);
		} else if(d>=2.0 && d<3.0) {
			baseColor = new Vector3D(1f,1f,0f);
		} else if(d>=3.0 && d<4.0) {
			baseColor = new Vector3D(0f,1f,1f);
		} else {
			baseColor = new Vector3D(0f,0f,1f);
		}
	}
	
	@Override
	public void additionalDraw(GL gl) {
		super.additionalDraw(gl);
//		m_tex.disable();
	}
	
	@Override
	public void additionalDrawingWithModel(GL gl) {
		super.additionalDrawingWithModel(gl);
		if(m_shadeTex == null) {
			try {
				m_shadeTex = TextureIO.newTexture(new FileInputStream(SHADE_FILE_NAME), true, "png");
			    m_shadeTex.setTexParameteri(GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR);
			    m_shadeTex.setTexParameteri(GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR);
			} catch (GLException e) {
				e.printStackTrace();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		m_shadeTex.bind();
		m_shadeTex.enable();
		gl.glBegin(GL.GL_QUADS);
		for (Face f : shadeFaces) {
			f.glDraw(gl);
		}
		gl.glEnd();
		m_shadeTex.disable();
		
//		m_tex.enable();
//		m_tex.bind();
	}
	
	@Override
	public void additionalRotation(GL gl) {
		super.additionalRotation(gl);
		
//		if(m_tex == null) {
//			try {
//				m_tex = TextureIO.newTexture(new FileInputStream(STRIP_TEXTURE_FILE_NAME), true, "png");
//			    m_tex.setTexParameteri(GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR);
//			    m_tex.setTexParameteri(GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR);
//			} catch (GLException e) {
//				e.printStackTrace();
//			} catch (IOException e) {
//				e.printStackTrace();
//			}
//		}
	}
	
	private Vector3D m_lastPv = Vector3D.origin;

	public void addToStrip(RunVasObject o) {
//		System.out.println("O.loc[0] = "+o.loc[0]+",o.loc[1]="+o.loc[1]);
		int x = (int)(((float)o.loc[0] / 640.0f) * 800.0f) - 1100;
		int y = 800 - (int)(((float)o.loc[1] / 480.0f) * 800.0f);
		
//		int dxToLastLoc = x-m_lastLoc.x;
//		int dyToLastLoc = y-m_lastLoc.y;
//		if(dxToLastLoc*dxToLastLoc + dyToLastLoc*dyToLastLoc < 10) {
//			System.out.println("drop " + o.id);
//			return; //throw away close points
//		}
		
//		Vector3D objSpeedV = new Vector3D(o.speed[0], o.speed[1], 0);
//		Vector3D objSpeedV = new Vector3D(0, 0, 0);

		// Vector3D objDirV = objSpeedV.getNormalized();
		// float angle = objDirV.innerProduct(Vector3D.Xaxis);

//		float norm = objSpeedV.getNorm();
//		System.out.println(norm);
		
//		0,0-640,480
//		150,150-

//		float stripSize = INITIAL_SIZE; // + norm;
		float halfStripSize = INITIAL_SIZE/2;

		Vector3D pv = Vector3D.origin;
		if (pairs.size() > 0) {
			VertexPair lastPair = pairs.get(pairs.size() - 1);
			float dx = x - (lastPair.a.x + lastPair.b.x)/2f;
			float dy = y - (lastPair.a.y + lastPair.b.y)/2f;
//			if(Math.abs(dx) < 1f || Math.abs(dy) < 1f) { return;}//drop close updates
			if(dx != 0f && dy != 0f) {
				Vector3D v = new Vector3D(dx, dy, 0).getNormalized();
				if(v.x != Float.NaN && v.y != Float.NaN && v.z != Float.NaN) {
					pv = v.crossProduct(Vector3D.Zaxis).multiply(halfStripSize);
					m_lastPv = pv;
				}
			} else {
				pv = m_lastPv;
			}
//			System.out.println(v + "     " + pv); 
		}

//		Vertex a = new Vertex(x - objSpeedV.y, y - objSpeedV.x, INITIAL_HEIGHT - halfStripSize);
		float angle = pv.getNormalized().innerProduct(m_lastPv.getNormalized());
		Vertex a = new Vertex(x - pv.x, y - pv.y, INITIAL_HEIGHT + angle*ANGLE_FACTOR);
//		a.vcolor = new float[] { o.color[0]/255f,
//				o.color[1]/255f, 
//				o.color[2]/255f, 
//				1f };
		a.vcolor = new float[] { baseColor.x/255f,
				baseColor.y/255f, 
				baseColor.z/255f, 
				1f };
		a.setNormal(Vector3D.Zaxis);
//		Vertex b = new Vertex(x + objSpeedV.y, y + objSpeedV.x, INITIAL_HEIGHT + halfStripSize);
		Vertex b = new Vertex(x + pv.x, y + pv.y, INITIAL_HEIGHT - angle*25f);
		b.vcolor = a.vcolor;
		b.setNormal(Vector3D.Zaxis);
		VertexPair pair = new VertexPair(a, b);
		synchronized (pairs) {
			pairs.add(pair);
		}
		
		m_lastLoc.x = x;
		m_lastLoc.y = y;
		
		VertexPair pm1 = null;
		if (pairs.size() > 1) {
			pm1 = pairs.get(pairs.size() - 2);
		} else {
			pm1 = pair;
		}
		VertexPair p = pair; // pairs.get(pairs.size() - 1);

		float xCen = (pair.a.x + pair.b.x)/2f;
		float yCen = (pair.a.y + pair.b.y)/2f;
//		float xCenBefor = (pm1.a.x + pm1.b.x)/2f;
//		float yCenBefor = (pm1.a.y + pm1.b.y)/2f;
		float xCenBefor = xCen;
		float yCenBefor = yCen;
		float top = yCen-SHADE_RECT_SIZE;
		float bottom = yCenBefor+SHADE_RECT_SIZE;
		float left = xCen-SHADE_RECT_SIZE;
		float right = xCenBefor+SHADE_RECT_SIZE;
		Vertex av = new Vertex(left,top,-100f);
		av.setNormal(Vector3D.Zaxis);
		Vertex bv = new Vertex(left,bottom,-100f);
		bv.setNormal(Vector3D.Zaxis);
		Vertex c = new Vertex(right,bottom,-100f);
		c.setNormal(Vector3D.Zaxis);
		Vertex d = new Vertex(right,top,-100f);
		d.setNormal(Vector3D.Zaxis);
		av.vcolor = bv.vcolor = c.vcolor = d.vcolor = new float[]{1f,1f,0f,1f};
		Face fac = Face.createQuadFace(av, bv, c, d, Vector3D.Zaxis, false);
		if(m_shadeTex != null) {
			TextureCoords tco = m_shadeTex.getImageTexCoords();
		    float texCoo[] = new float[] {tco.top(),tco.right(),tco.top(),tco.left(),tco.bottom(),tco.left(),tco.bottom(),tco.right()};
		    fac.setTexCoords(texCoo);
		}
		synchronized (shadeFaces) {
			shadeFaces.add(fac);
		}

		if (pairs.size() > 1) {
			ArrayList<Face> quadFaces = m.getQuadFaces();
			synchronized (quadFaces) {
				Face fc = Face.createQuadFace(p.a, p.b, pm1.b, pm1.a, Vector3D.Xaxis,false);
				if(m_tex != null) {
					TextureCoords tc = m_tex.getImageTexCoords();
				    float texC[] = new float[] {tc.top(),tc.right(),tc.top(),tc.left(),tc.bottom(),tc.left(),tc.bottom(),tc.right()};
				    fc.setTexCoords(texC);
				}
				quadFaces.add(fc);
			}
		}
	}

	private long stripLife = 0l;
	private boolean stripIsOlder = false;
	private boolean stripIsDone;
	@Override
	public void advance(long timeElapsed) {
		super.advance(timeElapsed);
		if(!stripIsOlder) {
			stripLife += timeElapsed;
		}
		if(stripLife > STRIP_OLD) stripIsOlder = true;
		float timeElapsedInSec = (float) timeElapsed / 1000f;
		float decayTimeFactor = 1f - (timeElapsedInSec/DECAY_TIME_REDUCE_FACTOR);

		if (pairs.size() > 1) {
			float[] fs = pairs.get(0).a.vcolor;
			if (Utils.floatEq(fs[3], 0f)) {
				synchronized (pairs) {
					pairs.remove(0);
				}
				synchronized (shadeFaces) {
					shadeFaces.remove(0);
				}
				ArrayList<Face> quadFaces = m.getQuadFaces();
				synchronized (quadFaces) {
					quadFaces.remove(0);
				}
			}
			for (int i = 0; i < pairs.size(); i++) {
				fs = pairs.get(i).a.vcolor;
				fs[3] *= DECAY_PER_SECOND_FACTOR * decayTimeFactor;
				
				fs = shadeFaces.get(i).vertices[0].vcolor;
				fs[3] *= DECAY_PER_SECOND_FACTOR * decayTimeFactor;
			}
		} else {
			//check how much time has gone past the last update
			//and kill strip if necessary
			if(stripIsOlder) {
				//more than a second of no updates - kill strip
				stripIsDone = true;
//				System.out.println("Strip is done");
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
