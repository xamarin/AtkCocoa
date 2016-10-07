#import <Cocoa/Cocoa.h>
#include <acelement.h>

@interface ACAccessibilityElement : NSAccessibilityElement

- (instancetype)initWithDelegate:(AcElement *)delegate;
- (AcElement *)delegate;

NSString *nsstring_from_cstring (const char *cstring);

@end