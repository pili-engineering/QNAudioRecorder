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

/*!
 * @abstract 麦克风采集音频数据的回调。
 *
 * @since v1.0.0
 */
- (void)microphoneRecorder:(QNMicrophoneRecorder *)Recorder didGetAudioBuffer:(AudioBuffer *)audioBuffer asbd:(const AudioStreamBasicDescription *)asbd;

/*!
 * @abstract 麦克风采集音频音量的回调，约 100ms 左右回调一次。
 *
 * @since v1.0.0
 */
- (void)microphoneRecorder:(QNMicrophoneRecorder *)Recorder volume:(float)voulme;

@end

@interface QNMicrophoneRecorder : NSObject

/*!
 * @abstract QNMicrophoneRecorderDelegate 代理。
 *
 * @since v1.0.0
 */
@property (nonatomic, weak) id<QNMicrophoneRecorderDelegate> delegate;

/*!
 * @abstract 开始录制。
 *
 * @since v1.0.0
 */
- (void)startRecording;

/*!
 * @abstract 停止录制。
 *
 * @since v1.0.0
 */
- (void)stopRecording;

@end

NS_ASSUME_NONNULL_END
