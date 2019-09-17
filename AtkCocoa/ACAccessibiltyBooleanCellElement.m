//
//  ACAccessibiltyBooleanCellElement.m
//  AtkCocoa
//
//  Created by iain on 10/09/2019.
//  Copyright © 2019 Microsoft. All rights reserved.
//

#import "atk-cocoa/ACAccessibiltyBooleanCellElement.h"

@implementation ACAccessibiltyBooleanCellElement

- (NSArray *)accessibilityActionNames
{
    return @[NSAccessibilityPressAction];
}

- (BOOL)accessibilityPerformPress
{
    GtkTreeViewColumn *column = [self column];
    GList *renderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT (column));

    GtkCellRendererToggle *toggle = g_list_nth_data(renderers, [self indexInColumn]);
    g_list_free (renderers);

    if (toggle == NULL || !GTK_IS_CELL_RENDERER_TOGGLE(toggle)) {
        return NO;
    }

    char *path = gtk_tree_path_to_string([self rowPath]);
    gtk_cell_renderer_activate(GTK_CELL_RENDERER(toggle), NULL, NULL, path, NULL, NULL, GTK_CELL_RENDERER_FOCUSED);

    g_free (path);

    return YES;
}
@end
