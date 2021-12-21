/**
 * @reference https://github.com/LLK/scratch-audio/blob/a049deba23013ced81ce21b1ba6ae2c4e9714eb3/src/Loudness.js
 */
export default class Loudness {
  /**
   * Instrument and detect a loudness value from a local microphone.
   * @param {AudioContext} audioContext - context to create nodes from for
   *     detecting loudness
   * @constructor
   */
  constructor(audioContext) {
    /**
     * AudioContext the mic will connect to and provide analysis of
     * @type {AudioContext}
     */
    this.audioContext = audioContext

    /**
     * Are we connecting to the mic yet?
     * @type {Boolean}
     */
    this.connectingToMic = false

    /**
     * microphone, for measuring loudness, with a level meter analyzer
     * @type {MediaStreamSourceNode}
     */
    this.mic = null
  }

  /**
   * Get the current loudness of sound received by the microphone.
   * Sound is measured in RMS and smoothed.
   * Some code adapted from Tone.js: https://github.com/Tonejs/Tone.js
   * @return {number} loudness scaled 0 to 1
   */
  getLoudness() {
    // The microphone has not been set up, so try to connect to it
    if (!this.mic && !this.connectingToMic) {
      this.connectingToMic = true // prevent multiple connection attempts
      navigator.mediaDevices
        .getUserMedia({ audio: true })
        .then((stream) => {
          this.audioStream = stream
          this.mic = this.audioContext.createMediaStreamSource(stream)
          this.analyser = this.audioContext.createAnalyser()
          this.mic.connect(this.analyser)
          this.micDataArray = new Float32Array(this.analyser.fftSize)
        })
        .catch((err) => {
          console.warn(err)
        })
    }

    // If the microphone is set up and active, measure the loudness
    if (this.mic && this.audioStream && this.audioStream.active) {
      this.analyser.getFloatTimeDomainData(this.micDataArray)
      let sum = 0
      // compute the RMS of the sound
      for (let i = 0; i < this.micDataArray.length; i++) {
        sum += Math.pow(this.micDataArray[i], 2)
      }
      let rms = Math.sqrt(sum / this.micDataArray.length)
      // smooth the value, if it is descending
      if (this._lastValue) {
        rms = Math.max(rms, this._lastValue * 0.6)
      }
      this._lastValue = rms

      // Scale the measurement so it's more sensitive to quieter sounds
      rms *= 1.63
      rms = Math.sqrt(rms)
      // Scale it up to 0-100 and round
      rms = Math.round(rms * 100)
      // Prevent it from going above 100
      rms = Math.min(rms, 100)

      rms = rms / 100
      return rms
    }

    // if there is no microphone input, return 0
    return 0
  }

  release() {
    this.connectingToMic = false
    if (this.mic) {
      this.mic.disconnect()
      this.mic = null
    }
    if (this.audioStream) {
      this.audioStream.getTracks().forEach((t) => t.stop())
      this.audioStream = undefined
    }
    if (this.analyser) {
      this.analyser.disconnect()
      this.analyser = undefined;
    }
  }
}
