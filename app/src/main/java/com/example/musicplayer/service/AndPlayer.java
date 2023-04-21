package com.example.musicplayer.service;

import android.text.TextUtils;
import android.util.Log;

import com.example.musicplayer.lisnter.IPlayerListener;
import com.example.musicplayer.lisnter.IOnPreparedListener;
import com.example.musicplayer.opengl.AndGLSurfaceView;

public class AndPlayer {
    IOnPreparedListener onPreparedListener;
    private AndGLSurfaceView andGLSfView;
    private IPlayerListener playerListener;

    static {
        System.loadLibrary("musicplayer");
    }
    private String source; // 数据源
    public void setSource(String source)
    {
        this.source = source;
    }
    public void setAndGLSurfaceView(AndGLSurfaceView surfaceview) {
        this.andGLSfView = surfaceview;
        Log.i("AndPlayer", "setAndGLSurfaceView: -------------" + this.hashCode());
    }
    
    // 设置prepared()、player的监听
    public void setOnPreparedListener(IOnPreparedListener iOnPreparedListener) {
        this.onPreparedListener = iOnPreparedListener;
    }
    public void setPlayerListener(IPlayerListener playerListener) {
        this.playerListener = playerListener;
    }

    /**
     * 在Native层调用java的方法  回调的方式，应用层入口
     */
    public void onCallPrepared() {
        Log.d("AndPlayer", "onCallPrepared");
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }
    public void onCallTimeInfo(int currentTime, int totalTime)
    {
        Log.d("AndPlayer", "onCallTimeInfo");
        if (playerListener == null) {
            return;
        }
        playerListener.onCurrentTime(currentTime, totalTime);
    }
    public void onCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v)
    {
        // opengl渲染  的java版本
//        if( this.davidView != null)
//        {
//            this.davidView.setYUVData(width, height, y, u, v);
//        }
    }
    public void prepared()
    {
        if(TextUtils.isEmpty(source)) {
            Log.e("AndPlayer","prepared source is empty");
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
    public void seek(int secds) { n_seek(secds); }
    public void setMute(int mute) {
        n_mute(mute);
    }
    public native void n_prepared(String source);
    public native void n_start();
    private native void n_pause();
    private native void n_seek(int secds);
    private native void n_resume();
    private native void n_mute(int mute);
    private native void n_volume(int percent);

}
