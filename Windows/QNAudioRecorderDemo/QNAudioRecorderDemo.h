#ifndef QN_MICROPHONE_DEMO_H
#define QN_MICROPHONE_DEMO_H
#include "minidump.h"
#include "../QNAudioDeviceModule/qn_audio_recorder_interface.h"
using namespace qiniu;

class QNAudioRecorderDemo
    : public QNAudioRecorder::QNAudioRecordingVolumeListener {
 public:
  QNAudioRecorderDemo();
  ~QNAudioRecorderDemo();

  void StartRecording();
  void StopRecording();
  // implements of QNRecordVolumeListener
  virtual void OnRecordVolumeChanged(uint32_t volume);

  QNAudioDeviceInfo audio_info;
  QNAudioRecorder* microphone_interface;
};


#endif // QN_MICROPHONE_DEMO_H