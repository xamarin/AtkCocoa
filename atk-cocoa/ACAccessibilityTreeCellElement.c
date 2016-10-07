#import "ACAccessibilityTreeCellElement.h"

#import "ACAccessibilityTreeColumnElement.h"
#import "ACAccessibilityTreeRowElement.h"

@implementation ACAccessibilityTreeCellElement {
    ACAccessibilityTreeRowElement *_rowElement;
    ACAccessibilityTreeColumnElement *_columnElement;
}

- (instancetype)initWithDelegate:(AcElement *)delegate
{
    self = [super initWithDelegate:delegate];
    if (!self) {
        return nil;
    }

    [self setAccessibilityRole:NSAccessibilityCellRole];
    return self;
}

- (void)addToRow:(ACAccessibilityTreeRowElement *)rowElement column:(ACAccessibilityTreeColumnElement *)columnElement
{
    _rowElement = rowElement;
    _columnElement = columnElement;

    [columnElement accessibilityAddChildElement:self];
    [rowElement accessibilityAddChildElement:self];
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkTreeViewColumn *column;
    GtkWidget *treeView;
    GtkTreePath *path;
    int x, y;

    column = [_columnElement column];
    treeView = gtk_tree_view_column_get_tree_view (column);
    path = gtk_tree_row_reference_get_path ([_rowElement rowReference]);

	gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, column, &cellSpace);
	gtk_tree_path_free (path);
	
	gtk_widget_translate_coordinates (treeView, gtk_widget_get_toplevel (treeView), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y += y;

    g_print ("Getting cellspace for cell\n");

    return cellSpace;
}

@end