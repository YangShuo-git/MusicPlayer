package com.example.musicplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;


import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.example.musicplayer.musicui.utils.DisplayUtil;
import com.example.musicplayer.lisnter.IPlayerListener;
import com.example.musicplayer.lisnter.IOnPreparedListener;
import com.example.musicplayer.opengl.AndGLSurfaceView;
import com.example.musicplayer.service.AndPlayer;


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
//
        File file = new File(Environment.getExternalStorageDirectory(),"input.mkv");
        paths.add(file.getAbsolutePath());
//        file = new File(Environment.getExternalStorageDirectory(),"input.avi");
//        paths.add(file.getAbsolutePath());

//        file = new File(Environment.getExternalStorageDirectory(),"input.rmvb");
//        file = new File(Environment.getExternalStorageDirectory(),"input.mp4");
        file = new File(Environment.getExternalStorageDirectory(),"brave_960x540.flv");
        paths.add(file.getAbsolutePath());
//        paths.add("http://mn.maliuedu.com/music/input.mp4");
        andPlayer.setPlayerListener(new IPlayerListener() {
            @Override
            public void onLoad(boolean load) {

            }
            @Override
            public void onCurrentTime(int currentTime, int totalTime) {
                // seek 需要当前时间、总时间
                if(!seek && totalTime > 0)
                {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            seekBar.setProgress(currentTime * 100 / totalTime);
                            tvTime.setText( DisplayUtil.secdsToDateFormat(currentTime)
                                    + "/" + DisplayUtil.secdsToDateFormat(totalTime));
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

    /**
     * 播放控制：开始 停止 暂停 恢复 切换
     */
    public void begin(View view) {
        andPlayer.setOnPreparedListener(new IOnPreparedListener() {
            @Override
            public void onPrepared() {
                andPlayer.start();  // 监听到Prepared()完成，就开始解码
            }
        });
//       File file = new File(Environment.getExternalStorageDirectory(),"input.rmvb");
//        File file = new File(Environment.getExternalStorageDirectory(),"input.mp4");
        File file = new File(Environment.getExternalStorageDirectory(),"brave_960x540.flv");
        andPlayer.setSource(file.getAbsolutePath());

//        andPlayer.setSource("http://sf1-hscdn-tos.pstatp.com/obj/media-fe/xgplayer_doc_video/flv/xgplayer-demo-360p.flv");
        andPlayer.prepared();
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
    public void speed0(View view) {
        andPlayer.setSpeed(1.0f);
    }
    public void speed1(View view) {
        andPlayer.setSpeed(1.5f);
    }
    public void speed2(View view) {
        andPlayer.setSpeed(2.0f);
    }
    public void normalTone(View view) {
        andPlayer.setTone(1.0f);
    }
    public void highTone(View view) {
        andPlayer.setTone(2.0f);
    }
    public void lowTone(View view) {
        andPlayer.setTone(0.5f);
    }
}
