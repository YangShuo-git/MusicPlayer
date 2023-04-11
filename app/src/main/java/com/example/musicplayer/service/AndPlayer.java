package com.example.musicplayer.service;

import android.text.TextUtils;
import android.util.Log;

import com.example.musicplayer.lisnter.IPlayerListener;
import com.example.musicplayer.lisnter.MNOnParparedListener;

public class AndPlayer {
    MNOnParparedListener mnOnParparedListener;
    static {
        System.loadLibrary("native-lib");
    }
    private String source;//数据源
    public void setSource(String source)
    {
        this.source = source;
    }
    public void parpared()
    {
        if(TextUtils.isEmpty(source))
        {
            Log.d("BaiYang","source not be empty");
            return;
        }

        // native层  开启子线程
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
            }
        }).start();
    }

    public void start()
    {
        if(TextUtils.isEmpty(source))
        {
            Log.d("david","source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }
    public void pause() {
        n_pause();
    }

    public native void n_start();
    private native void n_pause();


    public native void n_parpared(String source);


    public void onCallTimeInfo(int currentTime, int totalTime)
    {
        if (playerListener == null) {
            return;
        }
        playerListener.onCurrentTime(currentTime, totalTime);

    }
    public void onCallParpared() {
        Log.d("david--->", "onCallParpared");

        if (mnOnParparedListener != null) {
            mnOnParparedListener.onParpared();
        }
    }

    public void setMnOnParparedListener(MNOnParparedListener mnOnParparedListener) {
        this.mnOnParparedListener = mnOnParparedListener;
    }
    private IPlayerListener playerListener;

    public void setPlayerListener(IPlayerListener playerListener) {
        this.playerListener = playerListener;
    }

    public void seek(int secds) {
        n_seek(secds);
    }
    private native void n_seek(int secds);
    private native void n_resume();
    private native void n_mute(int mute);
    private native void n_volume(int percent);

    public void setMute(int mute) {
        n_mute(mute);
    }
}
