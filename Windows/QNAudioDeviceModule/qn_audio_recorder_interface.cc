#include "qn_audio_recorder_interface.h"
#include "qn_microphone_manager.h"

namespace qiniu {
  QNAudioRecorder* QNAudioRecorder::CreateAudioRecorder() {
    QNMicrophoneManager* tmp_ptr = QNMicrophoneManager::ObtainMicrophoneSessionInterface();
    if (!tmp_ptr) {
      return nullptr;
    }
    return dynamic_cast<QNAudioRecorder*>(tmp_ptr);
  }

  void QNAudioRecorder::DestroyAudioRecorder(QNAudioRecorder* interface_ptr) {
    if (interface_ptr) {
      QNMicrophoneManager::DestroyMicrophoneSessionInterface((QNMicrophoneManager*)interface_ptr);
    }
  }
}