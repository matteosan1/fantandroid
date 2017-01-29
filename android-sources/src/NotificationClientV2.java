package org.qtproject.example.notification;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.Activity;
import android.util.Log;
import org.qtproject.qt5.android.bindings.QtActivity;
import org.qtproject.fantandroid.R;

public class NotificationClientV2
{
    //public static Context activityContext;
    public static Activity m_activity;
    public static void notify(String text, QtActivity activity) {
	//activityContext = this.getApplicationContext();
	m_activity = new Activity();
	NotificationManager notificationManager = (NotificationManager)m_activity.getSystemService(Context.NOTIFICATION_SERVICE);

	String title = "Reminder";
	Notification mNotification = new Notification(R.drawable.icon, title, System.currentTimeMillis());
	String ntitle = "Medicine";
	String nText = "Don't forget";

	Intent myIntent = new Intent(Intent.ACTION_VIEW);
	//PendingIntent StartIntent = PendingIntent.getActivity(getApplicationContext(), 0, myIntent, PendingIntent.FLAG_CANCEL_CURRENT);
	//mNotification.setLatestEventInfo(getApplicationContext(), nTitle, nText, StartIntent);
	int NOTIFICATION_ID = 1;
	notificationManager.notify(NOTIFICATION_ID, mNotification);
    }
};
