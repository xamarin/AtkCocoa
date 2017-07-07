/*
 * AtkCocoa
 * Copyright 2016 Microsoft Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#import "atk-cocoa/ACAccessibilityNotebookElement.h"
#import "atk-cocoa/ACAccessibilityNotebookTabElement.h"
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"
#include "atk-cocoa/gailnotebook.h"
#include "atk-cocoa/gailnotebookpage.h"

@implementation ACAccessibilityNotebookElement

- (instancetype)initWithDelegate:(AcElement *)delegate
{
	return [super initWithDelegate:delegate];
}

- (BOOL)respondsToSelector:(SEL)sel
{
    // NSLog (@"Notebook: %@", NSStringFromSelector (sel));
    return [super respondsToSelector:sel];
}

- (BOOL)isAccessibilitySelectorAllowed:(SEL)sel
{
    BOOL res = [super isAccessibilitySelectorAllowed:sel];
    //NSLog (@"selector allowed: %@ %d", NSStringFromSelector (sel), res);

    return res;
}

- (NSArray *)accessibilityChildrenInNavigationOrder
{
    //NSArray *c = [super accessibilityChildrenInNavigationOrder];
    NSLog (@"Navigation order");

    return [super accessibilityChildren];
}

- (id)accessibilityValue
{
    GailNotebook *gailbook = GAIL_NOTEBOOK ([self delegate]);
    GtkNotebook *notebook = GTK_NOTEBOOK (ac_element_get_owner ([self delegate]));
    int currentPage = gtk_notebook_get_current_page (notebook);
    GailNotebookPage *page = g_list_nth_data (gailbook->page_cache, currentPage);

    NSLog (@"Got %@", (__bridge id)page->element);
    return (__bridge id)page->element;
}

- (NSArray *)accessibilityContents
{
    NSArray *children = [self accessibilityChildren];
    if (children == nil) {
        return nil;
    }

    NSMutableArray *contents = [NSMutableArray array];
    for (id c in children) {
        if ([c isKindOfClass:[ACAccessibilityNotebookTabElement class]]) {
            continue;
        }
        [contents addObject:c];
    }

    return contents;
}
@end
