// QNAudioRecorderDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <stdio.h>
#include "QNAudioRecorderDemo.h"

QNAudioRecorderDemo::QNAudioRecorderDemo() {

#ifndef _WIN64
  InitMinDump();
#endif // WIN32

  audio_recorder = QNAudioRecorder::CreateAudioRecorder();
  int count = audio_recorder->GetAudioRecordingDeviceCount();
  std::cout << " microphone count : " << count;
  for (int i(0); i < count; ++i) {
    QNAudioDeviceInfo audio_info = audio_recorder->GetAudioRecordingDeviceInfo(i);
    // 这里选择系统默认采集设备
    if (audio_info.is_default) {
      audio_recorder->SetAudioRecordingDevice(i);
      break;
    }
  }
}

QNAudioRecorderDemo::~QNAudioRecorderDemo() {
  if (audio_recorder) {
    QNAudioRecorder::DestroyAudioRecorder(audio_recorder);
    audio_recorder = nullptr;
  }
}

void QNAudioRecorderDemo::StartRecording() {
  if (audio_recorder) {
    audio_recorder->Start(this);
  }
}

void QNAudioRecorderDemo::StopRecording() {
  if (audio_recorder) {
    audio_recorder->Stop();
  }
}

void QNAudioRecorderDemo::OnVolumeChanged(double volume) {
  std::cout << " record volume : " << volume << std::endl;
}

int main() {
  QNAudioRecorderDemo demo;
  demo.StartRecording();
  system("pause");
}
