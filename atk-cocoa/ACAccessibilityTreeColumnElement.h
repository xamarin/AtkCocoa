#import <Cocoa/Cocoa.h>
#import "ACAccessibilityElement.h"
#include <acelement.h>
#include <gtk/gtk.h>

@interface ACAccessibilityTreeColumnElement : ACAccessibilityElement

- (instancetype)initWithDelegate:(AcElement *)delegate treeColumn:(GtkTreeViewColumn *)column;
- (GtkTreeViewColumn *)column;
@end