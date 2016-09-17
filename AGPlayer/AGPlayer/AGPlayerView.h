//
//  AGPlayerView.h
//  AGPlayer
//
//  Created by 吴书敏 on 16/7/14.
//  Copyright © 2016年 littledogboy. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

typedef NS_ENUM(NSInteger, TouchPlayerViewMode) {
    TouchPlayerViewModeNone, // 轻触
    TouchPlayerViewModeHorizontal, // 水平滑动
    TouchPlayerViewModeUnknow, // 未知
};


@interface AGPlayerView : UIView
{
    TouchPlayerViewMode _touchMode;
}


@property (strong, nonatomic) IBOutlet UILabel *titleLabel;

// AVPlayer 控制视频播放
@property (nonatomic, strong) AVPlayer *player;

// 播放状态
@property (nonatomic, assign) BOOL isPlaying;

// 是否横屏
@property (nonatomic, assign) BOOL isLandscape;

// 是否锁屏
@property (nonatomic, assign) BOOL isLock;

// 传入视频地址
- (void)updatePlayerWithURL:(NSURL *)url;

// 移除通知
- (void)removeObserveAndNOtification;

// 切换为横屏
- (void)setLandscapeLayout;

// 切换为竖屏
- (void)setProtraitLayout;

// 播放
- (void)play;

// 暂停
- (void)pause;

@end
