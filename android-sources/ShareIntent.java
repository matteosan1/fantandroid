package com.example.android.tools;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;


public class ShareIntent
    {
        static public void shareText(String title, String subject, 
                                     String content, QtActivity activity)
        {
            Intent share = new Intent(Intent.ACTION_SEND);
            share.setType("text/plain");

            share.putExtra(Intent.EXTRA_SUBJECT, subject);
            share.putExtra(Intent.EXTRA_TEXT, Html.fromHtml(content).toString());
            share.putExtra(Intent.EXTRA_HTML_TEXT, content);
            activity.startActivity(Intent.createChooser(share, title));
        }
    }
