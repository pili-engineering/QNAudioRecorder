## 设备以及系统要求

- 系统要求：Android 4.3 (API 18) 及以上
- Java 版本: Java 8+

## 开发环境

- Android Studio 开发工具，官方<a href="http://developer.android.com/intl/zh-cn/sdk/index.html" target="_blank">下载地址</a>
- Android 官方开发 SDK，官方<a href="https://developer.android.com/intl/zh-cn/sdk/index.html#Other" target="_blank">下载地址</a>

## SDK 导入
将 app/src/main/java/com/qiniu/droid/audio/recorder/library/ 包中的源码文件导入到项目工程使用即可.

## 接口使用
### 1. 添加应用权限
```java
<uses-permission android:name="android.permission.RECORD_AUDIO" />
```

### 2. 开始音频录制
> 音频录制需要访问平台硬件麦克风，为独占方式，不允许重复 start；需要保证 stop 后再次 start
```java
```java
   static QNAudioRecorder mAudioRecorder;

   mAudioRecorder = QNAudioRecorder.start(new QNAudioRecorder.QNAudioVolumeCallback() {
       @Override
       /**
        * 音量回调方法
        *
        * @param volume 音量值 [0.0 - 1.0f]
        */
       public void onVolumeChanged(double volume) {
           Log.i(MainActivity.class.getSimpleName(), "volume " + volume);
           runOnUiThread(() -> {
               mVolumeText.setText("音量: " + volume);
           });
       }
   });
   mStarted = (mAudioRecorder != null);

```

### 3. 停止音频录制
```java
    boolean success = mAudioRecorder.stop();
```
