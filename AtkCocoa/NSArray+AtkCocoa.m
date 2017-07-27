//
//  NSArray+AtkCocoa.m
//  AtkCocoa
//
//  Created by iain on 26/07/2017.
//  Copyright © 2017 Microsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "atk-cocoa/NSArray+AtkCocoa.h"

@implementation NSArray (AtkCocoa)

- (NSArray *)ac_addObject:(id)obj
{
    NSMutableArray *arrayCopy = [self mutableCopy];
    [arrayCopy addObject:obj];

    return arrayCopy;
}

- (NSArray *)ac_removeObject:(id)obj
{
    NSMutableArray *arrayCopy = [self mutableCopy];
    [arrayCopy removeObject:obj];
    return arrayCopy;
}
@end
