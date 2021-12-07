package com.qiniu.droid.audio.recorder.activity;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.qiniu.droid.audio.recorder.PermissionChecker;
import com.qiniu.droid.audio.recorder.QNAudioRecorder;
import com.qiniu.droid.audio.recorder.R;


public class MainActivity extends AppCompatActivity {

   QNAudioRecorder mAudioRecorder;
   TextView mVolumeText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mVolumeText = findViewById(R.id.volume_text);
        mAudioRecorder = new QNAudioRecorder(new QNAudioRecorder.QNAudioVolumeCallback() {
            @Override
            public void onVolumeChanged(double volume) {
                Log.i(MainActivity.class.getSimpleName(), "volume " + volume);
                runOnUiThread(() -> {
                    mVolumeText.setText("音量: " + volume);
                });
            }
        });
    }

    public void startRecord(View view) {
        if (!isPermissionOK()) {
            Toast.makeText(this, "Some permissions is not approved !!!", Toast.LENGTH_SHORT).show();
            return;
        }
        boolean success = mAudioRecorder.startRecording();
        Toast.makeText(this, "开始采集 " + (success ? "成功" : "失败"), Toast.LENGTH_SHORT).show();
    }

    public void stopRecord(View view) {
        boolean success = mAudioRecorder.stopRecording();
        Toast.makeText(this, "停止采集 " + (success ? "成功" : "失败"), Toast.LENGTH_SHORT).show();
    }

    private boolean isPermissionOK() {
        PermissionChecker checker = new PermissionChecker(this);
        return Build.VERSION.SDK_INT < Build.VERSION_CODES.M || checker.checkPermission();
    }
}
