//
//  ViewController.m
//  QNAudioRecorderDemo
//
//  Created by 冯文秀 on 2021/12/7.
//

#import "ViewController.h"
#import <QNAudioRecorder_iOS/QNAudioRecorder_iOS.h>

@interface ViewController ()<QNAudioRecorderDelegate>

@property (nonatomic, strong) NSTimer *durationTimer;
@property (nonatomic, strong) UILabel *titleLabel;
@property (nonatomic, assign) NSInteger duration;
@property (nonatomic, strong) UIButton *recordButton;

@property (nonatomic, strong) QNAudioRecorder *recorder;
@property (nonatomic, strong) UILabel *volumeLabel;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [self setupUI];
}

- (void)audioRecorder:(QNAudioRecorder *)recorder volume:(double)volume {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self setVolume:volume];
    });
}

#pragma mark - UI

- (void)setupUI {
    self.recordButton = [[UIButton alloc] initWithFrame:CGRectMake(0, 0, 100, 30)];

    self.recordButton.center = self.view.center;
    [self.recordButton setTitleColor:[UIColor greenColor] forState:UIControlStateNormal];
    [self.recordButton setTitle:@"录制" forState:UIControlStateNormal];
    [self.recordButton setTitleColor:[UIColor redColor] forState:UIControlStateSelected];
    [self.recordButton setTitle:@"停止" forState:UIControlStateSelected];
    [self.view addSubview:_recordButton];
    
    [self.recordButton addTarget:self action:@selector(recordButton:) forControlEvents:UIControlEventTouchUpInside];
    
    [self setTime:@"录制时长 00:00"];
    [self setVolume:0.0000000000];
}

- (void)setTime:(NSString *)time {
    if (nil == self.titleLabel) {
        self.titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 100, 30)];
        self.titleLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 100);
        
        if (@available(iOS 9.0, *)) {
            self.titleLabel.font = [UIFont monospacedDigitSystemFontOfSize:16 weight:(UIFontWeightRegular)];
        } else {
            self.titleLabel.font = [UIFont systemFontOfSize:16];
        }
        self.titleLabel.textAlignment = NSTextAlignmentCenter;
        self.titleLabel.textColor = [UIColor blackColor];
        [self.view addSubview:_titleLabel];
    }
    self.titleLabel.text = time;
    [self.titleLabel sizeToFit];
}

- (void)setVolume:(float)volume {
    if (nil == self.volumeLabel) {
        self.volumeLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 200, 30)];
        self.volumeLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 50);
        if (@available(iOS 9.0, *)) {
            self.volumeLabel.font = [UIFont monospacedDigitSystemFontOfSize:14 weight:(UIFontWeightRegular)];
        } else {
            self.volumeLabel.font = [UIFont systemFontOfSize:14];
        }
        self.volumeLabel.textAlignment = NSTextAlignmentCenter;
        self.volumeLabel.textColor = [UIColor blueColor];
        [self.view addSubview:_volumeLabel];
    }
    self.volumeLabel.text = [NSString stringWithFormat:@"当前音量：%.10f", volume];
    [self.volumeLabel sizeToFit];
}

#pragma mark - 连麦时长计算

- (void)startTimer {
    
    [self stoptimer];
    
    self.durationTimer = [NSTimer timerWithTimeInterval:1
                                             target:self
                                           selector:@selector(timerAction)
                                           userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:self.durationTimer forMode:NSRunLoopCommonModes];
}

- (void)timerAction {
    self.duration ++;
    NSString *str = [NSString stringWithFormat:@"录制时长 %02ld:%02ld", self.duration / 60, self.duration % 60];
    [self setTime:str];
}

- (void)stoptimer {
    if (self.durationTimer) {
        [self setTime:@"录制时长 00:00"];
        [self setVolume:0.0000000000];

        self.duration = 0;
        [self.durationTimer invalidate];
        self.durationTimer = nil;
    }
}

#pragma mark - button

- (void)recordButton:(UIButton *)button {
    button.selected = !button.selected;
    if (button.selected) {
        [self startTimer];
        QNAudioRecorder *recorder = [QNAudioRecorder start];
        if (recorder) {
            _recorder = recorder;
            if (_recorder) {
                _recorder.delegate = self;
            }
        } else {
            NSLog(@"开始录制失败！");
        }
    } else {
        [self stoptimer];
        BOOL isSuccess =[_recorder stop];
        NSLog(@"停止录制是否成功：%d", isSuccess);
    }
}

@end
