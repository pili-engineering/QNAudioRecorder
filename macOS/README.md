## 设备以及系统要求

- 系统要求：macOS 10.10 及以上


## 开发环境配置

- Xcode 开发工具。App Store 下载地址[下载地址](https://itunes.apple.com/cn/app/xcode/id497799835?ls=1&mt=12)


## 编译获取 SDK

编译 [macOS 文件夹](https://github.com/pili-engineering/QNAudioRecorder//tree/main/macOS)下的 `QNAudioRecorder_macOS`，工程 Products 会自动生成 QNAudioRecorder_macOS.framework。


## 手动导入 SDK

将获取得到的 QNAudioRecorder_macOS.framework 导入到你的 Xcode 工程中

在需要使用的类中，引入后使用，具体方式可参见 QNAudioRecorder-demo

```Objective-C
#import <QNAudioRecorder_macOS/QNAudioRecorder_macOS.h>
```


## 添加权限说明

我们需要在 Info.plist 文件中添加相应权限的说明，否则程序在 macOS 10.10 及以上系统会出现崩溃。

由于使用到了系统麦克风，故需要添加如下权限：

- 麦克风权限：Privacy - Microphone Usage Description 是否允许程序使用麦克风


## API 使用

### 开启录制

该方法使用到了系统内置麦克风设备，该设备为系统独占资源，请勿重复调用。

```objc
QNAudioRecorder *recorder = [QNAudioRecorder start];
```

### 设置代理

```objc
recorder.delegate = self;
```

### 实现回调

```objc
-(void)audioRecorder:(QNAudioRecorder *)audioReocrder onVolumeChanged:(double)volume{
    dispatch_async(dispatch_get_main_queue(), ^{
        // 更新 UI 展示音量
    });
}
```

### 停止录制

```objc
BOOL isSuccess = [recorder stop];
```
