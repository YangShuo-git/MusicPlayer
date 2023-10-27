package com.example.musicplayer.musicui.model;

import java.io.Serializable;

public class MusicData implements Serializable {
    private int mMusicRes;           // 音乐资源id
    private int mMusicPicRes;        // 专辑图片id
    private String mMusicName;       // 音乐名称
    private String mMusicAuthor;     // 作者

    public MusicData( String mMusicName) {
        this.mMusicName = mMusicName;
    }
    public MusicData(int mMusicRes, int mMusicPicRes, String mMusicName, String mMusicAuthor) {
        this.mMusicRes = mMusicRes;
        this.mMusicPicRes = mMusicPicRes;
        this.mMusicName = mMusicName;
        this.mMusicAuthor = mMusicAuthor;
    }

    public int getMusicRes() {
        return mMusicRes;
    }
    public int getMusicPicRes() {
        return mMusicPicRes;
    }
    public String getMusicName() {
        return mMusicName;
    }
    public String getMusicAuthor() {
        return mMusicAuthor;
    }
}
