#import <Cocoa/Cocoa.h>
#import "ACAccessibilityElement.h"
#include <acelement.h>
#include <gtk/gtk.h>

@class ACAccessibilityTreeColumnElement;
@class ACAccessibilityTreeRowElement;

@interface ACAccessibilityTreeCellElement : ACAccessibilityElement

- (instancetype)initWithDelegate:(AcElement *)delegate withDisclosureButton:(BOOL)canDisclose;
- (void)addToRow:(ACAccessibilityTreeRowElement *)rowElement column:(ACAccessibilityTreeColumnElement *)columnElement;
- (void)addDisclosureButton;
- (void)removeDisclosureButton;

@end