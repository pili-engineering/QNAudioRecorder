package com.qiniu.droid.audio.recorder.library;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.os.Build;
import android.os.Process;
import android.util.Log;

import java.nio.ByteBuffer;

public class QNAudioRecorder {
  private static final String TAG = "QNAudioRecorder";

  // Requested size of each recorded buffer provided to the client.
  private static final int CALLBACK_BUFFER_SIZE_MS = 10;

  // Average number of callbacks per second.
  private static final int BUFFERS_PER_SECOND = 1000 / CALLBACK_BUFFER_SIZE_MS;

  // We ask for a native buffer size of BUFFER_SIZE_FACTOR * (minimum required
  // buffer size). The extra space is allocated to guard against glitches under
  // high load.
  private static final int BUFFER_SIZE_FACTOR = 2;

  // The AudioRecordJavaThread is allowed to wait for successful call to join()
  // but the wait times out afther this amount of time.
  private static final long AUDIO_RECORD_THREAD_JOIN_TIMEOUT_MS = 2000;

  public static final int DEFAULT_AUDIO_SOURCE = AudioSource.MIC;

  // Default audio data format is PCM 16 bit per sample.
  // Guaranteed to be supported by all devices.
  public static final int DEFAULT_AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;

  private static final int DEFAULT_SAMPLE_RATE_HZ = 16000;

  private static final int DEFAULT_CHANNEL = 1;

  private static final int DEFAULT_VOLUME_UPDATE_FREQUENCY = 10;

  private int audioSource;
  private int audioFormat;
  private QNAudioVolumeCallback volumeCallback;

  private ByteBuffer byteBuffer;
  private AudioRecord audioRecord;
  private AudioRecordThread audioThread;

  public interface QNAudioVolumeCallback {
    void onVolumeChanged(double volume);
 }

  private class AudioRecordThread extends Thread {
    private volatile boolean keepAlive = true;
    private int count;

    public AudioRecordThread(String name) {
      super(name);
    }

    @Override
    public void run() {
      Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
      assertTrue(audioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING);

      long lastTime = System.nanoTime();
      while (keepAlive) {
        int bytesRead = audioRecord.read(byteBuffer, byteBuffer.capacity());
        if (bytesRead == byteBuffer.capacity()) {
          // It's possible we've been shut down during the read, and stopRecording() tried and
          // failed to join this thread. To be a bit safer, try to avoid calling any native methods
          // in case they've been unregistered after stopRecording() returned.
          if (keepAlive && volumeCallback != null) {
            if (++count == DEFAULT_VOLUME_UPDATE_FREQUENCY) {
              count = 0;
              byte[] temp = new byte[bytesRead];
              byteBuffer.rewind();
              byteBuffer.get(temp, 0, bytesRead);
              double volume = QNAudioUtil.doubleCalculateVolume(temp);
              volumeCallback.onVolumeChanged(volume / QNAudioUtil.MAV_VOLUME);
            }
          }
        } else {
          String errorMessage = "AudioRecord.read failed: " + bytesRead;
          Log.e(TAG, errorMessage);
          if (bytesRead == AudioRecord.ERROR_INVALID_OPERATION) {
            keepAlive = false;
          }
        }
      }

      try {
        if (audioRecord != null) {
          audioRecord.stop();
        }
      } catch (IllegalStateException e) {
        Log.e(TAG, "AudioRecord.stop failed: " + e.getMessage());
      }
    }

    // Stops the inner thread loop and also calls AudioRecord.stop().
    // Does not block the calling thread.
    public void stopThread() {
      Log.d(TAG, "stopThread");
      keepAlive = false;
    }
  }

  /**
   * QNAudioRecorder 构造方法
   *
   * @param volumeCallback 音量回调接口
   */
  public QNAudioRecorder(QNAudioVolumeCallback volumeCallback) {
    this.volumeCallback = volumeCallback;
    this.audioSource = DEFAULT_AUDIO_SOURCE;
    this.audioFormat = DEFAULT_AUDIO_FORMAT;
  }

  private int initRecording(int sampleRate, int channels) {
    Log.d(TAG, "initRecording(sampleRate=" + sampleRate + ", channels=" + channels + ")");
    if (audioRecord != null) {
      Log.e(TAG, "InitRecording called twice without StopRecording.");
      return -1;
    }
    final int bytesPerFrame = channels * getBytesPerSample(audioFormat);
    final int framesPerBuffer = sampleRate / BUFFERS_PER_SECOND;
    byteBuffer = ByteBuffer.allocateDirect(bytesPerFrame * framesPerBuffer);
    if (!(byteBuffer.hasArray())) {
      Log.e(TAG,"ByteBuffer does not have backing array.");
      return -1;
    }
    Log.d(TAG, "byteBuffer.capacity: " + byteBuffer.capacity());

    // Get the minimum buffer size required for the successful creation of
    // an AudioRecord object, in byte units.
    // Note that this size doesn't guarantee a smooth recording under load.
    final int channelConfig = channelCountToConfiguration(channels);
    int minBufferSize = AudioRecord.getMinBufferSize(sampleRate, channelConfig, audioFormat);
    if (minBufferSize == AudioRecord.ERROR || minBufferSize == AudioRecord.ERROR_BAD_VALUE) {
      Log.e(TAG,"AudioRecord.getMinBufferSize failed: " + minBufferSize);
      return -1;
    }
    Log.d(TAG, "AudioRecord.getMinBufferSize: " + minBufferSize);

    // Use a larger buffer size than the minimum required when creating the
    // AudioRecord instance to ensure smooth recording under load. It has been
    // verified that it does not increase the actual recording latency.
    int bufferSizeInBytes = Math.max(BUFFER_SIZE_FACTOR * minBufferSize, byteBuffer.capacity());
    Log.d(TAG, "bufferSizeInBytes: " + bufferSizeInBytes);
    try {
      audioRecord =
          new AudioRecord(audioSource, sampleRate, channelConfig, audioFormat, bufferSizeInBytes);
    } catch (IllegalArgumentException e) {
      Log.e(TAG,"AudioRecord ctor error: " + e.getMessage());
      releaseAudioResources();
      return -1;
    }
    if (audioRecord == null || audioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
      Log.e(TAG, "Failed to create a new AudioRecord instance");
      releaseAudioResources();
      return -1;
    }
    logMainParameters();
    logMainParametersExtended();
    return framesPerBuffer;
  }

  /**
   * 开始音频采集
   *
   * @return 开始是否成功
   */
  public boolean startRecording() {
    Log.d(TAG, "startRecording");
    if (audioRecord == null) {
      initRecording(DEFAULT_SAMPLE_RATE_HZ, DEFAULT_CHANNEL);
    }

    assertTrue(audioRecord != null);
    assertTrue(audioThread == null);
    try {
      audioRecord.startRecording();
    } catch (IllegalStateException e) {
      Log.e(TAG, "AudioRecord.startRecording failed: " + e.getMessage());
      return false;
    }
    if (audioRecord.getRecordingState() != AudioRecord.RECORDSTATE_RECORDING) {
      Log.e(TAG, "AudioRecord.startRecording failed - incorrect state :"
              + audioRecord.getRecordingState());
      return false;
    }
    audioThread = new AudioRecordThread("AudioRecordJavaThread");
    audioThread.start();
    return true;
  }

  /**
   * 停止采集接口
   *
   * @return 停止是否成功
   */
  public boolean stopRecording() {
    Log.d(TAG, "stopRecording");
    assertTrue(audioThread != null);
    audioThread.stopThread();
    if (!ThreadUtils.joinUninterruptibly(audioThread, AUDIO_RECORD_THREAD_JOIN_TIMEOUT_MS)) {
      Log.e(TAG, "Join of AudioRecordJavaThread timed out");
    }
    audioThread = null;
    releaseAudioResources();
    return true;
  }

  private void logMainParameters() {
    Log.d(TAG, "AudioRecord: "
            + "session ID: " + audioRecord.getAudioSessionId() + ", "
            + "channels: " + audioRecord.getChannelCount() + ", "
            + "sample rate: " + audioRecord.getSampleRate());
  }

  private void logMainParametersExtended() {
    if (Build.VERSION.SDK_INT >= 23) {
      Log.d(TAG,
          "AudioRecord: "
              // The frame count of the native AudioRecord buffer.
              + "buffer size in frames: " + audioRecord.getBufferSizeInFrames());
    }
  }

  // Helper method which throws an exception  when an assertion has failed.
  private static void assertTrue(boolean condition) {
    if (!condition) {
      throw new AssertionError("Expected condition to be true");
    }
  }

  private int channelCountToConfiguration(int channels) {
    return (channels == 1 ? AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_IN_STEREO);
  }

  // Releases the native AudioRecord resources.
  private void releaseAudioResources() {
    Log.d(TAG, "releaseAudioResources");
    if (audioRecord != null) {
      audioRecord.release();
      audioRecord = null;
    }
  }

  // Reference from Android code, AudioFormat.getBytesPerSample. BitPerSample / 8
  // Default audio data format is PCM 16 bits per sample.
  // Guaranteed to be supported by all devices
  private static int getBytesPerSample(int audioFormat) {
    switch (audioFormat) {
      case AudioFormat.ENCODING_PCM_8BIT:
        return 1;
      case AudioFormat.ENCODING_PCM_16BIT:
      case AudioFormat.ENCODING_IEC61937:
      case AudioFormat.ENCODING_DEFAULT:
        return 2;
      case AudioFormat.ENCODING_PCM_FLOAT:
        return 4;
      case AudioFormat.ENCODING_INVALID:
      default:
        throw new IllegalArgumentException("Bad audio format " + audioFormat);
    }
  }
}
