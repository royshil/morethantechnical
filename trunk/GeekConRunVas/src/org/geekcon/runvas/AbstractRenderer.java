package org.geekcon.runvas;

import javax.media.opengl.GL;

import org.geekcon.runvas.utils.Face;
import org.geekcon.runvas.utils.Vector3D;

public abstract class AbstractRenderer implements IRenderer {
	private boolean drawWireframe = false;

	private int displayListID = -1;

	// @Override
	public void glDraw2D(GL gl, float width, float height) {
	}

	public void additionalDrawing(GL gl) {
	}

	public void init(GL gl) {
	};

	public void glDraw(GL gl) {
		AbstractModel model = this.getModel();

		gl.glPushMatrix();

		Vector3D location = model.getLocation();
		gl.glTranslatef(location.x, location.y, location.z);

		this.getController().additionalRotation(gl);

		Vector3D globalOrientation = model.getGlobalOrientation();
		gl.glRotatef(globalOrientation.x, 1.0f, 0.0f, 0.0f);
		gl.glRotatef(globalOrientation.y, 0.0f, 1.0f, 0.0f);
		gl.glRotatef(globalOrientation.z, 0.0f, 0.0f, 1.0f);

		float scale = model.getScale();
		gl.glScalef(scale, scale, scale);

		// gl.glEnable(GL.GL_LIGHTING);
		// gl.glDisable(GL.GL_TEXTURE_2D);

		gl.glColor3fv(model.getColor().getFB());

		// gl.glMaterialfv(GL.GL_FRONT_AND_BACK, GL.GL_AMBIENT_AND_DIFFUSE,
		// model.getColor().getFB());

		// float mat_specular[]={1.0f, 1.0f, 1.0f, 1.0f};
		// gl.glMaterialfv(GL.GL_FRONT_AND_BACK, GL.GL_SPECULAR,
		// FloatBuffer.wrap(mat_specular));
		// float mat_emission[]={0f, 0f, 0f, 1.0f};
		// gl.glMaterialfv(GL.GL_FRONT_AND_BACK, GL.GL_EMISSION,
		// FloatBuffer.wrap(mat_emission));

		if (drawWireframe) {
			gl.glDisable(GL.GL_LIGHTING);
			gl.glBegin(GL.GL_LINES);
			for (Face triFace : model.getTriFaces()) {
				triFace.glDrawWireframe(gl);
			}
			for (Face quadFace : model.getQuadFaces()) {
				quadFace.glDrawWireframe(gl);
			}
			gl.glEnd();
			gl.glEnable(GL.GL_LIGHTING);
		} else {
			if (displayListID < 0) {
				displayListID = gl.glGenLists(1);
				gl.glNewList(displayListID, GL.GL_COMPILE_AND_EXECUTE);

				// Draw triangle
				gl.glBegin(GL.GL_TRIANGLES);
				for (Face triFace : model.getTriFaces()) {
					triFace.glDraw(gl);
				}
				gl.glEnd();

				// Draw rectangles
				gl.glBegin(GL.GL_QUADS);
				for (Face quadFace : model.getQuadFaces()) {
					quadFace.glDraw(gl);
				}
				gl.glEnd();
				gl.glEndList();
			} else {
				gl.glCallList(displayListID);
			}
		}

		gl.glPopMatrix();

		additionalDrawing(gl);
		getController().additionalDraw(gl);
	}

	public boolean isDrawWireframe() {
		return drawWireframe;
	}

	public void setDrawWireframe(boolean drawWireframe) {
		this.drawWireframe = drawWireframe;
	}

	public void setResetList() {
		displayListID = -1;
	}
}
