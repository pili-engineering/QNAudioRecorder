import AudioLevelCalculator from './AudioLevelCalculator'

const BUFFER_SIZE = 4096
const CHANNEL_COUNT = 1

const AudioContext = window.AudioContext || window.webkitAudioContext
const audioContext = new AudioContext({ latencyHint: 'interactive' })
const scriptNode = audioContext.createScriptProcessor(
  BUFFER_SIZE,
  CHANNEL_COUNT,
  CHANNEL_COUNT,
)
const gainNode = audioContext.createGain()
gainNode.gain.value = 0
scriptNode.connect(gainNode)
gainNode.connect(audioContext.destination)

let mediaStream
let mediaStreamSourceNode

function release() {
  if (mediaStreamSourceNode) {
    mediaStreamSourceNode.disconnect()
    mediaStreamSourceNode = undefined
  }
  if (mediaStream) {
    mediaStream.getTracks().forEach((t) => t.stop())
    mediaStream = undefined
  }
  audioContext.suspend()
}

export async function start(cb) {
  release()

  mediaStream = await navigator.mediaDevices.getUserMedia({ audio: true })
  mediaStreamSourceNode = audioContext.createMediaStreamSource(mediaStream)
  mediaStreamSourceNode.connect(scriptNode)
  scriptNode.onaudioprocess = (e) => {
    let samples = e.inputBuffer.getChannelData(0)
    samples = Int16Array.from(
      samples.map((x) => (x > 0 ? x * 0x7fff : x * 0x8000)),
    )
    let level = AudioLevelCalculator.calculateAudioLevel(
      samples,
      BUFFER_SIZE,
      32767,
    )
    cb(level)
  }
  audioContext.resume()
}

export function stop() {
  release()
}
