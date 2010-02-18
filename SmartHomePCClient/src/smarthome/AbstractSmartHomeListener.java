package smarthome;

public abstract class AbstractSmartHomeListener {
	protected ISmartHomeGUI m_gui;
	
	public AbstractSmartHomeListener(ISmartHomeGUI g) {
		m_gui = g;
	}
}
