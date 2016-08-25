package com.example.android.tools;

import android.util.Log;
import android.app.Activity;
import android.content.Intent;
import org.qtproject.qt5.android.QtNative;

public class QShareUtil {
    public static void share(String subject, String content) {
	String msgTag = "QShareUtil";
	try {
	    //Log.v(msgTag, "starting activity");
	    if (QtNative.activity() == null)
		return;
	    Intent sendIntent = new Intent();
	    sendIntent.setAction(Intent.ACTION_SEND);
	    sendIntent.setType("text/plain");

	    sendIntent.putExtra(Intent.EXTRA_TEXT, content);
	    sendIntent.putExtra(Intent.EXTRA_SUBJECT, subject);
	    // sendIntent.putExtra(Intent.EXTRA_CONTENT, content);
	    
	    QtNative.activity().startActivity(Intent.createChooser(sendIntent, subject));
	}
	catch (Exception e) {
	    Log.e(msgTag, e.toString());
	    e.printStackTrace();
	}
    }   
}
