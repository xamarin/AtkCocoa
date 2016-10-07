#import "ACAccessibilityTreeColumnElement.h"

@implementation ACAccessibilityTreeColumnElement {
    GtkTreeViewColumn *_column;
}

- (instancetype)initWithDelegate:(AcElement *)delegate treeColumn:(GtkTreeViewColumn *)column
{
    self = [super initWithDelegate:delegate];
    if (!self) {
        return nil;
    }

    // FIXME: Should we ref this?
    _column = column;

    [self setAccessibilityRole:NSAccessibilityColumnRole];
    return self;
}

- (NSString *)accessibilityLabel
{
    return nsstring_from_cstring (gtk_tree_view_column_get_title (_column));
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkWidget *treeView = gtk_tree_view_column_get_tree_view (_column);

    GtkTreePath *path = gtk_tree_path_new_first ();

    int x, y;

    gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, _column, &cellSpace);
    gtk_tree_path_free (path);

    cellSpace.y = 0;
    cellSpace.height = treeView->allocation.height;

    gtk_widget_translate_coordinates (treeView, gtk_widget_get_toplevel (treeView), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y += y;

    return cellSpace;
}

- (GtkTreeViewColumn *)column
{
    return _column;
}
@end