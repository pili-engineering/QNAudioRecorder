import Loudness from './Loudness'

const VOLUME_INTERVAL = 100 // ms
let volumeIntervalID

const AudioContext = window.AudioContext || window.webkitAudioContext
const audioContext = new AudioContext({ latencyHint: 'interactive' })
const loudness = new Loudness(audioContext)

export async function start(cb) {
  clearInterval(volumeIntervalID)
  audioContext.resume()

  volumeIntervalID = setInterval(() => {
    cb(loudness.getLoudness())
  }, VOLUME_INTERVAL)
}

export function stop() {
  clearInterval(volumeIntervalID)
  loudness.release()
}
