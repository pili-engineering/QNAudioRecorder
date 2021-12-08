## 开发语言以及系统要求

- 开发语言：C++14
- 系统要求：Windows 7 及以上版本的 Windows 系统

## 开发环境

- [Visual Studio 2017](https://www.visualstudio.com/zh-hans/vs/older-downloads/) 或 [Qt Creator](http://download.qt.io/archive/qt/)
- Win32 Platform


## 接口使用


### 1. 创建录制类
```c++
  microphone_interface = QNAudioRecorder::CreateAudioRecorder(this);
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
    microphone_interface->StartRecording();
  }
```

### 5. 停止音频录制
```c++
  if (microphone_interface) {
     microphone_interface->StopRecording();
  }

```
