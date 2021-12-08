#include "qn_microphone_manager.h"   
#include <mmdeviceapi.h>

#define  CALLBACK_PER_MS  100

namespace qiniu {

  QNMicrophoneManager*  QNMicrophoneManager::global_microphone_session_ = nullptr;
  QNMicrophoneManager* QNMicrophoneManager::ObtainMicrophoneSessionInterface(QNAudioVolumeCallback* listener) {
    if (!global_microphone_session_) {
      global_microphone_session_ = new QNMicrophoneManager(listener);
    }
    return global_microphone_session_;
  }
  void QNMicrophoneManager::DestroyMicrophoneSessionInterface(QNMicrophoneManager* ptr) {
    try {
      if (ptr) {
        delete ptr;
        global_microphone_session_ = nullptr;
      }
    } catch (const std::exception& e) {
      QN_LOG_ERROR << e.what();
    }  catch (...) {
      QN_LOG_ERROR << "unknow exception!";
    }
  }

  QNMicrophoneManager::QNMicrophoneManager(QNAudioVolumeCallback* listener)
      : record_volume_listener_(listener)
      , last_callback_time_(std::chrono::steady_clock::now()){

      //create adm
    adm_ptr_ = webrtc::CreateAudioDeviceWithDataObserver(
      webrtc::AudioDeviceModule::AudioLayer::kPlatformDefaultAudio,
      this);
    if (adm_ptr_->Init() != 0) {
      QN_LOG_ERROR << "Adm Init failed!";
      return ;
    }

    if (!adm_ptr_) {
      QN_LOG_ERROR << "Create adm failed!";
      return;
    }
  }

  QNMicrophoneManager::~QNMicrophoneManager() {
    if (adm_ptr_) {
      if (adm_ptr_->Recording()) {
        StopRecording();
      }
      adm_ptr_->Terminate();
      adm_ptr_.release();
      adm_ptr_ = nullptr;
    }
  }

  void QNMicrophoneManager::OnCaptureData(
      const void* audio_samples,
      const size_t num_samples,
      const size_t bytes_per_sample,
      const size_t num_channels,
      const uint32_t samples_per_sec) {
    std::chrono::steady_clock::time_point now_time = std::chrono::steady_clock::now();
    if (record_volume_listener_ ) {
      double volume = 0.0;
      volume = ProcessAudioLevel(
        (int16_t *)audio_samples,
        bytes_per_sample * num_samples / sizeof(int16_t));
      auto dur_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        now_time - last_callback_time_).count();
      if (dur_time >= CALLBACK_PER_MS) {
        record_volume_listener_->OnVolumeChanged(volume);
        last_callback_time_ = now_time;
      }
    }
  }

  void QNMicrophoneManager::OnRenderData(
      const void* audio_samples,
      const size_t num_samples,
      const size_t bytes_per_sample,
      const size_t num_channels,
      const uint32_t samples_per_sec) {
  }

  void QNMicrophoneManager::SetAudioRecordingVolumeListener(QNAudioVolumeCallback* listener) {
    record_volume_listener_ = listener;
  }

  int32_t QNMicrophoneManager::GetAudioRecordingDeviceCount() {
    return GetAudioRecordingDeviceCountInternal();
  }

  QNAudioDeviceInfo& QNMicrophoneManager::GetAudioRecordingDeviceInfo(int32_t index) {
    return GetAudioRecordingDeviceInfoInternal(index);
  }

  int32_t QNMicrophoneManager::SetAudioRecordingDevice(int32_t index) {
    return SetAudioRecordingDeviceInternal(index);
  }

  int32_t QNMicrophoneManager::StartRecording() {
    return StartRecordingInternal();
  }

  int32_t QNMicrophoneManager::StopRecording() {
    return StopRecordingInternal();
  }

  int32_t QNMicrophoneManager::GetAudioRecordingDeviceCountInternal() {
    QN_LOG_INFO;
    if (!adm_ptr_) {
      QN_LOG_ERROR << "adm is null";
      return Err_Internal_Null_Pointer;
    }

    audio_recording_device_info_vec_.clear();
    int dev_num = adm_ptr_->RecordingDevices();
    if (dev_num < 0) {
      QN_LOG_ERROR << "adm get recording device number failed!";
      return QNRTC_FAILED;
    }
    // 获取默认设备并前置 
    string default_device_id;
    auto ret = GetDefaultDeviceId(true, default_device_id);
    if (QNRTC_OK == ret && !default_device_id.empty()) {
      QN_LOG_INFO << "GetDefaultDeviceId success:" << default_device_id;
    } else {
      QN_LOG_WARN << "GetDefaultDeviceId failed!";
    }

    char dev_name[webrtc::kAdmMaxDeviceNameSize] = { 0 };
    char dev_guid[webrtc::kAdmMaxDeviceNameSize] = { 0 };
    for (int i(0); i < dev_num; ++i) {
      QNAudioDeviceInfo info;
      memset(dev_name, 0, sizeof(dev_name));
      memset(dev_guid, 0, sizeof(dev_guid));
      adm_ptr_->RecordingDeviceName(i, dev_name, dev_guid);

      wstring w_name = Utf8ToWstring(dev_name);
      string name = UnicodeToUtf(w_name.c_str());
      wstring w_guid = Utf8ToWstring(dev_guid);
      string guid = UnicodeToUtf(w_guid.c_str());
      info.id = guid;
      info.name = name;

      if (default_device_id.compare(info.id) == 0) {
        // 默认设备 
        info.is_default = true;
        audio_recording_device_info_vec_.emplace_back(info);
      } else {
        audio_recording_device_info_vec_.emplace_back(info);
      }

      QN_LOG_INFO << ", enum recording device name:" << dev_name
        << ", device guid:" << guid;
    }
    return audio_recording_device_info_vec_.size();
  }

  QNAudioDeviceInfo& QNMicrophoneManager::GetAudioRecordingDeviceInfoInternal(int32_t index) {
    QNAudioDeviceInfo tmp_recording_info;
    QN_LOG_INFO << "audio recording index:" << index
      << ", current audio recording size:" << audio_recording_device_info_vec_.size();
    if ((int32_t)audio_recording_device_info_vec_.size() <= index) {
      return tmp_recording_info;
    }
    return audio_recording_device_info_vec_.at(index);
  }

  int32_t QNMicrophoneManager::GetAudioPlaybackDeviceCountInternal() {
    QN_LOG_INFO;
    if (!adm_ptr_) {
      QN_LOG_ERROR << "adm is null";
      return Err_Internal_Null_Pointer;
    }

    audio_playback_device_info_vec_.clear();
    int dev_num = adm_ptr_->PlayoutDevices();
    if (dev_num < 0) {
      QN_LOG_ERROR << "adm get recording device number failed!";
      return QNRTC_FAILED;
    }
    // 获取默认设备并前置 
    string default_device_id;
    auto ret = GetDefaultDeviceId(false, default_device_id);
    if (QNRTC_OK == ret && !default_device_id.empty()) {
      QN_LOG_INFO << "GetDefaultDeviceId success:" << default_device_id;
    } else {
      QN_LOG_WARN << "GetDefaultDeviceId failed!";
    }

    char dev_name[webrtc::kAdmMaxDeviceNameSize] = { 0 };
    char dev_guid[webrtc::kAdmMaxDeviceNameSize] = { 0 };
    for (int i(0); i < dev_num; ++i) {
      QNAudioDeviceInfo info;
      memset(dev_name, 0, sizeof(dev_name));
      memset(dev_guid, 0, sizeof(dev_guid));
      adm_ptr_->PlayoutDeviceName(i, dev_name, dev_guid);

      wstring w_name = Utf8ToWstring(dev_name);
      string name = UnicodeToUtf(w_name.c_str());
      wstring w_guid = Utf8ToWstring(dev_guid);
      string guid = UnicodeToUtf(w_guid.c_str());

      info.id = guid;
      info.name = name;

      if (default_device_id.compare(info.id) == 0) {
        // 默认设备 
        info.is_default = true;
        audio_playback_device_info_vec_.emplace_back(info);
      } else {
        audio_playback_device_info_vec_.emplace_back(info);
      }

      QN_LOG_INFO << ", enum playout device name:" << dev_name
        << ", device guid:" << guid;
    }
    return audio_playback_device_info_vec_.size();
  }

  QNAudioDeviceInfo QNMicrophoneManager::GetAudioPlaybackDeviceInfoInternal(int32_t index) {
    QNAudioDeviceInfo tmp_playback_info;
    QN_LOG_INFO << "audio playback index:" << index
      << ", current audio playback size:" << audio_playback_device_info_vec_.size();
    if ((int32_t)audio_playback_device_info_vec_.size() <= index) {
      return tmp_playback_info;
    }
    return audio_playback_device_info_vec_.at(index);
  }

  int32_t QNMicrophoneManager::SetAudioRecordingDeviceInternal(int32_t index) {
    QN_LOG_INFO;
    if (!adm_ptr_) {
      QN_LOG_ERROR << "adm_ptr_ is null";
      return Err_Internal_Null_Pointer;
    }

    if (adm_ptr_->SetRecordingDevice(index) < 0) {
      QN_LOG_ERROR
        << " SetRecordingDevice index failed, please check the device index is valid?";
      return Err_Failed_Set_Recorder_Device;
    }
    return QNRTC_OK;
  }

  int32_t QNMicrophoneManager::SetAudioPlaybackDeviceInternal(int32_t index) {
    QN_LOG_INFO;
    if (!adm_ptr_) {
      QN_LOG_ERROR << "adm_ptr_ is null";
      return Err_Internal_Null_Pointer;
    }

    if (adm_ptr_->SetPlayoutDevice(index) < 0) {
      QN_LOG_ERROR
        << " SetRecordingDevice index failed, please check the device index is valid?";
      return Err_Failed_Set_Recorder_Device;
    }
    return QNRTC_OK;
  }

  int32_t QNMicrophoneManager::StartRecordingInternal() {
    QN_LOG_INFO;
    if (!adm_ptr_) {
      QN_LOG_ERROR << "adm_ptr_ is null";
      return Err_Internal_Null_Pointer;
    }
    if (!adm_ptr_->RecordingIsInitialized()) {
      adm_ptr_->InitRecording();
    }
    if (adm_ptr_->Recording()) {
      return QNRTC_OK;
    }
    return adm_ptr_->StartRecording();
  }

  int32_t QNMicrophoneManager::StopRecordingInternal() {
    QN_LOG_INFO;
    if (!adm_ptr_) {
      QN_LOG_ERROR << "adm_ptr_ is null";
      return Err_Internal_Null_Pointer;
    }
    if (!adm_ptr_->Recording()) {
      return QNRTC_OK;
    }
    return adm_ptr_->StopRecording();
  }

  int32_t QNMicrophoneManager::GetAudioVolumeInternal(AudioDeviceType device_type, uint32_t* volume) {
    uint32_t tmp_volume(0);
    if (!adm_ptr_) {
      return Err_Internal_Null_Pointer;
    }
    if (kAudioDeviceRecord == device_type) {
      if (0 == adm_ptr_->MicrophoneVolume(&tmp_volume)) {
        tmp_volume = tmp_volume / 255 * 100;
        *volume = tmp_volume;
        return QNRTC_OK;
      } else {
        return QNRTC_FAILED;
      }
    } else if (kAudioDevicePlayout == device_type) {
      if (0 == adm_ptr_->SpeakerVolume(&tmp_volume)) {
        tmp_volume = tmp_volume / 255 * 100;
        *volume = tmp_volume;
        return QNRTC_OK;
      } else {
        return QNRTC_FAILED;
      }
    }
    return QNRTC_FAILED;
  }

  double QNMicrophoneManager::ProcessAudioLevel(const int16_t* data, const int32_t& data_size) {
    double ret = 0.0;
    if (data_size > 0) {
      int32_t sum = 0;
      int16_t* pos = (int16_t *)data;
      for (int i = 0; i < data_size; i++) {
        sum += abs(*pos);
        pos++;
      }

      ret = sum * 5.0 / (data_size * VOLUMEMAX);
      ret = core_min(ret, 1.0);
    }

    return ret;
  }

  std::wstring QNMicrophoneManager::Utf8ToWstring(const std::string& utf8_str) {
#ifdef _WIN32
    int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), NULL, 0);
    wchar_t* pUnicode;
    pUnicode = new wchar_t[unicodeLen + 1];
    memset((void*)pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
    ::MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), (LPWSTR)pUnicode, unicodeLen);
    std::wstring wstrReturn(pUnicode);
    delete[] pUnicode;
    return wstrReturn;
#else
    return L"";
#endif // WIN32
  }

  std::string QNMicrophoneManager::WstringToUtf8(const std::wstring& wstr) {
#ifdef _WIN32
    char* pElementText;
    int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    pElementText = new char[iTextLen + 1];
    memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
    ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wstr.c_str(), (int)wstr.size(), pElementText, iTextLen, NULL, NULL);
    std::string strReturn(pElementText);
    delete[] pElementText;
    return strReturn;
#else
    return "";
#endif // WIN32
  }

  std::string QNMicrophoneManager::UnicodeToUtf(std::wstring str) {
#ifdef _WIN32
    std::string return_value;
    //获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的
    int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0, NULL, NULL);
    char *buffer = new(std::nothrow) char[len + 1];
    if (NULL == buffer) {
      return NULL;
    }
    WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    //删除缓冲区并返回值
    return_value.append(buffer);
    delete[]buffer;
    return return_value;
#else
    return "";
#endif // WIN32
  }

  std::wstring QNMicrophoneManager::UtfToUnicode(std::string str) {
#ifdef _WIN32
    //获取缓冲区的大小，并申请空间，缓冲区大小是按字符计算的
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
    TCHAR *buffer = new(std::nothrow) TCHAR[len + 1];
    if (NULL == buffer) {
      return NULL;
    }
    //多字节编码转换成宽字节编码
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), buffer, len);
    buffer[len] = '\0';
    std::wstring return_value;
    return_value.append(buffer);
    delete[]buffer;
    return return_value;
#else
    return L"";
#endif // WIN32
  }

  int32_t QNMicrophoneManager::GetDefaultDeviceId(bool is_recording, string& device_id) {
    HRESULT hr = S_OK;
    IMMDeviceEnumerator* _ptrEnumerator = NULL;
    IMMDevice* pDevice = NULL;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
      __uuidof(IMMDeviceEnumerator),
      reinterpret_cast<void**>(&_ptrEnumerator));

    if (!SUCCEEDED(hr) || !_ptrEnumerator) {
      QN_LOG_ERROR << "CoCreateInstance failed, result:" << hr;
      return QNRTC_FAILED;
    }

    hr = _ptrEnumerator->GetDefaultAudioEndpoint(
      is_recording ? eCapture : eRender,
      eConsole, // 只获取 默认设备，不获取通信设备 
      &pDevice
    );

    if (FAILED(hr)) {
      if (pDevice) {
        pDevice->Release();
        pDevice = NULL;
      }
      return QNRTC_FAILED;
    }
    LPWSTR pwszID = NULL;
    hr = pDevice->GetId(&pwszID);
    if (hr == S_OK) {
      device_id = UnicodeToUtf(pwszID);
    }
    if (pDevice) {
      pDevice->Release();
      pDevice = NULL;
    }
    CoTaskMemFree(pwszID);
    return hr;
  }
}
