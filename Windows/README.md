## 开发语言以及系统要求

- 开发语言：C++14
- 系统要求：Windows 7 及以上版本的 Windows 系统

## 开发环境

- [Visual Studio 2017](https://www.visualstudio.com/zh-hans/vs/older-downloads/) 或 [Qt Creator](http://download.qt.io/archive/qt/)

## sdk 获取
bin 目录下获取相应版本的 dll 和头文件

## 接口使用


### 1. 创建录制类
```c++
  microphone_interface = QNAudioRecorder::CreateAudioRecorder();
```

### 2. 释放录制类
```c++
  if (microphone_interface) {
    QNAudioRecorder::DestroyAudioRecorder(microphone_interface);
    microphone_interface = nullptr;
  }
```

### 3. 选择麦克风设备
```c++
  int count = microphone_interface->GetAudioRecordingDeviceCount();
  std::cout << " microphone count : " << count;
  for (int i(0); i < count; ++i) {
    QNAudioDeviceInfo audio_info = microphone_interface->GetAudioRecordingDeviceInfo(i);
    // 这里选择系统默认采集设备
    if (audio_info.is_default) {
      microphone_interface->SetAudioRecordingDevice(i);
      break;
    }
  }
```

### 4. 开始音频录制
```c++
  if (microphone_interface) {
    microphone_interface->StartRecording(this);
  }
```
### 5.实现回调
```c++
void QNAudioRecorderDemo::OnVolumeChanged(double volume) {
  // 音量更新 
  std::cout << " record volume : " << volume << std::endl;
}
```

### 6. 停止音频录制
```c++
  if (microphone_interface) {
     microphone_interface->StopRecording();
  }

```
