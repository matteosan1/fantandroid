package lqsa.util;

import android.content.Intent;
import org.qtproject.qt5.android.bindings.QtActivity;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

public class ImageFromActivityResult {
	
	public static byte[] getImage(Intent intent, QtActivity activity) throws IOException {
        InputStream is = activity.getContentResolver().openInputStream(intent.getData());
		ByteArrayOutputStream buffer = new ByteArrayOutputStream();
		int nRead;
		byte[] data = new byte[16384];

		while ((nRead = is.read(data, 0, data.length)) != -1) {
		  buffer.write(data, 0, nRead);
		}
		buffer.flush();
		return buffer.toByteArray();
    }
}
