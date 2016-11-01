# 基于 AVPlayer 自定义播放器

如果我只是简单的播放一个视频，而不需要考虑播放器的界面。iOS9.0 之前使用 `MPMoviePlayerController `, 或者内部自带一个 view 的 `MPMoviePlayerViewController `.  iOS9.0 之后，可以使用 ` AVPictureInPictureController`, ` AVPlayerViewController`, 或者 `WKWebView`。 

以上系统提供的播放器由于高度的封装性， 使得自定义播放器变的很难。 所以，如果我需要自定义播放器样式的时候，可以使用 `AVPlayer`。 AVPlayer 存在于 AVFoundtion 中，更接近于底层，也更加灵活。

![](http://ww3.sinaimg.cn/large/650c943bgw1f7vm4unkqij20g00ag74v)

### Representing and Using Media with AVFoundation
AVFoundtion 框架中主要使用 `AVAsset` 类来展示媒体信息，比如： title， duration， size 等等。  

* AVAsset : 存储媒体信息的一个抽象类，不能直接使用。  
* AVURLAsset : AVAsset 的一个子类，使用 URL 进行实例化，实例化对象包换 URL 对应视频资源的所有信息。  
* AVPlayerItem :  有一个属性为 asset。起到观察和管理视频信息的作用。 比如，asset, tracks ， status， duration ，loadedTimeRange 等。 

我的理解是， AVPlayItem 相当于 Model 层，包含 Media 的信息和播放状态，并提供这些数据给视频观察者 比如：属性 `asset` ，URL视频的信息. `loadedTimeRanges` ，已缓冲进度。


## AVPlayerItem 使用

### 1. 初始化 
`playerItemWithURL` 或者 `initWithURL:`

在使用 AVPlayer 播放视频时，提供视频信息的是 AVPlayerItem，一个 AVPlayerItem 对应着一个URL视频资源。

初始化一个 AVPlayItem 对象后，其属性并不是马上就可以使用。我们必须确保 AVPlayerItem 已经被加载好了，可以播放了，才能使用。 毕竟凡是和网络扯上关系的都需要时间去加载。 那么，什么时候属性才能正常使用呢。 官方文档给出了解决方案：    
1. 直到 AVPlayerItem 的 `status` 属性为 `AVPlayerItemStatusReadyToPlay`.  
2. 使用 KVO 键值观察者，其属性。 

因此我们在使用的时候，使用 URL 初始化 AVPlayerItem 后，还要给它添加观察者。 

### 2. 添加观察者

AVPlayreItem 的属性需要当 status 为 ReadyToPlay 的时候才可以正常使用。 

**观察status属性**

```
[_playerItem addObserver:self forKeyPath:@"status" options:(NSKeyValueObservingOptionNew) context:nil]; // 观察status属性，
```

**观察loadedTimeRanges**

如果想做缓冲进度条，显示当前视频的缓存进度，则需要观察 `loadedTimeRanges `.

```
[_playerItem addObserver:self forKeyPath:@"loadedTimeRanges" options:NSKeyValueObservingOptionNew context:nil]; // 观察缓冲进度
```


## AVPlayer & AVPlayerLayer

**AVPlayer创建方式**
AVPlayer 有三种创建方式： 

`init`,`initWithURL:`,`initWithPlayerItem:` （URL，Item遍历构造器方法）

使用 AVPlayer 时需要注意，AVPlayer 本身并不能显示视频， 显示视频的是 AVPlayerLayer。 AVPlayerLayer 继承自 CALayer，添加到 view.layer 上就可以使用了。

**AVPlayerLayer创建方式**

```
AVPlayerLayer *playerLayer = [AVPlayerLayer playerLayerWithPlayer:player];
[superlayer addSublayer:playerLayer];
```

AVPlayerLayer 显示视频，AVPlayerItem 提供视频信息， AVPlayer 管理和调控。 这是不是非常熟悉。 我觉得这里也体现了 MVC 的思想（虽然AVPlayer继承自NSObject）， 把响应层， 显示层， 信息层， 三层分离了。 明确了每层做的任务，使用起来就会更加得心应手。

使用 AVPlayer 的核心，在于 AVPlayer 和 AVPlayerItem， AVPlayerLayer 添加到视图的layer 上后，就没有什么事儿了。 思考一下，整个播放视频的步骤。 

1. 首先，得到视频的URL  
2. 根据URL创建AVPlayerItem 
3. 把AVPlayerItem 提供给 AVPlayer 
4. AVPlayerLayer 显示视频。 
5. AVPlayer 控制视频， 播放， 暂停， 拖动 等等。
6. 播放过程中获取缓冲进度，获取播放进度。  
7. 视频播放完成后做些什么，是暂停还是循环播放，还是获取最后一帧图像。

## 播放步骤

### 1. 布局页面，初始化 AVPlayer 和 AVPlayerLayer

```
 // setAVPlayer
        self.player = [[AVPlayer alloc] init];
        _playerLayer = [AVPlayerLayer playerLayerWithPlayer:_player];
        [self.playerView.layer addSublayer:_playerLayer];
```

### ２.　根据 URL 获取 AVPayerItem，并替换 AVPlayer 的 AVPlayerItem

在第一步，布局初始化时，AVPlayer 并没有 AVPlayerItem，AVPlayer 提供了 `- (void)replaceCurrentItemWithPlayerItem:(nullable AVPlayerItem *)item;`  方法，用于切换视频。

```
- (void)updatePlayerWithURL:(NSURL *)url {
    _playerItem = [AVPlayerItem playerItemWithURL:url]; // create item
    [_player  replaceCurrentItemWithPlayerItem:_playerItem]; // replaceCurrentItem
    [self addObserverAndNotification]; // 注册观察者，通知
}
```

### 3. KVO 获取视频信息， 观察缓冲进度
观察 AVPlayerItem 的 `status` 属性，当状态变为 `AVPlayerStatusReadyToPlay` 时才可以使用。

也可以观察 `loadedTimeRanges` 获取缓冲进度

注册观察者：

```
 [_playerItem addObserver:self forKeyPath:@"status" options:(NSKeyValueObservingOptionNew) context:nil]; // 观察status属性
```

执行观察者方法：

```
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context {
    AVPlayerItem *item = (AVPlayerItem *)object;
    if ([keyPath isEqualToString:@"status"]) {
            AVPlayerStatus status = [[change objectForKey:@"new"] intValue]; // 获取更改后的状态
            if (status == AVPlayerStatusReadyToPlay) {
                CMTime duration = item.duration; // 获取视频长度
                // 设置视频时间
                [self setMaxDuration:CMTimeGetSeconds(duration)];
                // 播放
                [self play];
            } else if (status == AVPlayerStatusFailed) {
                NSLog(@"AVPlayerStatusFailed");
            } else {
                NSLog(@"AVPlayerStatusUnknown");
            }
        
    } else if ([keyPath isEqualToString:@"loadedTimeRanges"]) {
        NSTimeInterval timeInterval = [self availableDurationRanges]; // 缓冲时间
        CGFloat totalDuration = CMTimeGetSeconds(_playerItem.duration); // 总时间
        [self.loadedProgress setProgress:timeInterval / totalDuration animated:YES]; // 更新缓冲条
    }
}
```

### 4. 播放过程中响应：播放、 暂停、 拖动

AVPlayer 提供了 `play` , `pause`, 和 `- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler` 方法。 

在看 AVPlayer 的 seekToTime 之前，先来认识一个结构体。 

**CMTime** 是专门用于标识电影时间的结构体.

```
typedef struct
{
	CMTimeValue	value;	 // 帧数
	CMTimeScale	timescale;  // 帧率（影片每秒有几帧）
	CMTimeFlags	flags;		
	CMTimeEpoch	epoch;
} CMTime;
```
AVPlayerItem 的 duration 属性就是一个 CMTime 类型的数据。 如果我们想要获取影片的总秒数那么就可以用 duration.value / duration.timeScale 计算出来。也可以使用 CMTimeGetSeconds 函数

**CMTimeGetSeconds(CMtime time)**  
double seconds = CMTimeGetSeconds(item.duration);  // 相当于 duration.value / duration.timeScale

如果一个影片为60frame（帧）每秒， 当前想要跳转到 120帧的位置，也就是两秒的位置，那么就可以创建一个 CMTime 类型数据。

CMTime,通常用如下两个函数来创建.

**CMTimeMake(int64\_t value, int32\_t scale)**  
CMTime time1 = CMTimeMake(120, 60);


**CMTimeMakeWithSeconds(Flout64 seconds, int32\_t scale)**  
CMTime time2 = CMTimeWithSeconds(120, 60); 

CMTimeMakeWithSeconds 和CMTimeMake 区别在于，第一个函数的第一个参数可以是float，其他一样。

拖拽方法如下：

```Object-C
- (IBAction)playerSliderValueChanged:(id)sender {
    _isSliding = YES;
    [self pause];
    // 跳转到拖拽秒处
    // self.playProgress.maxValue = value / timeScale
    // value = progress.value * timeScale
    // CMTimemake(value, timeScale) =  (progress.value, 1.0)
    CMTime changedTime = CMTimeMakeWithSeconds(self.playProgress.value, 1.0);
    [_playerItem seekToTime:changedTime completionHandler:^(BOOL finished) {
        // 跳转完成后
    }];
}
```

### 5. 观察 AVPlayer 播放进度

AVPlayerItem 是使用 KVO 模式观察状态，和 缓冲进度。而 AVPlayer 给我们直接提供了 观察播放进度更为方便的方法。 

`- (id)addPeriodicTimeObserverForInterval:(CMTime)interval queue:(nullable dispatch_queue_t)queue usingBlock:(void (^)(CMTime time))block;`

方法名如其意， “添加周期时间观察者” ，参数1 `interal` 为CMTime 类型的，参数2 为一个 返回值为空，参数为 CMTime 的block类型。 

简而言之就是，每隔一段时间后执行 block。

比如： 我把时间间隔设置为， 1/ 30 秒，然后 block 里面更新 UI。就是一秒钟更新 30次UI。

播放进度代码如下：

```Object-C
// 观察播放进度
- (void)monitoringPlayback:(AVPlayerItem *)item {
    __weak typeof(self)WeakSelf = self;
    
    // 观察间隔, CMTime 为30分之一秒
    _playTimeObserver = [_player addPeriodicTimeObserverForInterval:CMTimeMake(1, 30.0) queue:dispatch_get_main_queue() usingBlock:^(CMTime time) {
        if (_touchMode != TouchPlayerViewModeHorizontal) {
            // 获取 item 当前播放秒
            float currentPlayTime = (double)item.currentTime.value/ item.currentTime.timescale;
            // 更新slider, 如果正在滑动则不更新
            if (_isSliding == NO) {
                [WeakSelf updateVideoSlider:currentPlayTime];
            }
        } else {
            return;
        }
    }];
}
```

**注意：** 给 palyer 添加了 timeObserver 后，不使用的时候记得移除 `removeTimeObserver` 否则会占用大量内存。

比如，我在dealloc里面做了移除：

```Object-C
- (void)dealloc {
    [self removeObserveAndNOtification];
    [_player removeTimeObserver:_playTimeObserver]; // 移除playTimeObserver
}
```

### 6. AVPlayerItem 通知

AVPlaerItem 播放完成后，系统会自动发送通知，通知的定义详情请见 `AVPlayerItem.h`.

```Object-C
/* Note that NSNotifications posted by AVPlayerItem may be posted on a different thread from the one on which the observer was registered. */

// notifications                                                                                description
AVF_EXPORT NSString *const AVPlayerItemTimeJumpedNotification			 NS_AVAILABLE(10_7, 5_0);	// the item's current time has changed discontinuously
AVF_EXPORT NSString *const AVPlayerItemDidPlayToEndTimeNotification      NS_AVAILABLE(10_7, 4_0);   // item has played to its end time
AVF_EXPORT NSString *const AVPlayerItemFailedToPlayToEndTimeNotification NS_AVAILABLE(10_7, 4_3);   // item has failed to play to its end time
AVF_EXPORT NSString *const AVPlayerItemPlaybackStalledNotification       NS_AVAILABLE(10_9, 6_0);    // media did not arrive in time to continue playback
AVF_EXPORT NSString *const AVPlayerItemNewAccessLogEntryNotification	 NS_AVAILABLE(10_9, 6_0);	// a new access log entry has been added
AVF_EXPORT NSString *const AVPlayerItemNewErrorLogEntryNotification		 NS_AVAILABLE(10_9, 6_0);	// a new error log entry has been added

// notification userInfo key                                                                    type
AVF_EXPORT NSString *const AVPlayerItemFailedToPlayToEndTimeErrorKey     NS_AVAILABLE(10_7, 4_3);   // NSError
```

因此，如果我们想要在某个状态下，执行某些操作。监听 AVPlayerItem 的相关通知就行了。 比如，我想要播放完成后，暂停播放。 给`AVPlayerItemDidPlayToEndTimeNotification` 添加观察者。

```Object-C
 [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playbackFinished:) name:AVPlayerItemDidPlayToEndTimeNotification object:nil];
 
 // 播放完成后
 - (void)playbackFinished:(NSNotification *)notification {
    NSLog(@"视频播放完成通知");
    _playerItem = [notification object];
    [_playerItem seekToTime:kCMTimeZero]; // item 跳转到初始
	  //[_player play]; // 循环播放
}
```

## 总结

使用 AVPlayer 的时候，一定要注意 AVPlayer 、 AVPlayerLayer 和 AVPlayerItem 三者之间的关系。 AVPlayer 负责控制播放， layer 显示播放， item 提供数据，当前播放时间， 已加载情况。 Item 中一些基本的属性, status, duration, loadedTimeRanges， currentTime（当前播放时间）。

最重要的还是多总结，6月份写的这个 Demo ,现在才总结，懒癌到晚期 =。 =

当然，如果我写的文章有幸让你看到了最后，那么， 或许 你想要更多的功能。比如，横竖屏旋转，一些交互动画等等。 我有个简单地 demo，实现了一些小小的功能，放到了 github 上， 里面还有很多不足，多沟通交流 = W = 。 

[githubDemo地址](https://github.com/littledogboy/AGPlayer)

![AGPlayer录屏1.gif](http://upload-images.jianshu.io/upload_images/326377-cfe8b955c6a31ed1.gif?imageMogr2/auto-orient/strip)

















