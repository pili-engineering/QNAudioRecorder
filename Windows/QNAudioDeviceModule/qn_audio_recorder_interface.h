#ifndef QN_AUIDO_RECORD_INTERFACE_H
#define QN_AUIDO_RECORD_INTERFACE_H

#include <string>
#include <stdint.h>


namespace qiniu {

#ifdef __QINIU_DLL_EXPORT_ 
#define QINIU_EXPORT_DLL __declspec(dllexport)
#else
#define QINIU_EXPORT_DLL __declspec(dllimport)
#endif

#define QNRTC_OK 0
#define QNRTC_FAILED -1
#define Err_Internal_Null_Pointer 11001
#define Err_Failed_Set_Recorder_Device 11007

  // 音频设备信息 
  struct QNAudioDeviceInfo {
    std::string id;                   // 设备 ID，用于系统内做标识
    std::string name;                 // 设备名，用于展示给用户，较友好
    bool is_default;                  // 此设备是否为默认设备
  };

  class QINIU_EXPORT_DLL QNAudioRecorder {
   public:
     class QNAudioVolumeCallback {
      public:
        // 音量值范围 [0 - 1.0]
       virtual void OnVolumeChanged(double volume) {};
      protected:
       ~QNAudioVolumeCallback() {};
     };

   public:
     /**
      * 用于获取麦克风实例指针
      * @param listener, 注册音量监听回调接口
      */
     static QNAudioRecorder* CreateAudioRecorder(QNAudioVolumeCallback* listener);

     /**
      * 释放由 ObtainMicrophoneInterface 返回的指针
      */
     static void DestroyAudioRecorder(QNAudioRecorder* interface_ptr);

     /**
      * 用于获取音频采集设备数量
      *
      * @return 音频采集设备的数量
      */
     virtual int32_t GetAudioRecordingDeviceCount() = 0;

     /**
      * 获取指定 index 音频采集设备信息
      *
      * @param index，<= GetAudioRecordingDeviceCount()
      * @param audio_info 输出参数，用于返回指定的音频设备信息
      */
     virtual QNAudioDeviceInfo& GetAudioRecordingDeviceInfo(int32_t index) = 0;

     /**
      * 设置连麦使用的音频录制设备
      *
      * @param index，<= GetAudioPlaybackDeviceCount()
      *
      * @return 成功返回 0，其它请参考错误码列表
      */
     virtual int32_t SetAudioRecordingDevice(int32_t index) = 0;

     /**
      * 开启麦克风采集
      *
      * @return 成功返回 0，其它请参考错误码列表
      */
     virtual int32_t StartRecording() = 0;

     /**
      * 关闭麦克风采集
      *
      * @return 成功返回 0，其它请参考错误码列表
      */
     virtual int32_t StopRecording() = 0;

   protected:
    ~QNAudioRecorder() {};
  };
}

#endif // QN_AUIDO_RECORD_INTERFACE_H
