//
//  manager.h
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#ifndef manager_h
#define manager_h

#import <Cocoa/Cocoa.h>

@interface manager : NSObject
+ (BOOL)isInited;
+ (BOOL)initEventTap;
+ (BOOL)stopEventTap;
@end


#endif /* manager_h */
