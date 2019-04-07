
// Author: Pierce Brooks

package com.ssugamejam.stepdimension;

import android.app.NativeActivity;
import android.content.Context;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;

public class SFMLActivity extends NativeActivity {
    static {
        loadLibraries();
    }

    private static final String TAG = "SFML";
    private static final String VIBRATOR_SERVICE = Context.VIBRATOR_SERVICE;

    private Audio audio;

    public SFMLActivity() {
        super();
        audio = null;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        commonOnCreate(savedInstanceState, null);
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState, @Nullable PersistableBundle persistentState) {
        commonOnCreate(savedInstanceState, persistentState);
        super.onCreate(savedInstanceState, persistentState);
    }

    @Override
    protected void onDestroy() {
        audio.pause();
        audio.death();
        audio = null;
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
        audio = new Audio(getApplicationContext());
        audio.birth("sample.flac");
        audio.play();
    }

    @Override
    protected void onPause() {
        super.onPause();
        finish();
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
    }

    private void commonOnCreate(@Nullable Bundle savedInstanceState, @Nullable PersistableBundle persistentState) {
        loadLibraries();
        Log.d(TAG, stringFromJNI());
    }

    private static void loadLibrary(String library) {
        loadLibrary(library, true);
    }

    private static void loadLibrary(String library, boolean variant) {
        Log.d(TAG, "loadLibrary "+library);
        if (variant) {
            if (BuildConfig.DEBUG) {
                System.loadLibrary(library+"-d");
                return;
            }
        }
        System.loadLibrary(library);
    }

    private static void loadLibraries() {
        loadLibrary("openal", false);
        loadLibrary("sfml-system");
        loadLibrary("sfml-window");
        loadLibrary("sfml-audio");
        loadLibrary("sfml-graphics");
        loadLibrary("sfml-network");
        loadLibrary("sfml-activity");
        loadLibrary("native-lib", false);
    }

    @Override
    public String toString() {
        return TAG;
    }

    private native String stringFromJNI();

    @Override
    public Object getSystemService(@NonNull String name) {
        return super.getSystemService(name);
    }
}
