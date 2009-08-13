package com.arnonse.pelesms;

import android.app.ListActivity;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.Contacts.People;
import android.view.View;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;


public class ContactList extends ListActivity{
	private ListAdapter mAdapter;
	SharedPreferences prefs;

	/** Called when the activity is first created. */
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        Cursor contactsCursor = this.managedQuery(People.CONTENT_URI,
                     null, "number LIKE '05%'", null, "People.NAME ASC");
        startManagingCursor(contactsCursor);
        
        String[] columnsToMap = new String[] {People.NAME};
        int[] mapTo = new int[] {android.R.id.text1};

        mAdapter = new SimpleCursorAdapter(this,
                     android.R.layout.simple_list_item_1,
                     contactsCursor, columnsToMap, mapTo);
        this.setListAdapter(mAdapter);
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		// TODO Auto-generated method stub
		super.onListItemClick(l, v, position, id);
		Cursor C = (Cursor) mAdapter.getItem(position);
		String name; 
		String phoneNumber; 
		int nameColumn = C.getColumnIndex(People.NAME); 
		int phoneColumn = C.getColumnIndex(People.NUMBER);
		name = C.getString(nameColumn);
		phoneNumber = C.getString(phoneColumn);
		
		prefs.edit().putString("contactnum", phoneNumber).commit();
		
		finish();
	}
}






