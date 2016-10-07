#import <Cocoa/Cocoa.h>
#import "ACAccessibilityElement.h"
#include <acelement.h>
#include <gtk/gtk.h>

@interface ACAccessibilityTreeRowElement : ACAccessibilityElement

- (instancetype)initWithDelegate:(AcElement *)delegate treeRow:(GtkTreeRowReference *)row treeView:(GtkTreeView *)treeView;
- (GtkTreeRowReference *)rowReference;
- (void)addChildRowElement:(ACAccessibilityTreeRowElement *)child;
- (void)removeChildRowElement:(ACAccessibilityTreeRowElement *)child;

@end