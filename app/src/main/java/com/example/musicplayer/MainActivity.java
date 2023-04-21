package com.example.musicplayer;

import static com.example.musicplayer.musicui.widget.DiscView.DURATION_NEEDLE_ANIAMTOR;
import static com.example.musicplayer.service.MusicService.ACTION_OPT_MUSIC_VOLUME;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;


import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.example.musicplayer.lisnter.IPlayerListener;
import com.example.musicplayer.lisnter.IOnPreparedListener;
import com.example.musicplayer.musicui.model.MusicData;
import com.example.musicplayer.musicui.utils.DisplayUtil;
import com.example.musicplayer.musicui.widget.BackgourndAnimationRelativeLayout;
import com.example.musicplayer.musicui.widget.DiscView;
import com.example.musicplayer.opengl.AndGLSurfaceView;
import com.example.musicplayer.service.AndPlayer;
import com.example.musicplayer.service.MusicService;


public class MainActivity extends AppCompatActivity {
    private AndPlayer andPlayer;
    private TextView tvTime;
    private AndGLSurfaceView andGLSurfaceView;
    private SeekBar seekBar;
    private int position;
    private boolean seek = false;
    List<String> paths = new ArrayList<>();

    static {
        System.loadLibrary("musicplayer");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main_video);
        andGLSurfaceView = findViewById(R.id.andglsurfaceview);
        seekBar = findViewById(R.id.seekbar);
        tvTime = findViewById(R.id.tv_time);
        checkPermission();

        andPlayer = new AndPlayer();
        andPlayer.setAndGLSurfaceView(andGLSurfaceView);

        File file = new File(Environment.getExternalStorageDirectory(),"input.mkv");
        paths.add(file.getAbsolutePath());
        file = new File(Environment.getExternalStorageDirectory(),"input.avi");
        paths.add(file.getAbsolutePath());

//        file = new File(Environment.getExternalStorageDirectory(),"input.rmvb");
        file = new File(Environment.getExternalStorageDirectory(),"input.mp4");
        paths.add(file.getAbsolutePath());
        paths.add("http://mn.maliuedu.com/music/input.mp4");
        andPlayer.setPlayerListener(new IPlayerListener() {
            @Override
            public void onLoad(boolean load) {

            }

            @Override
            public void onCurrentTime(int currentTime, int totalTime) {
                if(!seek &&totalTime> 0)
                {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            seekBar.setProgress(currentTime* 100 / totalTime);
                            tvTime.setText( DisplayUtil.secdsToDateFormat(currentTime)
                                    + "/" + DisplayUtil.secdsToDateFormat( totalTime));
                        }
                    });

                }
            }
            @Override
            public void onError(int code, String msg) {

            }

            @Override
            public void onPause(boolean pause) {

            }

            @Override
            public void onDbValue(int db) {

            }

            @Override
            public void onComplete() {

            }

            @Override
            public String onNext() {
                return null;
            }
        });

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                position = progress * andPlayer.getDuration() / 100;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                seek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                andPlayer.seek(position);
                seek = false;
            }
        });

    }

    public boolean checkPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(
                Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
            }, 1);

        }
        return false;
    }

    public void begin(View view) {

        andPlayer.setOnPreparedListener(new IOnPreparedListener() {
            @Override
            public void onParpared() {
                MyLog.d("准备好了，可以开始播放声音了");
                andPlayer.start();
            }
        });
//音视频面试   30道 心里分析  切入点   步骤 
//       File file = new File(Environment.getExternalStorageDirectory(),"input.rmvb");
        File file = new File(Environment.getExternalStorageDirectory(),"input.mp4");

        andPlayer.setSource(file.getAbsolutePath());
//       andPlayer.setSource("rtmp://58.200.131.2:1935/livetv/cctv1");
//        andPlayer.setSource("http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8");
//        andPlayer.setSource("http://mn.maliuedu.com/music/input.mp4");
//        wlPlayer.setSource("/mnt/shared/Other/testvideo/楚乔传第一集.mp4");
//        andPlayer.setSource("/mnt/shared/Other/testvideo/屌丝男士.mov");
//        wlPlayer.setSource("http://ngcdn004.cnr.cn/live/dszs/index12.m3u8");
        andPlayer.parpared();
    }

    public void pause(View view) {

        andPlayer.pause();

    }

    public void resume(View view) {
        andPlayer.resume();
    }


    public void stop(View view) {
        andPlayer.stop();
    }


    public void next(View view) {
        //wlPlayer.playNext("/mnt/shared/Other/testvideo/楚乔传第一集.mp4");
    }

    public void speed1(View view) {
        andPlayer.setSpeed(1.5f);

    }

    public void speed2(View view) {
        andPlayer.setSpeed(2.0f);
    }
}
