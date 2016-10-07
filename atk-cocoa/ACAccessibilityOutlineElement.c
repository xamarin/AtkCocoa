#import "ACAccessibilityOutlineElement.h"
#include "acelement.h"
#include "acdebug.h"

@implementation ACAccessibilityOutlineElement {
}

- (instancetype)initWithDelegate:(AcElement *)delegate
{
	return [super initWithDelegate:delegate];
}

- (NSRect)accessibilityFrame
{
	return [super accessibilityFrame];
}

- (NSString *)accessibilityLabel
{
	return [super accessibilityLabel];
}

- (id<NSAccessibility>)accessibilityParent
{
	return [super accessibilityParent];
}

- (NSArray *)accessibilityColumns
{
	return nil;
}

- (NSArray *)accessibilitySelectedColumns
{
	return nil;
}

- (NSArray *)accessibilityVisibleColumns
{
	return nil;
}

- (NSArray *)accessibilityColumnHeaderUIElements
{
	return nil;
}

- (NSArray *)accessibilityRows
{
	return nil;
}

- (NSArray *)accessibilitySelectedRows
{
	return nil;
}

// NSArray contains id<NSAccessibilityRow> elements
- (void)setAccessibilitySelectedRows:(NSArray *)selectedRows
{
}

- (NSArray *)accessibilityVisibleRows
{
	return nil;
}

- (NSArray *)accessibilityRowHeaderUIElements
{
	return nil;
}

- (NSString *)accessibilityHeaderGroup
{
	return @"";
}

- (NSArray *)accessibilitySelectedCells
{
	return nil;
}

- (NSArray *)accessibilityVisibleCells
{
	return nil;
}

@end