 /* ----------------------------------------------------------------------------------------------------------------
 * Change History :
 * ----------------------------------------------------------------------------------------------------------------
 * ---------
 * Change #1
 * ---------
 * Version:            
 * Defect ID:           
 * CR ID:              
 * Date:               
 * Programmer:         
 * Supervisor:         
 * Changed:			   
 * Description:        
 
 *----------------------------------------------------------------------------------------------------*/
package com.arnonse.silentmode;

import java.util.HashMap;
import java.util.Hashtable;


public interface ServiceUpdateUIListener {
	public void updateUI(HashMap<String, Hashtable<String, String>> data);
}

