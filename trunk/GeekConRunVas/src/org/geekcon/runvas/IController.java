package org.geekcon.runvas;

import javax.media.opengl.GL;

public interface IController {

    /**
     * Load a model from the given file
     */
    void loadFromFile(String filename, boolean calcHalfEdge);

    /**
     * Notify this object to perform its movements
     * @param diff the time in msec from the last update
     */
    void advance(long diff);
    
    void additionalRotation(GL gl);
    
    void additionalDraw(GL gl);
    
    AbstractModel getModel();
    IRenderer getRenderer();
    
    boolean shouldBeRemoved();

}
