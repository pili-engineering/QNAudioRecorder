#ifndef QN_MICROPHONE_DEMO_H
#define QN_MICROPHONE_DEMO_H
#include "minidump.h"
#include "../QNAudioDeviceModule/qn_audio_recorder_interface.h"
using namespace qiniu;

class QNAudioRecorderDemo
    : public QNAudioRecorder::QNAudioVolumeCallback {
 public:
  QNAudioRecorderDemo();
  ~QNAudioRecorderDemo();

  void StartRecording();
  void StopRecording();
  // implements of QNRecordVolumeListener
  virtual void OnVolumeChanged(double volume);

 private:
  QNAudioRecorder* microphone_interface;
};


#endif // QN_MICROPHONE_DEMO_H