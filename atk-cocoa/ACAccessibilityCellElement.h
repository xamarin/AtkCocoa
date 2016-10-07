#import <Cocoa/Cocoa.h>
#include <gtk/gtk.h>
#include "gailcell.h"

@interface ACAccessibilityCellElement : NSAccessibilityElement

- (instancetype)initWithDelegate:(GailCell *)delegate
							 row:(GtkTreeRowReference *)row_ref 
						  column:(GtkTreeViewColumn *)column
						   index:(int)indexInColumn;
- (GailCell *)delegate;

@end