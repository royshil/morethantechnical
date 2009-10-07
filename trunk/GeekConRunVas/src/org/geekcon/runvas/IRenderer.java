package org.geekcon.runvas;

import javax.media.opengl.GL;

public interface IRenderer {
	 /**
     * draw this objects model using GL functions
     * @param gl 
     */
    void glDraw(GL gl);

    void additionalDrawing(GL gl);
    
    AbstractModel getModel();
    IController getController();

    void glDraw2D(GL gl, float width, float height);
    
    void setDrawWireframe(boolean drawWireframe);
    
    void setResetList();
    
    void init(GL gl);

}
