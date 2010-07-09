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

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

public class SmartHomeWidget {

	private Shell sShell = null;  //  @jve:decl-index=0:visual-constraint="10,10"
	private Group stations_grp = null;
	public Composite station1_cmpst = null;
	public Canvas canvas = null;
	private Composite composite = null;
	private Button checkBox = null;
	private Composite composite3 = null;
	public Button button_testStation = null;
	public Button button_pollStation = null;
	private Label label = null;
	public Composite station1_cmpst1 = null;
	private Label label1 = null;
	public Canvas canvas1 = null;
	private Composite composite1 = null;
	private Button checkBox1 = null;
	public Composite station1_cmpst2 = null;
	private Label label2 = null;
	public Canvas canvas2 = null;
	private Composite composite2 = null;
	private Button checkBox3 = null;
	private Label label3 = null;
	public Button button_turnON = null;
	public Button button_turnOFF = null;
	public Text textArea = null;
	private Label label4 = null;
	public Button radioButton_station1 = null;
	public Button radioButton_station2 = null;
	public Button radioButton_station3 = null;
	public Group master_group = null;
	private Label label5 = null;
	private Label label7 = null;
	public Text text_baudRate = null;
	public Button button_connect = null;
	public Button button_R = null;
	public Combo combo = null;
	/**
	 * This method initializes sShell
	 * @param display 
	 */
	public void createSShell() {
		GridData gridData8 = new GridData();
		gridData8.horizontalAlignment = GridData.FILL;
		gridData8.horizontalSpan = 2;
		gridData8.verticalSpan = 2;
		gridData8.heightHint = -1;
		gridData8.grabExcessVerticalSpace = true;
		gridData8.verticalAlignment = GridData.FILL;
		GridLayout gridLayout = new GridLayout();
		gridLayout.numColumns = 2;
		sShell = new Shell();
		sShell.setText("Smart Home Client");
		sShell.setLayout(gridLayout);
		createGroup();
		createStations_grp();
		sShell.setSize(new Point(359, 380));
		label4 = new Label(getsShell(), SWT.NONE);
		label4.setText("Log");
		Label filler = new Label(getsShell(), SWT.NONE);
		textArea = new Text(getsShell(), SWT.MULTI | SWT.WRAP | SWT.V_SCROLL);
		textArea.setLayoutData(gridData8);
	}

	public Shell getsShell() {
		return sShell;
	}

	/**
	 * This method initializes stations_grp	
	 *
	 */
	private void createStations_grp() {
		GridData gridData10 = new GridData();
		gridData10.horizontalAlignment = GridData.FILL;
		gridData10.grabExcessHorizontalSpace = true;
		gridData10.horizontalSpan = 2;
		gridData10.verticalAlignment = GridData.CENTER;
		GridLayout gridLayout1 = new GridLayout();
		gridLayout1.numColumns = 4;
		gridLayout1.horizontalSpacing = 6;
		stations_grp = new Group(getsShell(), SWT.NONE);
		stations_grp.setText("Stations");
		createStation1_cmpst();
		stations_grp.setLayout(gridLayout1);
		createStation1_cmpst1();
		createStation1_cmpst2();
		stations_grp.setLayoutData(gridData10);
		createComposite3();
	}

	/**
	 * This method initializes station1_cmpst	
	 *
	 */
	private void createStation1_cmpst() {
		GridData gridData = new GridData();
		gridData.grabExcessVerticalSpace = false;
		gridData.horizontalIndent = 0;
		gridData.heightHint = -1;
		gridData.grabExcessHorizontalSpace = false;
		RowLayout rowLayout = new RowLayout();
		rowLayout.type = org.eclipse.swt.SWT.VERTICAL;
		rowLayout.wrap = true;
		rowLayout.pack = true;
		rowLayout.spacing = 3;
		rowLayout.marginTop = 3;
		rowLayout.justify = false;
		rowLayout.fill = false;
		station1_cmpst = new Composite(stations_grp, SWT.BORDER);
		radioButton_station1 = new Button(station1_cmpst, SWT.RADIO | SWT.TOGGLE);
		radioButton_station1.setText("Station 1");
		label = new Label(station1_cmpst, SWT.NONE);
		label.setText("Temperature");
		createCanvas();
		station1_cmpst.setLayoutData(gridData);
		station1_cmpst.setLayout(rowLayout);
		createComposite();
	}

	/**
	 * This method initializes canvas	
	 *
	 */
	private void createCanvas() {
		canvas = new Canvas(station1_cmpst, SWT.NONE);
		canvas.setData("name", new Integer(0));
	}

	/**
	 * This method initializes composite	
	 *
	 */
	private void createComposite() {
		GridData gridData3 = new GridData();
		gridData3.horizontalAlignment = GridData.BEGINNING;
		gridData3.grabExcessHorizontalSpace = false;
		gridData3.verticalAlignment = GridData.CENTER;
		composite = new Composite(station1_cmpst, SWT.NONE);
		composite.setLayout(new GridLayout());
		checkBox = new Button(composite, SWT.CHECK);
		checkBox.setText("Button");
		checkBox.setLayoutData(gridData3);
	}

	/**
	 * This method initializes composite3	
	 *
	 */
	private void createComposite3() {
		GridData gridData2 = new GridData();
		gridData2.horizontalAlignment = GridData.FILL;
		gridData2.grabExcessHorizontalSpace = true;
		gridData2.verticalAlignment = GridData.BEGINNING;
		GridData gridData7 = new GridData();
		gridData7.horizontalAlignment = GridData.FILL;
		gridData7.verticalAlignment = GridData.CENTER;
		GridData gridData6 = new GridData();
		gridData6.horizontalAlignment = GridData.FILL;
		gridData6.verticalAlignment = GridData.CENTER;
		GridData gridData5 = new GridData();
		gridData5.horizontalAlignment = GridData.FILL;
		gridData5.verticalAlignment = GridData.CENTER;
		GridData gridData4 = new GridData();
		gridData4.horizontalAlignment = GridData.FILL;
		gridData4.verticalAlignment = GridData.CENTER;
		GridData gridData1 = new GridData();
		gridData1.horizontalAlignment = GridData.FILL;
		gridData1.verticalAlignment = GridData.CENTER;
		composite3 = new Composite(stations_grp, SWT.NONE);
		composite3.setLayout(new GridLayout());
		composite3.setLayoutData(gridData2);
		button_testStation = new Button(composite3, SWT.NONE);
		button_testStation.setText("Test Station");
		button_testStation.setLayoutData(gridData4);
		button_pollStation = new Button(composite3, SWT.NONE);
		button_pollStation.setText("Poll Station");
		button_pollStation.setLayoutData(gridData1);
		label3 = new Label(composite3, SWT.NONE);
		label3.setText("");
		label3.setLayoutData(gridData5);
		button_turnON = new Button(composite3, SWT.NONE);
		button_turnON.setText("Turn ON");
		button_turnON.setLayoutData(gridData6);
		button_turnOFF = new Button(composite3, SWT.NONE);
		button_turnOFF.setText("Turn OFF");
		button_turnOFF.setLayoutData(gridData7);
	}

	/**
	 * This method initializes station1_cmpst1	
	 *
	 */
	private void createStation1_cmpst1() {
		RowLayout rowLayout1 = new RowLayout();
		rowLayout1.type = SWT.VERTICAL;
		rowLayout1.justify = false;
		rowLayout1.marginTop = 3;
		rowLayout1.pack = true;
		rowLayout1.spacing = 3;
		rowLayout1.wrap = true;
		rowLayout1.fill = false;
		station1_cmpst1 = new Composite(stations_grp, SWT.BORDER);
		station1_cmpst1.setLayout(rowLayout1);
		radioButton_station2 = new Button(station1_cmpst1, SWT.RADIO| SWT.TOGGLE);
		radioButton_station2.setText("Station 2");
		label1 = new Label(station1_cmpst1, SWT.NONE);
		label1.setText("Temperature");
		createCanvas1();
		createComposite1();
	}

	/**
	 * This method initializes canvas1	
	 *
	 */
	private void createCanvas1() {
		canvas1 = new Canvas(station1_cmpst1, SWT.NONE);
		canvas1.setData("name", new Integer(1));
	}

	/**
	 * This method initializes composite1	
	 *
	 */
	private void createComposite1() {
		composite1 = new Composite(station1_cmpst1, SWT.NONE);
		composite1.setLayout(new GridLayout());
		checkBox1 = new Button(composite1, SWT.CHECK);
		checkBox1.setText("Button");
	}

	/**
	 * This method initializes station1_cmpst2	
	 *
	 */
	private void createStation1_cmpst2() {
		RowLayout rowLayout2 = new RowLayout();
		rowLayout2.type = SWT.VERTICAL;
		rowLayout2.justify = false;
		rowLayout2.marginTop = 3;
		rowLayout2.pack = true;
		rowLayout2.spacing = 3;
		rowLayout2.wrap = true;
		rowLayout2.fill = false;
		station1_cmpst2 = new Composite(stations_grp, SWT.BORDER);
		station1_cmpst2.setLayout(rowLayout2);
		radioButton_station3 = new Button(station1_cmpst2, SWT.RADIO| SWT.TOGGLE);
		radioButton_station3.setText("Station 3");
		label2 = new Label(station1_cmpst2, SWT.NONE);
		label2.setText("Temperature");
		createCanvas2();
		createComposite2();
	}

	/**
	 * This method initializes canvas2	
	 *
	 */
	private void createCanvas2() {
		canvas2 = new Canvas(station1_cmpst2, SWT.NONE);
		canvas2.setData("name", new Integer(2));
	}

	/**
	 * This method initializes composite2	
	 *
	 */
	private void createComposite2() {
		composite2 = new Composite(station1_cmpst2, SWT.NONE);
		composite2.setLayout(new GridLayout());
		checkBox3 = new Button(composite2, SWT.CHECK);
		checkBox3.setText("Button");
	}

	/**
	 * This method initializes group	
	 *
	 */
	private void createGroup() {
		GridData gridData13 = new GridData();
		gridData13.horizontalAlignment = GridData.FILL;
		gridData13.grabExcessHorizontalSpace = true;
		gridData13.widthHint = 56;
		gridData13.verticalAlignment = GridData.CENTER;
		GridData gridData11 = new GridData();
		gridData11.grabExcessHorizontalSpace = true;
		gridData11.verticalAlignment = GridData.CENTER;
		gridData11.horizontalIndent = 1;
		gridData11.horizontalAlignment = GridData.FILL;
		GridLayout gridLayout2 = new GridLayout();
		gridLayout2.numColumns = 6;
		GridData gridData9 = new GridData();
		gridData9.horizontalSpan = 2;
		gridData9.horizontalAlignment = GridData.FILL;
		gridData9.verticalAlignment = GridData.CENTER;
		gridData9.grabExcessHorizontalSpace = true;
		master_group = new Group(getsShell(), SWT.NONE);
		master_group.setText("Master");
		master_group.setLayout(gridLayout2);
		master_group.setLayoutData(gridData9);
		label5 = new Label(master_group, SWT.NONE);
		label5.setText("COM port");
		createCombo();
		label7 = new Label(master_group, SWT.NONE);
		label7.setText("Baud");
		text_baudRate = new Text(master_group, SWT.BORDER);
		text_baudRate.setText("38400");
		text_baudRate.setLayoutData(gridData13);
		button_connect = new Button(master_group, SWT.NONE);
		button_connect.setText("Connect");
		button_connect.setLayoutData(gridData11);
		button_R = new Button(master_group, SWT.NONE);
		button_R.setText("R");
	}

	/**
	 * This method initializes combo	
	 *
	 */
	private void createCombo() {
		combo = new Combo(master_group, SWT.NONE);
	}
}
