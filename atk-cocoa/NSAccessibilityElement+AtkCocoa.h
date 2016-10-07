#import <Cocoa/Cocoa.h>

@interface NSAccessibilityElement (AtkCocoa)

- (void)ac_accessibilityRemoveChildElement:(id<NSAccessibility>)element;

@end