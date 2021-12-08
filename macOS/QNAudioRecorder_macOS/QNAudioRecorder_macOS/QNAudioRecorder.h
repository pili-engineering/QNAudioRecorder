//
//  QNAudioRecorder.h
//  QNAudioRecorder
//
//  Created by tony.jing on 2021/12/8.
//

#import <Foundation/Foundation.h>

@class QNAudioRecorder;

@protocol QNAudioRecorderDelegate <NSObject>

/**
 * 录制音量回调
 *
 * @param audioReocrder QNAudioRecorder实例
 *
 * @param volume 录制音频音量 . 范围 [0 - 1.0]
 */
- (void)audioRecorder:(QNAudioRecorder *)audioReocrder onVolumeChanged:(double)volume;

@end

@interface QNAudioRecorder : NSObject

@property (nonatomic, weak) id<QNAudioRecorderDelegate> delegate;

/**
 * 初始化方法
 *
 * @return QNAudioRecorder实例
 */
+ (QNAudioRecorder *) sharedInstance;

/**
 * 开始录制
 *
 * @return 是否成功开始录制   YES：成功    NO：失败
 */
- (BOOL)startRecording;

/**
 * 停止录制
 *
 * @return 是否成功停止录制   YES：成功    NO：失败
 */
- (BOOL)stopRecording;
@end

