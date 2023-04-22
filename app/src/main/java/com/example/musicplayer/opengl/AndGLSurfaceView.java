package com.example.musicplayer.opengl;

import android.opengl.GLSurfaceView;
import android.content.Context;
import android.util.AttributeSet;

public class AndGLSurfaceView extends GLSurfaceView {
    private AndRender andRender;
    public AndGLSurfaceView(Context context) {
        this(context, null);
    }

    public AndGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        andRender = new AndRender(context);
        setRenderer(andRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v)
    {
        if (andRender != null) {
            andRender.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }
}