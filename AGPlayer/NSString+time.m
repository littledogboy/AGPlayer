//
//  NSString+time.m
//  AGPlayer
//
//  Created by 吴书敏 on 16/7/16.
//  Copyright © 2016年 littledogboy. All rights reserved.
//

#import "NSString+time.h"

@implementation NSString (time)

+ (NSString *)convertTime:(CGFloat)second {
    // 相对格林时间
    NSDate *date = [NSDate dateWithTimeIntervalSince1970:second];
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    
    if (second / 3600 >= 1) {
        [formatter setDateFormat:@"HH:mm:ss"];
    } else {
        [formatter setDateFormat:@"mm:ss"];
    }
    
    NSString *showTimeNew = [formatter stringFromDate:date];
    return showTimeNew;
}

@end
