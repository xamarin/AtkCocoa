/* GAIL - The GNOME Accessibility Implementation Library
 * Copyright 2004 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#undef GTK_DISABLE_DEPRECATED

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "atk-cocoa/gailcombobox.h"

#import "atk-cocoa/ACAccessibilityComboBoxElement.h"
#import "atk-cocoa/ACAccessibilityPopupMenuElement.h"

static void         gail_combo_box_class_init              (GailComboBoxClass *klass);
static void         gail_combo_box_init                    (GailComboBox      *combo_box);
static void         gail_combo_box_real_initialize         (AtkObject      *obj,
                                                            gpointer       data);
static id<NSAccessibility> get_real_accessibility_element  (AcElement *element);

static void         gail_combo_box_changed_gtk             (GtkWidget      *widget);

static const gchar* gail_combo_box_get_name                (AtkObject      *obj);
static gint         gail_combo_box_get_n_children          (AtkObject      *obj);
static AtkObject*   gail_combo_box_ref_child               (AtkObject      *obj,
                                                            gint           i);
static void         gail_combo_box_finalize                (GObject        *object);
static void         atk_action_interface_init              (AtkActionIface *iface);

static gboolean     gail_combo_box_do_action               (AtkAction      *action,
                                                            gint           i);
static gboolean     idle_do_action                         (gpointer       data);
static gint         gail_combo_box_get_n_actions           (AtkAction      *action);
static const gchar* gail_combo_box_get_description         (AtkAction      *action,
                                                            gint           i);
static const gchar* gail_combo_box_get_keybinding          (AtkAction       *action,
		                                             gint            i);
static const gchar* gail_combo_box_action_get_name         (AtkAction      *action,
                                                            gint           i);
static gboolean              gail_combo_box_set_description(AtkAction      *action,
                                                            gint           i,
                                                            const gchar    *desc);
static void         atk_selection_interface_init           (AtkSelectionIface *iface);
static gboolean     gail_combo_box_add_selection           (AtkSelection   *selection,
                                                            gint           i);
static gboolean     gail_combo_box_clear_selection         (AtkSelection   *selection);
static AtkObject*   gail_combo_box_ref_selection           (AtkSelection   *selection,
                                                            gint           i);
static gint         gail_combo_box_get_selection_count     (AtkSelection   *selection);
static gboolean     gail_combo_box_is_child_selected       (AtkSelection   *selection,
                                                            gint           i);
static gboolean     gail_combo_box_remove_selection        (AtkSelection   *selection,
                                                            gint           i);
static gboolean real_perform_press (AcElement *element);
static gboolean real_perform_show_menu (AcElement *element);

static void popup_created (GailComboBox *gail_combo_box,
                           AtkObject *popup);
static void popup_destroyed (AtkObject *popup);

G_DEFINE_TYPE_WITH_CODE (GailComboBox, gail_combo_box, GAIL_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_ACTION, atk_action_interface_init)
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_SELECTION, atk_selection_interface_init))

static void
gail_combo_box_class_init (GailComboBoxClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  AcElementClass *element_class = AC_ELEMENT_CLASS (klass);

  gobject_class->finalize = gail_combo_box_finalize;

  class->get_name = gail_combo_box_get_name;
  class->get_n_children = gail_combo_box_get_n_children;
  class->ref_child = gail_combo_box_ref_child;
  class->initialize = gail_combo_box_real_initialize;

  element_class->get_accessibility_element = get_real_accessibility_element;
  element_class->perform_press = real_perform_press;
  element_class->perform_show_menu = real_perform_show_menu;
}

static void
gail_combo_box_init (GailComboBox      *combo_box)
{
  combo_box->press_description = NULL;
  combo_box->press_keybinding = NULL;
  combo_box->old_selection = -1;
  combo_box->name = NULL;
  combo_box->popup_set = FALSE;
}

static gboolean
toggle_button_focused (GtkToggleButton *button,
                       GdkEventFocus *focus,
                       AtkObject *obj)
{
  gboolean return_val = FALSE;

  ACAccessibilityElement *real_element = ac_element_get_accessibility_element (AC_ELEMENT (obj));

  // We ignore the owner so that we don't get into a loop. At this point ATK things the focus is on the togglebutton
  // but NSAccessibilty thinks the focus is on the combobox
  [real_element setAccessibilityFocused:YES ignoringOwner:YES];
  [[NSApplication sharedApplication] setAccessibilityApplicationFocusedUIElement:real_element];
  NSAccessibilityPostNotificationWithUserInfo([NSApplication sharedApplication],
                                              NSAccessibilityFocusedUIElementChangedNotification,
                                              @{NSAccessibilityUIElementsKey: @[real_element]});
  return FALSE;
}

static void
toggle_button_toggled (GtkToggleButton *button,
                       gpointer obj)
{
  GtkComboBox *combo = GTK_COMBO_BOX (obj);
  AtkObject *popup = gtk_combo_box_get_popup_accessible(combo);
  AtkObject *combo_accessible = gtk_widget_get_accessible(GTK_WIDGET (obj));

  if (gtk_toggle_button_get_active(button)) {
    popup_created(GAIL_COMBO_BOX (combo_accessible), popup);
  } else {
    popup_destroyed(popup);
  }
}

struct _foreachData {
  GtkComboBox *combo;
  AtkObject *accessible;
};

static void
combo_box_forall_cb (GtkWidget *child, gpointer obj)
{
  struct _foreachData *data = (struct _foreachData *)obj;

  if (GTK_IS_TOGGLE_BUTTON (child)) {
    g_signal_connect (child, "focus-in-event", G_CALLBACK (toggle_button_focused), data->accessible);
    g_signal_connect (child, "toggled", G_CALLBACK (toggle_button_toggled), data->combo);
  }
}

static void
gail_combo_box_real_initialize (AtkObject *obj,
                                gpointer  data)
{
  GtkComboBox *combo_box;
  GailComboBox *gail_combo_box;
  AtkObject *popup;

  ATK_OBJECT_CLASS (gail_combo_box_parent_class)->initialize (obj, data);

  combo_box = GTK_COMBO_BOX (data);

  gail_combo_box = GAIL_COMBO_BOX (obj);

  g_signal_connect (combo_box,
                    "changed",
                    G_CALLBACK (gail_combo_box_changed_gtk),
                    NULL);
  gail_combo_box->old_selection = gtk_combo_box_get_active (combo_box);

  popup = gtk_combo_box_get_popup_accessible (combo_box);
  if (popup)
    {
      atk_object_set_parent (popup, obj);
      gail_combo_box->popup_set = TRUE;
    }

  if (gtk_combo_box_get_has_entry (combo_box) || GTK_IS_COMBO_BOX_ENTRY(combo_box)) {
    atk_object_set_parent (gtk_widget_get_accessible (gtk_bin_get_child (GTK_BIN (combo_box))), obj);
    obj->role = ATK_ROLE_FILLER;
  } else {
    obj->role = ATK_ROLE_COMBO_BOX;
  }

  // When the combobox gets focus, it passes the focus to an internal GtkToggleButton
  // We need to watch for that togglebutton being focused and focus the main ComboBox
  // because that is where the accessibility works for VoiceOver.
  // But we can't get the internal togglebutton in any sane way, so we need to go looking for it

  struct _foreachData fed;
  fed.combo = combo_box;
  fed.accessible = gail_combo_box;
  gtk_container_forall(GTK_CONTAINER (combo_box), combo_box_forall_cb, &fed);
}

static id<NSAccessibility>
get_real_accessibility_element (AcElement *element)
{
  GtkComboBox *box = GTK_COMBO_BOX(ac_element_get_owner(element));

  if (gtk_combo_box_get_has_entry(box) || GTK_IS_COMBO_BOX_ENTRY(box)) {
    return [[ACAccessibilityComboBoxElement alloc] initWithDelegate:element];
  } else {
    return [[ACAccessibilityPopupMenuElement alloc] initWithDelegate:element];
  }
}

static void
gail_combo_box_changed_gtk (GtkWidget *widget)
{
  GtkComboBox *combo_box;
  AtkObject *obj;
  GailComboBox *gail_combo_box;
  gint index;

  combo_box = GTK_COMBO_BOX (widget);

  index = gtk_combo_box_get_active (combo_box);
  obj = gtk_widget_get_accessible (widget);
  gail_combo_box = GAIL_COMBO_BOX (obj);
  if (gail_combo_box->old_selection != index)
    {
      gail_combo_box->old_selection = index;
      g_object_notify (G_OBJECT (obj), "accessible-name");
      g_signal_emit_by_name (obj, "selection_changed");

      ac_element_notify(AC_ELEMENT (obj), NSAccessibilityValueChangedNotification, @{NSAccessibilityUIElementsKey: @[ac_element_get_accessibility_element(AC_ELEMENT(obj))]});
    }
}

static const gchar*
gail_combo_box_get_name (AtkObject *obj)
{
  GtkWidget *widget;
  GtkComboBox *combo_box;
  GailComboBox *gail_combo_box;
  GtkTreeIter iter;
  const gchar *name;
  GtkTreeModel *model;
  gint n_columns;
  gint i;

  g_return_val_if_fail (GAIL_IS_COMBO_BOX (obj), NULL);

  name = ATK_OBJECT_CLASS (gail_combo_box_parent_class)->get_name (obj);
  if (name)
    return name;

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return NULL;

  combo_box = GTK_COMBO_BOX (widget);
  gail_combo_box = GAIL_COMBO_BOX (obj);
  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    {
      model = gtk_combo_box_get_model (combo_box);
      n_columns = gtk_tree_model_get_n_columns (model);
      for (i = 0; i < n_columns; i++)
        {
          GValue value = { 0, };

          gtk_tree_model_get_value (model, &iter, i, &value);
          if (G_VALUE_HOLDS_STRING (&value))
            {
	      if (gail_combo_box->name) g_free (gail_combo_box->name);
              gail_combo_box->name =  g_strdup ((gchar *) 
						g_value_get_string (&value));
	      g_value_unset (&value);
              break;
            }
	  else
	    g_value_unset (&value);
        }
    }
  return gail_combo_box->name;
}

/*
 * The children of a GailComboBox are the list of items and the entry field
 * if it is editable.
 */
static gint
gail_combo_box_get_n_children (AtkObject* obj)
{
  gint n_children = 0;
  GtkWidget *widget;

  g_return_val_if_fail (GAIL_IS_COMBO_BOX (obj), 0);

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return 0;

  n_children++;
  if (gtk_combo_box_get_has_entry (GTK_COMBO_BOX (widget)) ||
      GTK_IS_COMBO_BOX_ENTRY (widget))
    n_children ++;

  return n_children;
}

static AtkObject*
gail_combo_box_ref_child (AtkObject *obj,
                          gint      i)
{
  GtkWidget *widget;
  AtkObject *child;
  GailComboBox *box;

  g_return_val_if_fail (GAIL_IS_COMBO_BOX (obj), NULL);

  widget = GTK_ACCESSIBLE (obj)->widget;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return NULL;

  if (i == 0)
    {
      child = gtk_combo_box_get_popup_accessible (GTK_COMBO_BOX (widget));
      box = GAIL_COMBO_BOX (obj);
      if (box->popup_set == FALSE)
        {
          atk_object_set_parent (child, obj);
          box->popup_set = TRUE;
        }
    }
  else if (i == 1 && (gtk_combo_box_get_has_entry (GTK_COMBO_BOX (widget)) ||
                      GTK_IS_COMBO_BOX_ENTRY (widget)))
    {
      child = gtk_widget_get_accessible (gtk_bin_get_child (GTK_BIN (widget)));
    }
  else
    {
      return NULL;
    }
  return g_object_ref (child);
}

static void
atk_action_interface_init (AtkActionIface *iface)
{
  iface->do_action = gail_combo_box_do_action;
  iface->get_n_actions = gail_combo_box_get_n_actions;
  iface->get_description = gail_combo_box_get_description;
  iface->get_keybinding = gail_combo_box_get_keybinding;
  iface->get_name = gail_combo_box_action_get_name;
  iface->set_description = gail_combo_box_set_description;
}

static gboolean
gail_combo_box_do_action (AtkAction *action,
                          gint      i)
{
  GailComboBox *combo_box;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (action)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  if (!gtk_widget_get_sensitive (widget) || !gtk_widget_get_visible (widget))
    return FALSE;

  combo_box = GAIL_COMBO_BOX (action);
  if (i == 0)
    {
      if (combo_box->action_idle_handler)
        return FALSE;

      combo_box->action_idle_handler = gdk_threads_add_idle (idle_do_action, combo_box);
      return TRUE;
    }
  else
    return FALSE;
}

static gboolean
popup_unmapped (GtkWidget *widget,
                GailComboBox *combo)
{
  ac_element_focus_and_ignore_next(AC_ELEMENT (combo));
  return FALSE;
}

static void
popup_created (GailComboBox *gail_combo_box,
               AtkObject *popup)
{
  id<NSAccessibility> comboElement = ac_element_get_accessibility_element(AC_ELEMENT (gail_combo_box));
  id<NSAccessibility> menuElement = ac_element_get_accessibility_element(AC_ELEMENT (popup));

  [comboElement setAccessibilityLinkedUIElements:@[menuElement]];
  NSAccessibilityPostNotification(menuElement, NSAccessibilityCreatedNotification);

  // Focus the menu
  GtkWidget *widget = GTK_ACCESSIBLE(popup)->widget;
  if (GTK_IS_MENU (widget)) {
    gtk_menu_shell_select_first(GTK_MENU_SHELL (widget), FALSE);
  }

  g_signal_connect (widget, "hide", G_CALLBACK (popup_unmapped), gail_combo_box);
}

static void
popup_destroyed (AtkObject *popup)
{
  ac_element_notify(AC_ELEMENT (popup), NSAccessibilityUIElementDestroyedNotification, NULL);
}

static gboolean
maybe_show_popup (GailComboBox *gail_combo_box)
{
  GtkComboBox *combo_box;
  GtkWidget *widget;
  AtkObject *popup;
  gboolean do_popup;

  widget = GTK_ACCESSIBLE (gail_combo_box)->widget;

  combo_box = GTK_COMBO_BOX (widget);

  popup = gtk_combo_box_get_popup_accessible (combo_box);
  do_popup = !gtk_widget_get_mapped (GTK_ACCESSIBLE (popup)->widget);
  if (do_popup) {
    gtk_combo_box_popup (combo_box);
    popup_created(gail_combo_box, popup);
  } else {
    gtk_combo_box_popdown (combo_box);
    popup_destroyed (popup);
  }

  return TRUE;
}

static gboolean
idle_do_action (gpointer data)
{
  GtkComboBox *combo_box;
  GtkWidget *widget;
  GailComboBox *gail_combo_box;
  AtkObject *popup;
  gboolean do_popup;

  gail_combo_box = GAIL_COMBO_BOX (data);
  gail_combo_box->action_idle_handler = 0;
  widget = GTK_ACCESSIBLE (gail_combo_box)->widget;
  if (widget == NULL || /* State is defunct */
      !gtk_widget_get_sensitive (widget) || !gtk_widget_get_visible (widget))
    return FALSE;

  combo_box = GTK_COMBO_BOX (widget);

  popup = gtk_combo_box_get_popup_accessible (combo_box);

  do_popup = !gtk_widget_get_mapped (GTK_ACCESSIBLE (popup)->widget);
  if (do_popup) {
    gtk_combo_box_popup (combo_box);
    popup_created(gail_combo_box, popup);
  } else {
    gtk_combo_box_popdown (combo_box);
    popup_destroyed(popup);
  }
  return FALSE;
}

static gboolean
real_perform_press (AcElement *element)
{
  maybe_show_popup (GAIL_COMBO_BOX (element));
  return TRUE;
}

static gboolean
real_perform_show_menu (AcElement *element)
{
  maybe_show_popup (GAIL_COMBO_BOX (element));
  return TRUE;
}

static gint
gail_combo_box_get_n_actions (AtkAction *action)
{
  /*
   * The default behavior of a combo_box box is to have one action -
   */
  return 1;
}

static const gchar*
gail_combo_box_get_description (AtkAction *action,
                           gint      i)
{
  if (i == 0)
    {
      GailComboBox *combo_box;

      combo_box = GAIL_COMBO_BOX (action);
      return combo_box->press_description;
    }
  else
    return NULL;
}

static const gchar*
gail_combo_box_get_keybinding (AtkAction *action,
		                    gint      i)
{
  GailComboBox *combo_box;
  gchar *return_value = NULL;
  switch (i)
  {
     case 0:
      {
	  GtkWidget *widget;
	  GtkWidget *label;
	  AtkRelationSet *set;
	  AtkRelation *relation;
	  GPtrArray *target;
	  gpointer target_object;
	  guint key_val;

	  combo_box = GAIL_COMBO_BOX (action);
	  widget = GTK_ACCESSIBLE (combo_box)->widget;
	  if (widget == NULL)
             return NULL;
	  set = atk_object_ref_relation_set (ATK_OBJECT (action));
	  if (!set)
             return NULL;
	  label = NULL;
	  relation = atk_relation_set_get_relation_by_type (set, ATK_RELATION_LABELLED_BY);
	  if (relation)
	  {
	     target = atk_relation_get_target (relation);
	     target_object = g_ptr_array_index (target, 0);
	     if (GTK_IS_ACCESSIBLE (target_object))
	     {
	        label = GTK_ACCESSIBLE (target_object)->widget;
	     }
	  }
	  g_object_unref (set);
	  if (GTK_IS_LABEL (label))
	  {
             key_val = gtk_label_get_mnemonic_keyval (GTK_LABEL (label));
	     if (key_val != GDK_VoidSymbol)
             return_value = gtk_accelerator_name (key_val, GDK_MOD1_MASK);
	  }
	   g_free (combo_box->press_keybinding);
	   combo_box->press_keybinding = return_value;
	   break;
       }
    default:
	   break;
  }
  return return_value;
}


static const gchar*
gail_combo_box_action_get_name (AtkAction *action,
                                gint      i)
{
  if (i == 0)
    return "press";
  else
    return NULL;
}

static gboolean
gail_combo_box_set_description (AtkAction   *action,
                                gint        i,
                                const gchar *desc)
{
  if (i == 0)
    {
      GailComboBox *combo_box;

      combo_box = GAIL_COMBO_BOX (action);
      g_free (combo_box->press_description);
      combo_box->press_description = g_strdup (desc);
      return TRUE;
    }
  else
    return FALSE;
}

static void
atk_selection_interface_init (AtkSelectionIface *iface)
{
  iface->add_selection = gail_combo_box_add_selection;
  iface->clear_selection = gail_combo_box_clear_selection;
  iface->ref_selection = gail_combo_box_ref_selection;
  iface->get_selection_count = gail_combo_box_get_selection_count;
  iface->is_child_selected = gail_combo_box_is_child_selected;
  iface->remove_selection = gail_combo_box_remove_selection;
  /*
   * select_all_selection does not make sense for a combo_box
   * so no implementation is provided.
   */
}

static gboolean
gail_combo_box_add_selection (AtkSelection *selection,
                              gint         i)
{
  GtkComboBox *combo_box;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  combo_box = GTK_COMBO_BOX (widget);

  gtk_combo_box_set_active (combo_box, i);
  return TRUE;
}

static gboolean 
gail_combo_box_clear_selection (AtkSelection *selection)
{
  GtkComboBox *combo_box;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  combo_box = GTK_COMBO_BOX (widget);

  gtk_combo_box_set_active (combo_box, -1);
  return TRUE;
}

static AtkObject*
gail_combo_box_ref_selection (AtkSelection *selection,
                              gint         i)
{
  GtkComboBox *combo_box;
  GtkWidget *widget;
  AtkObject *obj;
  gint index;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return NULL;

  combo_box = GTK_COMBO_BOX (widget);

  /*
   * A combo_box box can have only one selection.
   */
  if (i != 0)
    return NULL;

  obj = gtk_combo_box_get_popup_accessible (combo_box);
  index = gtk_combo_box_get_active (combo_box);
  return atk_object_ref_accessible_child (obj, index);
}

static gint
gail_combo_box_get_selection_count (AtkSelection *selection)
{
  GtkComboBox *combo_box;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return 0;

  combo_box = GTK_COMBO_BOX (widget);

  return (gtk_combo_box_get_active (combo_box) == -1) ? 0 : 1;
}

static gboolean
gail_combo_box_is_child_selected (AtkSelection *selection,
                                  gint         i)
{
  GtkComboBox *combo_box;
  gint j;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  combo_box = GTK_COMBO_BOX (widget);

  j = gtk_combo_box_get_active (combo_box);

  return (j == i);
}

static gboolean
gail_combo_box_remove_selection (AtkSelection *selection,
                                 gint         i)
{
  if (atk_selection_is_child_selected (selection, i))
    atk_selection_clear_selection (selection);

  return TRUE;
}

static void
gail_combo_box_finalize (GObject *object)
{
  GailComboBox *combo_box = GAIL_COMBO_BOX (object);

  g_free (combo_box->press_description);
  g_free (combo_box->press_keybinding);
  g_free (combo_box->name);
  if (combo_box->action_idle_handler)
    {
      g_source_remove (combo_box->action_idle_handler);
      combo_box->action_idle_handler = 0;
    }

  G_OBJECT_CLASS (gail_combo_box_parent_class)->finalize (object);
}
