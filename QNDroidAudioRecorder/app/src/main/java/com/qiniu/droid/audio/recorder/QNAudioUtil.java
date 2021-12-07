package com.qiniu.droid.audio.recorder;

public class QNAudioUtil {

    /**
     * byte[] temp = new byte[size];
     * for (int i = 0; i < size; i++) {
     *      temp[i] = Byte.MAX_VALUE;
     * }
     * maxVolume = AudioUtil.doubleCalculateVolume(temp);
     */
    public static final double MAV_VOLUME = 39.11730073691797;

    /**
     * 计算每个 buffer 的平均音量值
     * 位宽按照 16bit 计算
     */
    public static double doubleCalculateVolume(byte[] buffer) {
        double sumVolume = 0.0;
        double avgVolume = 0.0;
        double volume = 0.0;
        for (int i = 0; i < buffer.length; i += 2) {
            int v1 = buffer[i] & 0xFF;
            int v2 = buffer[i + 1] & 0xFF;
            int temp = v1 + (v2 << 8);
            if (temp >= 0x8000) {
                temp = 0xffff - temp;
            }
            sumVolume += Math.abs(temp);
        }
        avgVolume = sumVolume / buffer.length / 2;
        volume = Math.log10(1 + avgVolume) * 10;
        return volume;
    }
}

