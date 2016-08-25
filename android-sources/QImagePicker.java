package com.example.android.tools;

import android.util.Log;
import android.app.Activity;
import android.content.Intent;
import org.qtproject.qt5.android.bindings.QtActivity;

public class QImagePicker {
    public static Intent imagePicker() {
	String msgTag = "QShareUtil";
	try {
	    //Log.v(msgTag, "starting activity");
	    if (QtNative.activity() == null)
		return;
	    Intent i = new Intent(Intent.ACTION_PICK);
	    //sendIntent.setAction(Intent.ACTION_SEND);
	    i.setType("image/*");

	    return Intent.createChooser(i, "Seleziona Logo");
	}
	catch (Exception e) {
	    Log.e(msgTag, e.toString());
	    e.printStackTrace();
	}
    }   

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	if (requestCode == 123456789)  {
	    if (resultCode == RESULT_OK) {
		String path;
		Cursor cursor = getContentResolver().query(data.getData(), null, null, null, null);
		if (cursor == null) {
		    path = data.getData().getPath();
		    Log.d("PIPPO1", path);
		} else {
		    cursor.moveToFirst();
		    int idx = cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA);
		    path = cursor.getString(idx);
		    Log.d("PIPPO2", idx);
		    Log.d("PIPPO3", path);
		    cursor.close();
		}

		MyNative.notifyExtra(path);
	    }
	}
	
	super.onActivityResult(requestCode, resultCode, data);
    }

    public String _getRealPathFromURI(Uri contentURI) {
	String result = "pippo";
	Context text = getApplicationContext();
	
	Cursor cursor = test.getContentResolver().query(contentURI, null, null, null, null);
	if (cursor == null) {
	    result = contentURI.getPath();
	} else {
	    cursor.moveToFirst();
	    int idx = cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA);
	    result = cursor.getString(idx);
	    cursor.close();
	}

	return result;
    }

    public static String getRealPathFromURI(Uri contentURI) {
	QImagePicker me = new QImagePicker();
	String result = me._getRealPathFormURI(contentURI);
	return result;
    }
}

