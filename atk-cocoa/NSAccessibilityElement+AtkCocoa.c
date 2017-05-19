#import "NSAccessibilityElement+AtkCocoa.h"

@implementation NSAccessibilityElement (AtkCocoa)

- (void)ac_accessibilityRemoveChildElement:(id<NSAccessibility>)element
{
    NSMutableArray *oldChildren = [[self accessibilityChildren] mutableCopy];
    [oldChildren removeObject:element];
    [self setAccessibilityChildren:oldChildren];
}

- (void)ac_accessibilityAddColumnElement:(id<NSAccessibility>)child
{
    NSArray *arr = [self accessibilityColumns];
    NSMutableArray *oldColumns;

    if (arr == nil) {
        oldColumns = [NSMutableArray array];
    } else {
        oldColumns = [arr mutableCopy];
    }

    [oldColumns addObject:child];
    [self setAccessibilityColumns:oldColumns];
}

- (void)ac_accessibilityRemoveColumnElement:(id<NSAccessibility>)child
{
    NSMutableArray *oldColumns = [[self accessibilityColumns] mutableCopy];
    [oldColumns removeObject:child];
    [self setAccessibilityColumns:oldColumns];
}

- (void)ac_accessibilityAddRowElement:(id<NSAccessibility>)child
{
    NSArray *arr = [self accessibilityRows];
    NSMutableArray *oldRows;

    if (arr == nil) {
        oldRows = [NSMutableArray array];
    } else {
        oldRows = [arr mutableCopy];
    }

    [oldRows addObject:child];
    [self setAccessibilityColumns:oldRows];
}

- (void)ac_accessibilityRemoveRowElement:(id<NSAccessibility>)child
{
    NSMutableArray *oldRows = [[self accessibilityRows] mutableCopy];
    [oldRows removeObject:child];
    [self setAccessibilityColumns:oldRows];
}

@end