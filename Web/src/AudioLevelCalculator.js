export default class AudioLevelCalculator {
  /**
   * Calculates the audio level of a signal with specific
   * <tt>samples</tt>.
   *
   * @param samples  the samples whose audio level we need to
   * calculate.  The samples are specified as an <tt>int</tt>
   * array starting at <tt>offset</tt>, extending <tt>length</tt>
   * number of elements, and each <tt>int</tt> element in the
   * specified range representing a sample whose audio level we\
   * need to calculate.  Though a sample is provided in the
   * form of an <tt>int</tt> value, the sample size in bits
   * is determined by the caller via <tt>overload</tt>.
   *
   * @param length  the length of the signal specified in
   * <tt>samples<tt>, starting at <tt>offset</tt>.
   *
   * @param overload  the overload (point) of <tt>signal</tt>.
   * For example, <tt>overload</tt> can be {@link Byte#MAX_VALUE}
   * for 8-bit signed samples or {@link Short#MAX_VALUE} for
   * 16-bit signed samples.
   *
   * @return  the audio level of the specified signal.
   */
  static calculateAudioLevel(samples, length, overload) {
    /*
     * Calculate the root mean square (RMS) of the signal.
     */
    let rms = 0

    for (let offset = 0; offset < length; offset++) {
      let sample = samples[offset]

      sample /= overload
      rms += sample * sample
    }
    rms = length == 0 ? 0 : Math.sqrt(rms / length)

    /*
     * The audio level is a logarithmic measure of the
     * rms level of an audio sample relative to a reference
     * value and is measured in decibels.
     */
    let db

    /*
     * The minimum audio level permitted.
     */
    const MIN_AUDIO_LEVEL = -127
    /*
     * The maximum audio level permitted.
     */
    const MAX_AUDIO_LEVEL = 0

    if (rms > 0) {
      /*
       * The "zero" reference level is the overload level,
       * which corresponds to 1.0 in this calculation, because
       * the samples are normalized in calculating the RMS.
       */
      db = 20 * Math.log10(rms)

      /*
       * Ensure that the calculated level is within the minimum
       * and maximum range permitted.
       */
      if (db < MIN_AUDIO_LEVEL) db = MIN_AUDIO_LEVEL
      else if (db > MAX_AUDIO_LEVEL) db = MAX_AUDIO_LEVEL
    } else {
      db = MIN_AUDIO_LEVEL
    }

    let level = Math.round(db)
    level = (level + 127) / 127

    return level
  }
}
