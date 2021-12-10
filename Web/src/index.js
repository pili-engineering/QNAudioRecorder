const FFT_SIZE = 2048
const AUDIO_WEIGHT_LIMIT = 0
const VOLUME_INTERVAL = 100 // ms

const AudioContext = window.AudioContext || window.webkitAudioContext
const audioContext = new AudioContext({ latencyHint: 'interactive' })
const analyserNode = audioContext.createAnalyser()
analyserNode.fftSize = FFT_SIZE
const gainNode = audioContext.createGain()
gainNode.gain.value = 0
analyserNode.connect(gainNode)
gainNode.connect(audioContext.destination)
const dataArray = new Uint8Array(analyserNode.frequencyBinCount)

let mediaStream
let mediaStreamSourceNode
let volumeIntervalID

function release() {
  if (mediaStream) {
    mediaStream.getTracks().forEach((t) => t.stop())
    mediaStream = undefined
  }
  if (mediaStreamSourceNode) {
    mediaStreamSourceNode.disconnect()
    mediaStreamSourceNode = undefined
  }
  clearInterval(volumeIntervalID)
  audioContext.suspend()
}

export async function start(cb) {
  release()

  mediaStream = await navigator.mediaDevices.getUserMedia({ audio: true })
  mediaStreamSourceNode = audioContext.createMediaStreamSource(mediaStream)
  mediaStreamSourceNode.connect(analyserNode)
  audioContext.resume()

  volumeIntervalID = setInterval(() => {
    analyserNode.getByteFrequencyData(dataArray)
    cb(computeVolume())
  }, VOLUME_INTERVAL)
}

export function stop() {
  release()
}

function aWeight(f) {
  const f2 = f * f
  return (
    (1.2588966 * 148840000 * f2 * f2) /
    ((f2 + 424.36) *
      Math.sqrt((f2 + 11599.29) * (f2 + 544496.41)) *
      (f2 + 148840000))
  )
}

function computeVolume() {
  let sum = 0
  let length = dataArray.length
  dataArray.forEach((d, i) => {
    const frequency = (i * (audioContext.sampleRate || 44100)) / length
    if (frequency > 22050) {
      length -= 1
      return
    }
    const weight = (aWeight(frequency) * d) / 255.0
    if (weight <= AUDIO_WEIGHT_LIMIT) {
      length -= 1
      return
    }
    sum += weight * weight
  })

  return length === 0 ? 0 : Math.sqrt(sum / length)
}
