#import "NSAccessibilityElement+AtkCocoa.h"

@implementation NSAccessibilityElement (AtkCocoa)

- (void)ac_accessibilityRemoveChildElement:(id<NSAccessibility>)element
{
    NSMutableArray *oldChildren = [[self accessibilityChildren] mutableCopy];
    [oldChildren removeObject:element];
    [self setAccessibilityChildren:oldChildren];
}

@end