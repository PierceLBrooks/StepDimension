
// Author: Pierce Brooks

package com.ssugamejam.stepdimension;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Bundle;
//import android.support.multidex.MultiDex;
//import android.support.multidex.MultiDexApplication;
import android.util.Log;

public class StepDimension extends Application implements Application.ActivityLifecycleCallbacks {
    private static final String TAG = "IDC";

    public StepDimension() {
        super();
        registerActivityLifecycleCallbacks(this);
    }

    /*
    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        MultiDex.install(this);
    }
    */


    @Override
    protected void finalize() throws Throwable {
        unregisterActivityLifecycleCallbacks(this);
        super.finalize();
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }

    @Override
    public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
        Log.d(TAG, "onActivityCreated "+activity.toString());
    }

    @Override
    public void onActivityStarted(Activity activity) {
        Log.d(TAG, "onActivityStarted "+activity.toString());
    }

    @Override
    public void onActivityResumed(Activity activity) {
        Log.d(TAG, "onActivityResumed "+activity.toString());
    }

    @Override
    public void onActivityPaused(Activity activity) {
        Log.d(TAG, "onActivityPaused "+activity.toString());
    }

    @Override
    public void onActivityStopped(Activity activity) {
        Log.d(TAG, "onActivityStopped "+activity.toString());
    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
        Log.d(TAG, "onActivitySaveInstanceState "+activity.toString());
    }

    @Override
    public void onActivityDestroyed(Activity activity) {
        Log.d(TAG, "onActivityDestroyed "+activity.toString());
    }
}
