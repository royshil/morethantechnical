package org.geekcon.runvas;

public class DefaultRenderer extends AbstractRenderer {
    AbstractModel mo;
    IController co;
    public DefaultRenderer(AbstractModel m, IController c) {
            this.mo = m;
            this.co = c;
    }
//  @Override
    public IController getController() {return co;}
//  @Override
    public AbstractModel getModel() {return mo;}

}
