package org.geekcon.runvas;

import javax.media.opengl.GL;

public class DefaultController extends AbstractController {
    AbstractModel m = new AbstractModel() {};
    DefaultRenderer r = new DefaultRenderer(m,this);
    
//  @Override
    public void additionalRotation(GL gl) {}

//  @Override
    public AbstractModel getModel() {
            return m;
    }

//  @Override
    public IRenderer getRenderer() {
            return r;
    }

	@Override
	public void additionalDrawingWithModel(GL gl) {
	}
}
