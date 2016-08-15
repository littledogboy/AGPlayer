//
//  NSString+time.h
//  AGPlayer
//
//  Created by 吴书敏 on 16/7/16.
//  Copyright © 2016年 littledogboy. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@interface NSString (time)

// 播放器时间转换
+ (NSString *)convertTime:(CGFloat)second;

@end
