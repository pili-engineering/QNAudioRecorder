//
//  QNMicrophoneRecorder.m
//  QNAudioRecorder
//
//  Created by 冯文秀 on 2021/12/7.
//

#import "QNMicrophoneRecorder.h"
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIKit.h>
#import "QNCommon.h"

#ifdef DEBUG
    #define NSLog NSLog
#else
    #define NSLog(...);
#endif

const NSInteger kQNAudioCaptureSampleRate = 48000;

@interface QNMicrophoneRecorder ()
@property (nonatomic, assign) BOOL muted;
@property (nonatomic, assign) BOOL allowAudioMixWithOthers;
@property (nonatomic, assign) float microphoneInputGain;
@property (nonatomic, assign) NSInteger numberOfChannels;

@property (nonatomic, assign) AudioComponentInstance componentInstance;
@property (nonatomic, assign) AudioComponent component;
@property (nonatomic, strong) dispatch_queue_t taskQueue;
@property (nonatomic, assign) BOOL isRunning;
@property (nonatomic, assign) AudioStreamBasicDescription asbd;
@property (nonatomic, assign) BOOL isInInterruption;

@property (nonatomic, assign) NSInteger count;

@end

@implementation QNMicrophoneRecorder

- (instancetype)init {
    if(self = [super init]) {
        NSLog(@"QNMicrophoneRecorder init: %p", self);
        self.isRunning = NO;
        self.taskQueue = dispatch_queue_create("com.qiniu.qrd.audiocapture", NULL);
        self.numberOfChannels = 1;
        self.microphoneInputGain = 1.0;
        self.count = 0;

        [self setupASBD];
        [self setupAudioComponent];
        [self setupAudioSession];
        [self addObservers];
    }
    return self;
}

- (void)setupASBD {
    _asbd.mSampleRate = kQNAudioCaptureSampleRate;
    _asbd.mFormatID = kAudioFormatLinearPCM;
    _asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
    _asbd.mChannelsPerFrame = 1;
    _asbd.mFramesPerPacket = 1;
    _asbd.mBitsPerChannel = 16;
    _asbd.mBytesPerFrame = _asbd.mBitsPerChannel / 8 * _asbd.mChannelsPerFrame;
    _asbd.mBytesPerPacket = _asbd.mBytesPerFrame * _asbd.mFramesPerPacket;
}

- (void)setupAudioComponent {
    AudioComponentDescription acd;
    acd.componentType = kAudioUnitType_Output;
    acd.componentSubType = kAudioUnitSubType_RemoteIO;
    acd.componentManufacturer = kAudioUnitManufacturer_Apple;
    acd.componentFlags = 0;
    acd.componentFlagsMask = 0;
    
    self.component = AudioComponentFindNext(NULL, &acd);
    OSStatus status = AudioComponentInstanceNew(self.component, &_componentInstance);
    if (noErr != status) {
        NSLog(@"AudioComponentInstanceNew error, status: %d", status);
        return;
    }
    
    UInt32 flagOne = 1;
    AudioUnitSetProperty(self.componentInstance, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &flagOne, sizeof(flagOne));
    
    AURenderCallbackStruct cb;
    cb.inputProcRefCon = (__bridge void *)(self);
    cb.inputProc = handleInputBuffer;
    AudioUnitSetProperty(self.componentInstance, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &_asbd, sizeof(_asbd));
    AudioUnitSetProperty(self.componentInstance, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 1, &cb, sizeof(cb));
    
    status = AudioUnitInitialize(self.componentInstance);
    if (noErr != status) {
        NSLog(@"AudioUnitInitialize error, status: %d", status);
        return;
    }
}

- (void)setupAudioSession {
    NSError *sessionError = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    
    if (![self resetAudioSessionCategorySettings]) {
        return;
    }
    
    [session setMode:AVAudioSessionModeVideoChat error:&sessionError];
    if (sessionError) {
        NSLog(@"error:%ld, set session mode error : %@", sessionError.code, sessionError.localizedDescription);
        return;
    }
    
    [session setActive:YES error:&sessionError];
    if (sessionError) {
        NSLog(@"error:%ld, set session active error : %@", sessionError.code, sessionError.localizedDescription);
        return;
    }
    
    [session setPreferredSampleRate:kQNAudioCaptureSampleRate error:&sessionError];
    if (sessionError) {
        NSLog(@"error:%ld, setPreferredSampleRate error : %@", sessionError.code, sessionError.localizedDescription);
        return;
    }
    
    [session setPreferredIOBufferDuration:1024.0 / kQNAudioCaptureSampleRate error:&sessionError];
    if (sessionError) {
        NSLog(@"error:%ld, setPreferredIOBufferDuration error : %@", sessionError.code, sessionError.localizedDescription);
        return;
    }
    
    // use bottom microphone for capture by default
    if (AVAudioSessionOrientationBottom != [session inputDataSource].orientation) {
        for (AVAudioSessionDataSourceDescription *dataSource in [session inputDataSources]) {
            if (AVAudioSessionOrientationBottom == dataSource.orientation) {
                [session setInputDataSource:dataSource error:&sessionError];
                if (sessionError) {
                    NSLog(@"error:%ld, set input data source error : %@", sessionError.code, sessionError.localizedDescription);
                }
            }
        }
    }
    
    return;
}

- (void)addObservers {
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleRouteChange:)
                                                 name: AVAudioSessionRouteChangeNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleInterruption:)
                                                 name: AVAudioSessionInterruptionNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleApplicationActive:)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
}

- (BOOL)resetAudioSessionCategorySettings {
    NSError *sessionError = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    
    AVAudioSessionCategoryOptions options = AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth;
    
    if (self.allowAudioMixWithOthers) {
        options = AVAudioSessionCategoryOptionMixWithOthers | AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth;
    }
    
    [session setCategory:AVAudioSessionCategoryPlayAndRecord withOptions:options error:&sessionError];
    if (sessionError) {
        NSLog(@"error:%ld, set session category error : %@", sessionError.code, sessionError.localizedDescription);
        return NO;
    }
    
    return YES;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    AudioOutputUnitStop(self.componentInstance);
    AudioComponentInstanceDispose(self.componentInstance);
    self.componentInstance = nil;
    self.component = nil;
    NSLog(@"QNMicrophoneRecorder dealloc: %p", self);
}

- (void)startRecording {
    NSLog(@"startRecording");
    
    dispatch_async(self.taskQueue, ^{
        if (self.isRunning) {
            return;
        }
        
        if ([self resetAudioSession]) {
            OSStatus result = AudioOutputUnitStart(self.componentInstance);
            NSLog(@"AudioOutputUnitStart result: %ld", (long)result);
            self.isRunning = YES;
        }
    });
}

- (void)stopRecording {
    NSLog(@"stopRecording");
    
    dispatch_async(self.taskQueue, ^{
        if (!self.isRunning) {
            return;
        }
        
        OSStatus result = AudioOutputUnitStop(self.componentInstance);
        NSLog(@"AudioOutputUnitStop result: %ld", (long)result);
        self.isRunning = NO;
    });
}

#pragma mark - NSNotification
- (void)handleRouteChange:(NSNotification *)notification {
    NSString* seccReason = @"";
    NSInteger  reason = [[[notification userInfo] objectForKey:AVAudioSessionRouteChangeReasonKey] integerValue];
    switch (reason) {
        case AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory:
            [self resetAudioSession];
            seccReason = @"The route changed because no suitable route is now available for the specified category.";
            break;
        case AVAudioSessionRouteChangeReasonWakeFromSleep:
            [self resetAudioSession];
            seccReason = @"The route changed when the device woke up from sleep.";
            break;
        case AVAudioSessionRouteChangeReasonOverride:
            [self resetAudioSession];
            seccReason = @"The output route was overridden by the app.";
            break;
        case AVAudioSessionRouteChangeReasonCategoryChange:
            seccReason = @"The category of the session object changed.";
            break;
        case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
            [self resetAudioSession];
            seccReason = @"The previous audio output path is no longer available.";
            break;
        case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
            [self resetAudioSession];
            seccReason = @"A preferred new audio output path is now available.";
            break;
        case AVAudioSessionRouteChangeReasonUnknown:
        default:
            seccReason = @"The reason for the change is unknown.";
            break;
    }
    
    NSLog(@"handleRouteChange: %@ reason %@",[notification name], seccReason);
}

- (BOOL)resetAudioSession {
    NSError *sessionError = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    
    [session setActive:YES error:&sessionError];
    if (sessionError) {
        NSLog(@"error:%ld, set session active error : %@", sessionError.code, sessionError.localizedDescription);
        return NO;
    }
    
    // use bottom microphone for capture by default
    if (AVAudioSessionOrientationBottom != [session inputDataSource].orientation) {
        for (AVAudioSessionDataSourceDescription *dataSource in [session inputDataSources]) {
            if (AVAudioSessionOrientationBottom == dataSource.orientation) {
                [session setInputDataSource:dataSource error:&sessionError];
                if (sessionError) {
                    NSLog(@"error:%ld, set input data source error : %@", sessionError.code, sessionError.localizedDescription);
                }
            }
        }
    }
    
    return YES;
}

- (void)handleInterruption:(NSNotification *)notification {
    NSLog(@"handleInterruption: %@" ,notification);
    if (![notification.name isEqualToString:AVAudioSessionInterruptionNotification]) {
        return;
    }
    
    NSInteger interruptionType = [[[notification userInfo] objectForKey:AVAudioSessionInterruptionTypeKey] integerValue];
    if (interruptionType == AVAudioSessionInterruptionTypeBegan) {
        dispatch_sync(self.taskQueue, ^{
            if (self.isRunning) {
                OSStatus result = AudioOutputUnitStop(self.componentInstance);
                NSLog(@"AVAudioSessionInterruptionTypeBegan, AudioOutputUnitStop, result: %d", result);
                self.isInInterruption = YES;
            }
        });
    }
    else if (interruptionType == AVAudioSessionInterruptionTypeEnded) {
        dispatch_async(self.taskQueue, ^{
            if (self.isRunning) {
                OSStatus result = AudioOutputUnitStart(self.componentInstance);
                NSLog(@"AVAudioSessionInterruptionTypeEnded, AudioOutputUnitStart, result: %d", result);
                self.isInInterruption = NO;
            }
        });
    }
}

- (void)handleApplicationActive:(NSNotification *)notification {
    dispatch_async(self.taskQueue, ^{
        if (self.isInInterruption && self.isRunning) {
            OSStatus result = AudioOutputUnitStart(self.componentInstance);
            NSLog(@"applicationActive, AudioOutputUnitStart, result: %d", result);
            self.isInInterruption = NO;
        }
    });
}

#pragma mark - CallBack
static OSStatus handleInputBuffer(void *inRefCon,
                                  AudioUnitRenderActionFlags *ioActionFlags,
                                  const AudioTimeStamp *inTimeStamp,
                                  UInt32 inBusNumber,
                                  UInt32 inNumberFrames,
                                  AudioBufferList *ioData) {
    @autoreleasepool {
        QNMicrophoneRecorder *source = (__bridge QNMicrophoneRecorder *)inRefCon;
        if (!source) {
            return -1;
        }

        AudioBuffer buffer;
        buffer.mDataByteSize = inNumberFrames * 2 * source.numberOfChannels;
        buffer.mData = malloc(buffer.mDataByteSize);
        buffer.mNumberChannels = source.numberOfChannels;

        AudioBufferList bufferList;
        bufferList.mNumberBuffers = 1;
        bufferList.mBuffers[0] = buffer;

        OSStatus status = AudioUnitRender(source.componentInstance,
                                          ioActionFlags,
                                          inTimeStamp,
                                          inBusNumber,
                                          inNumberFrames,
                                          &bufferList);

        if (status || buffer.mDataByteSize <= 0) {
            NSLog(@"AudioUnitRender error, status: %d", status);
            free(buffer.mData);
            return status;
        }

        if (source.muted) {
            memset(buffer.mData, 0, buffer.mDataByteSize);
        }
        
        if (source.microphoneInputGain != 1.0) {
            [QNCommon scaleWithSat:&buffer scale:source.microphoneInputGain max:10.0 min:0.0];
        }

        if (source.delegate && [source.delegate respondsToSelector:@selector(microphoneRecorder:didGetAudioBuffer:asbd:)]) {
            AudioStreamBasicDescription *asbd = calloc(1, sizeof(AudioStreamBasicDescription));
            memcpy(asbd, &source->_asbd, sizeof(AudioStreamBasicDescription));
            [source.delegate microphoneRecorder:source didGetAudioBuffer:&buffer asbd:asbd];
            free(asbd);
        }
        
        source.count++;
        if (source.count >= 5) {
            source.count = 0;
            float volume = [QNCommon volumeWithAudioBuffer:&buffer];
            if (source.delegate && [source.delegate respondsToSelector:@selector(microphoneRecorder:volume:)]) {
                [source.delegate microphoneRecorder:source volume:volume];
            }
        }

        free(buffer.mData);
        return status;
    }
}

@end