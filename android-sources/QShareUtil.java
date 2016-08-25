package com.example.android.tools;

import android.util.Log;
import org.qtproject.qt5.android.QtNative;

static public void share(String text) {
    String msgTag = "PUPPA";
    try {
	Log.v(msgTag, "starting activity");
	Activity mother = QtNative.activity();
	Log.v(msgTag, mother.toString());
	Log.v(msgTag, MainActivity.class.getName());
	Intent intent = new Intent(mother, MainActivity.class);
	mother.startActivity(intent);
    }
    catch (Exception e) {
	Log.e(msgTag, e.toString());
	e.printStackTrace();
    }

//    if (QtNativa.activity() == null)
//	return;
//    Intent sendIntent = new Intent();
//    Log.d("PIPPO", text)
//    sendIntent.setAction(Intent.ACTION_SEND);
//    sendIntent.putExtra(Intent.EXTRA_TEXT, text);
//    sendIntent.setType("text/plain");
//    QtNative.activity().startActivity(sendIntent);
}
