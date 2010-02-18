package smarthome;

import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.UnsupportedCommOperationException;

public interface ISmartHomeListener {

	public abstract void startListener() throws NoSuchPortException,
			PortInUseException, UnsupportedCommOperationException;

	public abstract void stopListener() throws InterruptedException;

	public abstract boolean isM_shouldCancel();

	public abstract void setM_shouldCancel(boolean mShouldCancel);

	public abstract void setM_doReset(boolean mDoReset);

}