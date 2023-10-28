package com.example.musicplayer.service;

import android.text.TextUtils;
import android.util.Log;

import com.example.musicplayer.lisnter.IPlayerListener;
import com.example.musicplayer.lisnter.IOnPreparedListener;

public class AndPlayer {
    IOnPreparedListener onPreparedListener;
    private IPlayerListener playerListener;
    private int duration = 0;  // 总时长

    static {
        System.loadLibrary("musicplayer");
    }
    private String source; // 数据源
    public void setSource(String source)
    {
        this.source = source;
    }

    /**
     * 设置prepared()、player的监听
     */
    public void setOnPreparedListener(IOnPreparedListener iOnPreparedListener) {
        this.onPreparedListener = iOnPreparedListener;
    }
    public void setPlayerListener(IPlayerListener playerListener) {
        this.playerListener = playerListener;
    }

    /**
     * 1、C++调用java的方法
     * onCallPrepared、onCallTimeInfo，这两个方法是在native层进行回调，所以java层没有显示usage;
     */
    public void onCallPrepared() {
        Log.d("AndPlayer", "onCallPrepared");
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }
    public void onCallTimeInfo(int currentTime, int totalTime)
    {
//        Log.d("AndPlayer", "onCallTimeInfo");
        duration = totalTime;
        if (playerListener == null) {
            return;
        }
        playerListener.onCurrentTime(currentTime, totalTime);
    }

    /**
     * 播放器的主要逻辑：perpared、start、pause、resume
     */
    public void prepared()
    {
        if(TextUtils.isEmpty(source)) {
            Log.e("AndPlayer","prepared source is empty");
            return;
        }
        // 开启解封装线程  调用Native层方法  是准备工作
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
            Log.e("AndPlayer","start source is empty");
            return;
        }
        // 开启解码线程  调用Native层方法  开始解码播放
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }

    public void pause() { n_pause(); }
    public void resume() { n_resume(); }
    public int getDuration() { return duration; }
    public void seek(int secds) { n_seek(secds); }
    public void setMute(int mute) {
        n_mute(mute);
    }
    public void setSpeed(float speed) { n_speed(speed); }
    public void setTone(float tone) { n_setTone(tone); }
    public void stop() {}


    /**
     * 2、java调用C++的方法
     * native层接口
     */
    public native void n_prepared(String source);
    public native void n_start();
    private native void n_pause();
    private native void n_seek(int secds);
    private native void n_resume();
    private native void n_mute(int mute);
    private native void n_volume(int percent);
    private native void n_speed(float speed);
    private native void n_setTone(float tone);
}
