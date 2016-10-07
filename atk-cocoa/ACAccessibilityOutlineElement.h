#import <Cocoa/Cocoa.h>
#include <ACAccessibilityElement.h>

@interface ACAccessibilityOutlineElement : ACAccessibilityElement <NSAccessibilityOutline>

- (instancetype)initWithDelegate:(AcElement *)delegate;

@end