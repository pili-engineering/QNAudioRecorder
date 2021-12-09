## 设备以及系统要求

- 设备要求：iPhone 6 及以上
- 系统要求：iOS 9.0 及以上


## 开发环境配置

- Xcode 开发工具。App Store 下载地址[下载地址](https://itunes.apple.com/cn/app/xcode/id497799835?ls=1&mt=12)


## 编译获取 SDK

编译 [iOS 文件夹](https://github.com/pili-engineering/QNAudioRecorder/tree/main/iOS)下的 `QNAudioRecorder`，工程 Products 会自动生成 QNAudioRecorder_iOS.framework，右键 "Show in Finder" 后，即可拿到对应的 SDK 真机版本，注意区分 Debug 和 Release。


## 手动导入 SDK

将获取得到的 QNAudioRecorder_iOS.framework 导入到你的 Xcode 工程中

在需要使用的类中，引入后使用，具体方式可参见 QNAudioRecorderDemo

```Objective-C
#import <QNAudioRecorder_iOS/QNAudioRecorder_iOS.h>
```


## 添加权限说明

我们需要在 Info.plist 文件中添加相应权限的说明，否则程序在 iOS 10 及以上系统会出现崩溃。

由于使用到了系统麦克风，故需要添加如下权限：

- 麦克风权限：Privacy - Microphone Usage Description 是否允许 App 使用麦克风


## API 使用

### 开启录制

```objc
QNAudioRecorder *recorder = [QNAudioRecorder start];
```

### 设置代理

```objc
recorder.delegate = self;
```

### 实现回调

```objc
- (void)audioRecorder:(QNAudioRecorder *)audioRecorder volume:(double)volume {
    dispatch_async(dispatch_get_main_queue(), ^{
        // 更新 UI 展示音量
    });
}
```

### 停止录制

```objc
BOOL isSuccess = [recorder stop];
```