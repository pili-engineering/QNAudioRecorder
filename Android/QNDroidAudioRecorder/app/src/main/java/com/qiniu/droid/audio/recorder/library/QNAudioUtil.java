package com.qiniu.droid.audio.recorder.library;

public class QNAudioUtil {

    static final double MAX_AUDIO_LEVEL = 127;
    public static int calculateAudioLevel(
           short[] samples, int length,
           int overload) {
       /*
        * Calculate the root mean square (RMS) of the signal.
        */
       double rms = 0;

       int offset = 0;
       for (; offset < length; offset++) {
           double sample = samples[offset];

           sample /= overload;
           rms += sample * sample;
       }
       rms = (length == 0) ? 0 : Math.sqrt(rms / length);

       /*
        * The audio level is a logarithmic measure of the
        * rms level of an audio sample relative to a reference
        * value and is measured in decibels.
        */
       double db;

       /*
        * The minimum audio level permitted.
        */
        final double MIN_AUDIO_LEVEL = -127;

       /*
        * The maximum audio level permitted.
        */
       final double MAX_AUDIO_LEVEL = 0;

       if (rms > 0) {
           /*
            * The "zero" reference level is the overload level,
            * which corresponds to 1.0 in this calculation, because
            * the samples are normalized in calculating the RMS.
            */
           db = 20 * Math.log10(rms);

           /*
            * Ensure that the calculated level is within the minimum
            * and maximum range permitted.
            */
           if (db < MIN_AUDIO_LEVEL)
               db = MIN_AUDIO_LEVEL;
           else if (db > MAX_AUDIO_LEVEL)
               db = MAX_AUDIO_LEVEL;
       } else {
           db = MIN_AUDIO_LEVEL;
       }
       return (int)Math.round(db);
    }
}

