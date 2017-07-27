//
//  NSArray+AtkCocoa.h
//  AtkCocoa
//
//  Created by iain on 26/07/2017.
//  Copyright © 2017 Microsoft. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSArray (AtkCocoa)

- (NSArray *)ac_addObject:(id)obj;
- (NSArray *)ac_removeObject:(id)obj;

@end
