
// Author: Pierce Brooks

package com.ssugamejam.stepdimension;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.AsyncTask;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Audio extends AsyncTask<Void, Void, Void> implements Lock, MediaPlayer.OnPreparedListener, MediaPlayer.OnErrorListener, MediaPlayer.OnCompletionListener, MediaPlayer.OnInfoListener {
    private static final String TAG = "AUDIO";

    private boolean isBorn;
    private boolean isPlaying;
    private long duration;
    private long time;
    private MediaPlayer player;
    private AudioManager manager;
    private ReentrantLock lock;
    private Context context;

    public Audio(Context context) {
        super();
        isBorn = true;
        lock = new ReentrantLock();
        death();
        manager = (AudioManager)(context.getSystemService(Context.AUDIO_SERVICE));
        this.context = context;
    }

    public boolean birth(String path) {
        boolean result = true;
        lock();
        try {
            if (isBorn) {
                result = false;
            } else {
                isBorn = true;
                try {
                    AssetFileDescriptor asset = null;
                    asset = context.getAssets().openFd(path);
                    player = new MediaPlayer();
                    player.setOnPreparedListener(this);
                    player.setOnErrorListener(this);
                    player.setOnCompletionListener(this);
                    player.setOnInfoListener(this);
                    player.setAudioStreamType(AudioManager.STREAM_MUSIC);
                    player.setDataSource(asset.getFileDescriptor(), asset.getStartOffset(), asset.getLength());
                    player.prepareAsync();
                } catch (Exception exceptionInner) {
                    exceptionInner.printStackTrace();
                }
            }
        } catch (Exception exceptionOuter) {
            exceptionOuter.printStackTrace();
        } finally {
            unlock();
        }
        if (result) {
            execute();
        }
        return result;
    }

    public boolean death() {
        boolean result = true;
        lock();
        try {
            if (!isBorn) {
                result = false;
            } else {
                isBorn = false;
                isPlaying = false;
                duration = 0;
                time = 0;
                if (player != null) {
                    player.release();
                    player = null;
                }
            }
        } catch (Exception exception) {
            exception.printStackTrace();
        } finally {
            unlock();
        }
        return result;
    }

    public boolean play() {
        boolean result = true;
        lock();
        try {
            if ((!isBorn) || (isPlaying)) {
                result = false;
            } else {
                isPlaying = true;
                player.start();
            }

        } catch (Exception exception) {
            exception.printStackTrace();
        }
        finally {
            unlock();
        }
        return result;
    }

    public boolean pause() {
        boolean result = true;
        lock();
        try {
            if ((!isBorn) || (!isPlaying)) {
                result = false;
            } else {
                isPlaying = false;
                player.pause();
            }

        } catch (Exception exception) {
            exception.printStackTrace();
        }
        finally {
            unlock();
        }
        return result;
    }

    @Override
    protected Void doInBackground(Void... voids) {
        boolean isBorn = false;
        boolean isPlaying = false;
        while (true) {
            lock();
            try {
                isBorn = this.isBorn;
                isPlaying = this.isPlaying;
            } catch (Exception exception) {
                exception.printStackTrace();
            } finally {
                unlock();
            }
            if (isBorn) {
                if (isPlaying) {
                    time = player.getCurrentPosition();
                }
                continue;
            }
            break;
        }
        return null;
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

    @Override
    public void onPrepared(MediaPlayer mp) {
        lock();
        try {
            mp.start();
            duration = mp.getDuration();
        } catch (Exception exception) {
            exception.printStackTrace();
        } finally {
            unlock();
        }
    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        return false;
    }

    @Override
    public void onCompletion(MediaPlayer mp) {

    }

    @Override
    public boolean onInfo(MediaPlayer mp, int what, int extra) {
        return false;
    }
}
