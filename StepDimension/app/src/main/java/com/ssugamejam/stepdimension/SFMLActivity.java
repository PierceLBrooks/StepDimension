
// Author: Pierce Brooks

package com.ssugamejam.stepdimension;

import android.app.NativeActivity;
import android.content.Context;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;

import java.util.HashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class SFMLActivity extends NativeActivity implements Lock {
    static {
        loadLibraries();
    }

    private static final String TAG = "SFML";
    private static final String VIBRATOR_SERVICE = Context.VIBRATOR_SERVICE;
    private static SFMLActivity self = null;

    private Audio audio;
    private HashMap<Object, Boolean> flags;
    private ReentrantLock lock;

    public SFMLActivity() {
        super();
        audio = null;
        flags = new HashMap<Object, Boolean>();
        lock = new ReentrantLock();
        self = this;
    }

    public static SFMLActivity getSelf() {
        return self;
    }

    @Override
    public void onBackPressed()
    {
        lock();
        try
        {
            flags.put("BACK", true);
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
        }
        finally
        {
            unlock();
        }
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
        pause();
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();

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
        loadLibrary("IDCP", false);
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

    public void play(String song) {
        pause();
        audio = new Audio(getApplicationContext());
        audio.birth(song);
        audio.play();
    }

    public void pause() {
        if (audio == null) {
            return;
        }
        audio.pause();
        audio.death();
        audio = null;
    }

    public boolean checkFlag(Object name)
    {
        boolean flag = false;
        if (!flags.containsKey(name))
        {
            return flag;
        }
        lock();
        try
        {
            flag = flags.get(name).booleanValue();
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
        }
        finally
        {
            unlock();
        }
        flags.put(name, false);
        return flag;
    }

    @Override
    public void lock() {
        lock.lock();
    }

    @Override
    public void lockInterruptibly() throws InterruptedException {
        lock.lockInterruptibly();
    }

    @Override
    public boolean tryLock() {
        return lock.tryLock();
    }

    @Override
    public boolean tryLock(long time, TimeUnit unit) throws InterruptedException {
        return lock.tryLock(time, unit);
    }

    @Override
    public void unlock() {
        lock.unlock();
    }

    @Override
    public Condition newCondition() {
        return lock.newCondition();
    }
}
