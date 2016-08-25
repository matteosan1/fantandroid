package com.example.android.tools;

import android.app.Activity;
import android.content.Intent;
import org.qtproject.qt5.android.bindings.QtActivity;

public class MyNative {
    public static native void notifyExtra(String path);
}
