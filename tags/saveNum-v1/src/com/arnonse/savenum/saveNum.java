package com.arnonse.savenum;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.ClipboardManager;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class saveNum extends Activity {
	/** Called when the activity is first created. */

	EditText phnNum;
	Button btnAdd;
	Button btnDial;
	private boolean monitorChange = true;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		setTheme(android.R.style.Theme_Dialog);
		ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		phnNum = (EditText) findViewById(R.id.phnNum);
		phnNum.setText(clipboard.getText());

		btnAdd = (Button) findViewById(R.id.btnAdd);
		btnDial = (Button) findViewById(R.id.btnDial);

		if (clipboard.getText().toString().equals("")) {
			btnAdd.setText(getString(R.string.btnSetText));
		}

		phnNum.addTextChangedListener(new TextWatcher() {

			public void afterTextChanged(Editable arg0) {
				// TODO Auto-generated method stub

			}

			public void beforeTextChanged(CharSequence arg0, int arg1,
					int arg2, int arg3) {
				// TODO Auto-generated method stub

			}

			public void onTextChanged(CharSequence arg0, int arg1, int arg2,
					int arg3) {
				if (monitorChange) {
					monitorChange = false;
					btnAdd.setText(getString(R.string.btnSetText));
				}

			}

		});

		btnAdd.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (btnAdd.getText().equals(getString(R.string.btnClear))) {
					ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
					clipboard.setText("");
					phnNum.setText("");
					btnAdd.setText(getString(R.string.btnSetText));
				} else if (btnAdd.getText().equals(
						getString(R.string.btnSetText))) {
					ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
					clipboard.setText(phnNum.getText());
					showToast(getString(R.string.copied));
					finish();
				}
			}

		});

		btnDial.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				performDial();
			}

		});
	}

	public void performDial() {
		if (!phnNum.getText().toString().equals(""))
			startActivity(new Intent(Intent.ACTION_CALL, Uri.parse("tel:" + phnNum.getText())));
		else showToast("Nothing to dial");
	}

	public void showToast(String s) {
		Toast.makeText(getBaseContext(), s, Toast.LENGTH_SHORT).show();
	}

}
