package org.qtproject.example.notification;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.util.Log;
import org.qtproject.qt5.android.bindings.QtActivity;
import org.qtproject.fantandroid.R;
import org.qtproject.qt5.android.QtNative;

public class NotificationClient extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;
    private static NotificationClient m_instance;

    public NotificationClient()
    {
	Log.v("JNot", "PUPPA");
	try {
	    m_instance = this;
	} catch (Throwable t) {
	    t.printStackTrace();
	}
    }	

    public static void notify(String s)
    {
	try {
	    Log.v("JNot", s);
	    if (m_notificationManager == null) {
		m_notificationManager = (NotificationManager)m_instance.getSystemService(Context.NOTIFICATION_SERVICE);
		m_builder = new Notification.Builder(m_instance);
		m_builder.setContentTitle("A message from Qt!");
		m_builder.setSmallIcon(R.drawable.icon);
	    }
	    
	    m_builder.setContentText(s);
	    m_notificationManager.notify(1, m_builder.build());
	} catch (Throwable e) {
	    e.printStackTrace();
	}
    }
}
//
//	    String mNotTitle = "Medicine!";
//	    String mNotText = "Don't forget to take your medicine!";
//
//	    int NOTIFICATION_ID = 1;
//	    notificationManager.notify(NOTIFICATION_ID, mNotification);
//	}
//    //		m_builder = new Notification.Builder(m_instance);
//    //		m_builder.setSmallIcon(R.drawable.icon);
//    //		m_builder.setContentTitle("A message from Qt !");
//    //	    }
//    //	}
//    	catch (Exception e) {
//    	    Log.e("JNot", e.toString());
//    	    e.printStackTrace();
//    	}
//    //
//    //	m_builder.setContentText(s);
//    //	m_notificationManager.notify(1, m_builder.build());
//    }
//}
