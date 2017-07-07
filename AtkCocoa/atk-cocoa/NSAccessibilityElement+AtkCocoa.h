#import <Cocoa/Cocoa.h>

@interface NSAccessibilityElement (AtkCocoa)

- (void)ac_accessibilityRemoveChildElement:(id<NSAccessibility>)element;

- (void)ac_accessibilityAddColumnElement:(id<NSAccessibility>)child;
- (void)ac_accessibilityRemoveColumnElement:(id<NSAccessibility>)child;
- (void)ac_accessibilityAddRowElement:(id<NSAccessibility>)child;
- (void)ac_accessibilityRemoveRowElement:(id<NSAccessibility>)child;

@end