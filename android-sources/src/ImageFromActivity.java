package com.examples.android.tools;

import android.content.Intent;
import org.qtproject.qt5.android.bindings.QtActivity;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import android.util.Log;

public class ImageFromActivity {
    public static byte[] getImage(Intent intent, QtActivity activity) throws IOException {
	Log.d("ECCOMI", "PIPPO");
	InputStream is = activity.getContentResolver().openInputStream(intent.getData());
	ByteArrayOutputStream buffer = new ByteArrayOutputStream();
	int nRead;
	byte[] data = new byte[16384];
	while((nRead = is.read(data, 0, data.length)) != -1) {
	    buffer.write(data, 0, nRead);
	}
	buffer.flush();
	return buffer.toByteArray();
    }
}
