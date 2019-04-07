
// Author: Pierce Brooks

package com.ssugamejam.stepdimension;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMetadataRetriever;
import android.os.AsyncTask;
import android.support.annotation.NonNull;
import android.util.Log;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Audio extends AsyncTask<Void, Void, Void> implements Lock {
    public static enum Status {
        BUFFER_NULL,
        BUFFER_FULL,
        EXTRACTOR_DONE,
        INCORRECT_TRACK,
        NORMAL
    }
    private class Track extends MediaCodec.Callback implements Lock {
        private static final String TAG = "TRACK";
        private static final boolean VERBOSE = true;
        private static final int SIZING = 2;

        private boolean status;
        private int index;
        private int session;
        private int counter;
        private int read;
        private long time;
        private ByteBuffer buffer;
        private AudioTrack track;
        private MediaCodec decoder;
        private MediaFormat output;
        private MediaFormat input;
        private AudioManager manager;
        private ReentrantLock lock;

        public Track(int index, AudioManager manager, MediaFormat format) {
            super();
            this.index = index;
            this.decoder = null;
            this.track = null;
            this.buffer = null;
            death();
            this.output = null;
            this.input = format;
            this.manager = manager;
            this.lock = new ReentrantLock();
        }

        public void birth() {
            track = getTrack();
            decoder = getDecoder();
            decoder.configure(input, null, null, 0);
            decoder.start();
        }

        public void death() {
            status = false;
            session = -1;
            counter = 0;
            read = 0;
            time = 0;
            if (track != null) {
                track.release();
                track = null;
            }
            if (decoder != null) {
                decoder.stop();
                decoder.release();
                decoder = null;
            }
        }

        public boolean play() {
            if (track == null) {
                return false;
            }
            track.play();
            return true;
        }

        public boolean pause() {
            if (track == null) {
                return false;
            }
            track.pause();
            return true;
        }

        public Status write(MediaExtractor extractor) {
            Status result = null;
            if (index != extractor.getSampleTrackIndex()) {
                result = Status.INCORRECT_TRACK;
            } else {
                lock();
                try {
                    if (buffer == null) {
                        result = Status.BUFFER_NULL;
                    } else {
                        read = extractor.readSampleData(buffer, 0);
                        time = extractor.getSampleTime();
                        status = !extractor.advance();
                        result = Status.NORMAL;
                    }
                } catch (Exception exception) {
                    exception.printStackTrace();
                } finally {
                    unlock();
                }
            }
            return result;
        }

        @Override
        public void onError(@NonNull MediaCodec codec, @NonNull MediaCodec.CodecException exception) {
            if (VERBOSE) {
                Log.d(TAG, "audio decoder: error: " + exception.getMessage());
            }
        }

        @Override
        public void onOutputFormatChanged(@NonNull MediaCodec codec, @NonNull MediaFormat format) {
            output = format;
            if (VERBOSE) {
                Log.d(TAG, "audio decoder: output format changed: " + output);
            }
        }

        public void onInputBufferAvailable(@NonNull MediaCodec codec, int index) {
            boolean check = false;
            ByteBuffer decoderInputBuffer = codec.getInputBuffer(index);
            buffer = decoderInputBuffer;
            lock();
            try {
                check = status;
            } catch (Exception exception) {
                exception.printStackTrace();
            } finally {
                unlock();
            }
            while (!check) {
                int size = -1;
                long presentationTime = 0;
                lock();
                try {
                    size = read;
                    presentationTime = time;
                } catch (Exception exception) {
                    exception.printStackTrace();
                } finally {
                    unlock();
                }
                if (VERBOSE) {
                    Log.d(TAG, "audio extractor: returned buffer of size " + size);
                    Log.d(TAG, "audio extractor: returned buffer for time " + presentationTime);
                }
                if (size >= 0) {
                    codec.queueInputBuffer(index, 0, size, presentationTime, extractor.getSampleFlags());
                }
                if (status) {
                    if (VERBOSE) {
                        Log.d(TAG, "audio extractor: EOS");
                    }
                    codec.queueInputBuffer(index, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                }
                ++counter;
                if (size >= 0) {
                    break;
                }
                lock();
                try {
                    check = status;
                } catch (Exception exception) {
                    exception.printStackTrace();
                } finally {
                    unlock();
                }
            }
            lock();
            try {
                read = 0;
                time = 0;
                buffer = null;
            } catch (Exception exception) {
                exception.printStackTrace();
            } finally {
                unlock();
            }
        }

        public void onOutputBufferAvailable(@NonNull MediaCodec codec, int index, @NonNull MediaCodec.BufferInfo info) {
            if (VERBOSE) {
                Log.d(TAG, "audio decoder: returned output buffer: " + index);
            }
            if (VERBOSE) {
                Log.d(TAG, "audio decoder: returned buffer of size " + info.size);
            }
            ByteBuffer decoderOutputBuffer = codec.getOutputBuffer(index);
            if ((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                if (VERBOSE) {
                    Log.d(TAG, "audio decoder: codec config buffer");
                }
                codec.releaseOutputBuffer(index, false);
                return;
            }
            if (VERBOSE) {
                Log.d(TAG, "audio decoder: returned buffer for time " + info.presentationTimeUs);
            }
            ++counter;
            if (decoderOutputBuffer != null) {
                byte[] bytes = new byte[decoderOutputBuffer.remaining()];
                decoderOutputBuffer.get(bytes);
                track.write(bytes, 0, info.size);
            }
        }

        private AudioTrack getTrack() {
            int channel = getFormatInteger(input, MediaFormat.KEY_CHANNEL_MASK, AudioFormat.CHANNEL_OUT_STEREO);
            //int encoding = getFormatInteger(input, MediaFormat.KEY_PCM_ENCODING, AudioFormat.ENCODING_PCM_16BIT);
            int encoding = AudioFormat.ENCODING_PCM_16BIT;
            int sampleRate = getFormatInteger(input, MediaFormat.KEY_SAMPLE_RATE, 44100);
            AudioFormat.Builder format = new AudioFormat.Builder();
            AudioTrack.Builder track = new AudioTrack.Builder();
            buffer = ByteBuffer.allocate(AudioTrack.getMinBufferSize(sampleRate, channel, encoding)*SIZING);
            session = manager.generateAudioSessionId();
            format = format.setChannelMask(channel);
            format = format.setEncoding(encoding);
            format = format.setSampleRate(sampleRate);
            track = track.setBufferSizeInBytes(buffer.capacity()/SIZING);
            track = track.setSessionId(session);
            track = track.setAudioFormat(format.build());
            track = track.setTransferMode(AudioTrack.MODE_STREAM);
            return track.build();
        }

        private MediaCodec getDecoder() {
            MediaCodec decoder = null;
            try {
                decoder = MediaCodec.createDecoderByType(input.getString(MediaFormat.KEY_MIME));
                decoder.setCallback(this);
            } catch (Exception exception) {
                exception.printStackTrace();
            }
            return decoder;
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

        private int getFormatInteger(MediaFormat format, String key, int except) {
            int integer = except;
            try {
                integer = format.getInteger(key);
            } catch (Exception exception) {
                exception.printStackTrace();
                integer = except;
            }
            return integer;
        }
    }

    private static final String TAG = "AUDIO";

    private boolean isBorn;
    private boolean isPlaying;
    private long duration;
    private long time;
    private MediaExtractor extractor;
    private AudioManager manager;
    private ArrayList<Track> tracks;
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
                tracks = new ArrayList<Track>();
                extractor = new MediaExtractor();
                try {
                    MediaFormat format;
                    AssetFileDescriptor asset = null;
                    MediaMetadataRetriever data = new MediaMetadataRetriever();
                    asset = context.getAssets().openFd(path);
                    data.setDataSource(asset.getFileDescriptor(), asset.getStartOffset(), asset.getLength());
                    duration = Long.parseLong(data.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION));
                    data.release();
                    extractor.setDataSource(asset.getFileDescriptor(), asset.getStartOffset(), asset.getLength());
                    for (int i = 0; i != extractor.getTrackCount(); ++i) {
                        format = extractor.getTrackFormat(i);
                        Log.d(TAG, "Format @ "+i+": "+format);
                        if (format == null) {
                            tracks.add(null);
                        } else {
                            tracks.add(new Track(i, manager, format));
                        }
                    }
                    for (int i = 0; i != tracks.size(); ++i) {
                        if (tracks.get(i) == null) {
                            continue;
                        }
                        tracks.get(i).birth();
                    }
                } catch (Exception exceptionInner) {
                    exceptionInner.printStackTrace();
                }
            }
        } catch (Exception exceptionOuter) {
            exceptionOuter.printStackTrace();
        } finally {
            unlock();
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
                if (tracks != null) {
                    for (int i = 0; i != tracks.size(); ++i) {
                        if (tracks.get(i) == null) {
                            continue;
                        }
                        tracks.get(i).death();
                    }
                    tracks.clear();
                    tracks = null;
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
                for (int i = 0; i != tracks.size(); ++i) {
                    if (tracks.get(i) == null) {
                        continue;
                    }
                    if (!tracks.get(i).play()) {
                        result = false;
                        break;
                    }
                }
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
                for (int i = 0; i != tracks.size(); ++i) {
                    if (tracks.get(i) == null) {
                        continue;
                    }
                    if (!tracks.get(i).pause()) {
                        result = false;
                        break;
                    }
                }
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
        Track track;
        Status status;
        boolean check;
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
                    for (int i = 0; i != tracks.size(); ++i) {
                        track = tracks.get(i);
                        if (track == null) {
                            continue;
                        }
                        status = track.write(extractor);
                        check = false;
                        if (status != null) {
                            Log.v(TAG, "Status @ "+i+": "+status.name());
                            switch (status) {
                                case INCORRECT_TRACK:
                                    check = true;
                                    break;
                                case EXTRACTOR_DONE:
                                    break;
                                case BUFFER_NULL:
                                case BUFFER_FULL:
                                    death();
                                    break;
                            }
                        } else {
                            death();
                        }
                        if (check) {
                            continue;
                        }
                        time = extractor.getSampleTime();
                        break;
                    }
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
}
