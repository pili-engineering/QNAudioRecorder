//
//  QNCommon.m
//  QNAudioRecorder
//
//  Created by 冯文秀 on 2021/12/7.
//

#import "QNCommon.h"
#define kQNMixVolume 87.2984313

@implementation QNCommon

+ (void)scaleWithSat:(AudioBuffer *)audioBuffer scale:(double)scale max:(float)max min:(float) min {
    @autoreleasepool {
        if (audioBuffer->mDataByteSize == 0) {
            return;
        }
        
        if (scale > max) {
            scale = max;
        }
        if (scale < min) {
            scale = min;
        }
        
        short bufferByte[audioBuffer->mDataByteSize/2];
        memcpy(bufferByte, audioBuffer->mData, audioBuffer->mDataByteSize);
        
        // 将 buffer 内容取出，乘以 scale
        for (int i = 0; i < audioBuffer->mDataByteSize/2; i++) {
            bufferByte[i] = bufferByte[i]*scale;
        }
        memcpy(audioBuffer->mData, bufferByte, audioBuffer->mDataByteSize);
    }
}

+ (float)volumeWithAudioBuffer:(AudioBuffer *)audioBuffer {
    if (audioBuffer->mDataByteSize == 0) {
        return 0.0;
    }
    
    long long pcmAllLenght = 0;
    short bufferByte[audioBuffer->mDataByteSize/2];
    memcpy(bufferByte, audioBuffer->mData, audioBuffer->mDataByteSize);
    
    // 将 buffer 内容取出，进行平方和运算
    for (int i = 0; i < audioBuffer->mDataByteSize/2; i++) {
        pcmAllLenght += bufferByte[i] * bufferByte[i];
    }
    // 平方和除以数据总长度，得到音量大小。
    float mean = pcmAllLenght / (double)audioBuffer->mDataByteSize;
    float volume = 0.0;
    if (mean != 0) {
        volume =10 * log10(mean);
    }
    
    return abs(volume)/kQNMixVolume;
}
@end
