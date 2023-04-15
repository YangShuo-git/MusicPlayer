package com.example.musicplayer.service;

import android.text.TextUtils;
import android.util.Log;

import com.example.musicplayer.lisnter.IPlayerListener;
import com.example.musicplayer.lisnter.IOnPreparedListener;

public class AndPlayer {
    IOnPreparedListener onPreparedListener;

    static {
        System.loadLibrary("musicplayer");
    }
    private String source; // 数据源
    public void setSource(String source)
    {
        this.source = source;
    }
    public void prepared()
    {
        if(TextUtils.isEmpty(source)) {
            Log.e("AndPlayer","source is empty");
            return;
        }
        // 开启解封装线程  调用Native层方法  准备工作
        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d("AndPlayer","start pthread to prepare");
                n_prepared(source);
            }
        }).start();
    }

    public void start()
    {
        if(TextUtils.isEmpty(source)) {
            Log.e("AndPlayer","source is empty");
            return;
        }
        // 开启解码线程  调用Native层方法  开始播放
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

    private native void n_pause();

    public native void n_prepared(String source);
    public native void n_start();

    // 在Native层调用onCallPrepared、onCallTimeInfo
    public void onCallPrepared() {
        Log.d("AndPlayer", "onCallPrepared");
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }
    public void onCallTimeInfo(int currentTime, int totalTime)
    {
        if (playerListener == null) {
            return;
        }
        playerListener.onCurrentTime(currentTime, totalTime);

    }

    // 设置prepared()、player的监听
    public void setOnPreparedListener(IOnPreparedListener iOnPreparedListener) {
        this.onPreparedListener = iOnPreparedListener;
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
