//
//  QNMicrophoneRecorder.h
//  QNAudioRecorder
//
//  Created by 冯文秀 on 2021/12/7.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

NS_ASSUME_NONNULL_BEGIN

@class QNMicrophoneRecorder;

@protocol QNMicrophoneRecorderDelegate <NSObject>

@optional

- (void)microphoneRecorder:(QNMicrophoneRecorder *)Recorder didGetAudioBuffer:(AudioBuffer *)audioBuffer asbd:(const AudioStreamBasicDescription *)asbd;
- (void)microphoneRecorder:(QNMicrophoneRecorder *)Recorder volume:(float)voulme;

@end

@interface QNMicrophoneRecorder : NSObject

@property (nonatomic, weak) id<QNMicrophoneRecorderDelegate> delegate;

- (void)startRecording;

- (void)stopRecording;

@end

NS_ASSUME_NONNULL_END
