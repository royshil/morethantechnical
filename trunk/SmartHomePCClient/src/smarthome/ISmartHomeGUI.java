/**
Copyright (c) 2010, Roy S.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package smarthome;

public interface ISmartHomeGUI {
	/**
	 * This station is now live
	 * @param index
	 */
	void stationUp(int index);
	/**
	 * This station is down
	 * @param index
	 */
	void stationDown(int index);
	/**
	 * Log a message to the screen
	 * @param s
	 */
	void logMessage(String s);
	
	/**
	 * get the prefred COM port from the user
	 * @return
	 */
	String getCOMPort();
	/**
	 * get the baud rate from the user
	 * @return
	 */
	int getBaudRate();
	
	/**
	 * set the temperature read of the station
	 * @param index
	 * @param temp
	 */
	void stationTemp(int index, int temp);
	/**
	 * show user that master module is responsive
	 */
	void master_online();
	/**
	 * show an error to the user
	 * @param string
	 * @param line
	 */
	void showError(String title, String description);
}
