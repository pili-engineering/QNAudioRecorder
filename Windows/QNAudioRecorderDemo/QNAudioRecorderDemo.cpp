// QNAudioRecorderDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <stdio.h>
#include "QNAudioRecorderDemo.h"

QNAudioRecorderDemo::QNAudioRecorderDemo() {

#ifndef _WIN64
  InitMinDump();
#endif // WIN32

  microphone_interface = QNAudioRecorder::CreateAudioRecorder();
  microphone_interface->SetAudioRecordingVolumeListener(this);
  int count = microphone_interface->GetAudioRecordingDeviceCount();
  std::cout << " microphone count : " << count;
  for (int i(0); i < count; ++i) {
    audio_info = microphone_interface->GetAudioRecordingDeviceInfo(i);
    if (audio_info.is_default) {
      microphone_interface->SetAudioRecordingDevice(i);
      break;
    }
  }
}

QNAudioRecorderDemo::~QNAudioRecorderDemo() {
  if (microphone_interface) {
    QNAudioRecorder::DestroyAudioRecorder(microphone_interface);
    microphone_interface = nullptr;
  }
}

void QNAudioRecorderDemo::StartRecording() {
  if (microphone_interface) {
    microphone_interface->StartRecording();
  }
}

void QNAudioRecorderDemo::StopRecording() {
  if (microphone_interface) {
    microphone_interface->StopRecording();
  }
}

void QNAudioRecorderDemo::OnRecordVolumeChanged(uint32_t volume) {
  std::cout << " record volume : " << volume << std::endl;
}

int main() {
  QNAudioRecorderDemo demo;
  demo.StartRecording();
  system("pause");
}
