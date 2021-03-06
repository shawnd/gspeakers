/* 
 *  Copyright (C) 2002 Jorn Baayen <jorn@nl.linux.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkitemfactory.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkdnd.h>
#include <string.h>

#include<libintl.h>
#define _(String) gettext(String)
#define N_(String) (String)


#include "eggsidebar.h"
#include "eggsidebarbutton.h"

static void     egg_sidebar_button_class_init                  (EggSidebarButtonClass *klass);
static void     egg_sidebar_button_init                        (EggSidebarButton      *button);

static void     egg_sidebar_button_finalize                    (GObject               *object);
static void     egg_sidebar_button_set_property                (GObject               *object,
								guint                  prop_id,
								const GValue          *value,
								GParamSpec            *pspec);
static void     egg_sidebar_button_get_property                (GObject               *object,
								guint                  prop_id,
								GValue                *value,
								GParamSpec            *pspec);

static gboolean egg_sidebar_button_button_press_event_cb       (GtkWidget             *widget,
								GdkEventButton        *event,
								EggSidebarButton      *button);
static char *   egg_sidebar_button_item_factory_translate_func (const char            *path,
								gpointer               unused);
static void     egg_sidebar_button_popup_rename_cb             (EggSidebarButton      *button,
								guint                  action,
								GtkWidget             *widget);
static void     egg_sidebar_button_popup_delete_cb             (EggSidebarButton      *button,
								guint                  action,
								GtkWidget             *widget);
static void     egg_sidebar_button_set_editing                 (EggSidebarButton      *button,
								gboolean               editing);
static void     egg_sidebar_button_entry_activate_cb           (GtkWidget             *entry,
								EggSidebarButton      *button);
static gboolean egg_sidebar_button_entry_focus_out_event_cb    (GtkWidget             *widget,
								GdkEventFocus         *event,
								EggSidebarButton      *button);
static void     egg_sidebar_button_drag_data_get_cb            (GtkWidget             *widget,
								GdkDragContext        *context,
								GtkSelectionData      *selection_data,
								guint                  info,
								guint                  time,
								EggSidebarButton      *button);
static void     egg_sidebar_button_drag_begin_cb               (GtkWidget             *widget,
								GdkDragContext        *context,
								EggSidebarButton      *button);


struct EggSidebarButtonPrivate
{
	char *stock_id;
	char *text;
	gboolean is_static;

	GtkItemFactory *popup_factory;
	
	GtkWidget *entry;
	GtkWidget *box;
	
	gboolean editing;
	
	EggSidebar *sidebar;

	GtkWidget *dnd_widget;
	GtkWidget *dnd_image;
	GtkWidget *dnd_label;
};

enum
{
	PROP_0,
	PROP_UNIQUE_ID,
	PROP_STOCK_ID,
	PROP_TEXT,
	PROP_STATIC,
	PROP_SIDEBAR
};

enum
{
	EDITED,
	DELETED,
	LAST_SIGNAL
};

static GObjectClass *parent_class = NULL;

static guint egg_sidebar_button_signals[LAST_SIGNAL] = { 0 };

GType
egg_sidebar_button_get_type (void)
{
	static GType egg_sidebar_button_type = 0;

	if (egg_sidebar_button_type == 0)
	{
		static const GTypeInfo our_info =
		{
			sizeof (EggSidebarButtonClass),
			NULL,
			NULL,
			(GClassInitFunc) egg_sidebar_button_class_init,
			NULL,
			NULL,
			sizeof (EggSidebarButton),
			0,
			(GInstanceInitFunc) egg_sidebar_button_init
		};

		egg_sidebar_button_type = g_type_register_static (GTK_TYPE_RADIO_BUTTON,
								 "EggSidebarButton",
								 &our_info, 0);
	}

	return egg_sidebar_button_type;
}

static void
egg_sidebar_button_class_init (EggSidebarButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = egg_sidebar_button_finalize;

	object_class->set_property = egg_sidebar_button_set_property;
	object_class->get_property = egg_sidebar_button_get_property;

	g_object_class_install_property (object_class,
					 PROP_UNIQUE_ID,
					 g_param_spec_string ("unique_id",
							      "Unique ID",
							      "Unique ID",
							      NULL,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_STOCK_ID,
					 g_param_spec_string ("stock_id",
							      "Stock ID",
							      "Stock icon ID",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_TEXT,
					 g_param_spec_string ("text",
							      "Text",
							      "Text",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_STATIC,
					 g_param_spec_boolean ("static",
							       "Static",
							       "Static",
							       FALSE,
							       G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_SIDEBAR,
					 g_param_spec_object ("sidebar",
							      "Sidebar",
							      "Sidebar object",
							      EGG_TYPE_SIDEBAR,
							      G_PARAM_READWRITE));
	
	egg_sidebar_button_signals[EDITED] =
		g_signal_new ("edited",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EggSidebarButtonClass, edited),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	egg_sidebar_button_signals[DELETED] =
		g_signal_new ("deleted",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EggSidebarButtonClass, deleted),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
}

static void
egg_sidebar_button_init (EggSidebarButton *button)
{
	GtkWidget *dnd_ebox, *dnd_vbox, *dnd_border_ebox;
	GdkColor black = { 0, 0x0000, 0x0000, 0x0000 };

	static GtkItemFactoryEntry popup_menu_items[] =
	{
		{ N_("/_Rename"), NULL, egg_sidebar_button_popup_rename_cb, 0, "<Item>",      NULL             },
		{ N_("/_Delete"), NULL, egg_sidebar_button_popup_delete_cb, 0, "<StockItem>", GTK_STOCK_DELETE }
	};

	static GtkTargetEntry drag_types[] =
	{
		{ "EggSidebarButton", 0, EGG_SIDEBAR_DND_TYPE_BUTTON }
	};
	
	button->priv = g_new0 (EggSidebarButtonPrivate, 1);

	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);

	button->priv->box = gtk_vbox_new (FALSE, 2);
	
	/* image */
	button->image = gtk_image_new ();
	gtk_box_pack_start (GTK_BOX (button->priv->box),
			    button->image,
			    TRUE,
			    TRUE,
			    0);
	
	/* label */
	button->label = gtk_label_new ("");
	gtk_label_set_use_underline (GTK_LABEL (button->label), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (button->label), GTK_WIDGET (button));
	gtk_label_set_line_wrap (GTK_LABEL (button->label), TRUE);
	gtk_box_pack_start (GTK_BOX (button->priv->box),
			    button->label,
			    FALSE,
			    TRUE,
			    0);

	/* entry for editing */
	button->priv->entry = gtk_entry_new ();
	gtk_entry_set_has_frame (GTK_ENTRY (button->priv->entry),
				 FALSE);
	gtk_widget_modify_bg (button->priv->entry, GTK_STATE_ACTIVE,
			      &button->priv->entry->style->bg[GTK_STATE_NORMAL]);
	gtk_widget_modify_bg (button->priv->entry, GTK_STATE_PRELIGHT,
			      &button->priv->entry->style->bg[GTK_STATE_NORMAL]);
	g_signal_connect (G_OBJECT (button->priv->entry),
			  "activate",
			  G_CALLBACK (egg_sidebar_button_entry_activate_cb),
			  button);
	g_signal_connect (G_OBJECT (button->priv->entry),
			  "focus_out_event",
			  G_CALLBACK (egg_sidebar_button_entry_focus_out_event_cb),
			  button);

	gtk_widget_show (button->image);
	gtk_widget_show (button->label);
	gtk_widget_show (button->priv->entry);

	g_object_ref (G_OBJECT (button->priv->entry));
	g_object_ref (G_OBJECT (button->label));

	gtk_container_add (GTK_CONTAINER (button), button->priv->box);

	g_signal_connect (G_OBJECT (button), "button_press_event",
			  G_CALLBACK (egg_sidebar_button_button_press_event_cb),
			  button);
	
	/* dnd */
	g_signal_connect (G_OBJECT (button), "drag_data_get",
			  G_CALLBACK (egg_sidebar_button_drag_data_get_cb), button);
	g_signal_connect (G_OBJECT (button), "drag_begin",
			  G_CALLBACK (egg_sidebar_button_drag_begin_cb), button);
	gtk_drag_source_set (GTK_WIDGET (button), GDK_BUTTON1_MASK,
			     drag_types, G_N_ELEMENTS (drag_types),
			     GDK_ACTION_COPY | GDK_ACTION_MOVE);

	/* popup menu */
	button->priv->popup_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<main>", NULL);
	gtk_item_factory_set_translate_func (button->priv->popup_factory,
					     egg_sidebar_button_item_factory_translate_func,
					     NULL, NULL);
	gtk_item_factory_create_items (button->priv->popup_factory,
				       G_N_ELEMENTS (popup_menu_items),
				       popup_menu_items, button);

	/* dnd widget */
	button->priv->dnd_widget = gtk_window_new (GTK_WINDOW_POPUP);
	dnd_border_ebox = gtk_event_box_new ();
	gtk_widget_modify_bg (dnd_border_ebox, GTK_STATE_NORMAL, &black);
	dnd_ebox = gtk_event_box_new ();
	gtk_widget_modify_bg (dnd_ebox, GTK_STATE_NORMAL, &dnd_ebox->style->base[GTK_STATE_NORMAL]);
	dnd_vbox = gtk_vbox_new (FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (dnd_ebox), 1);
	gtk_container_add (GTK_CONTAINER (dnd_border_ebox), dnd_ebox);
	gtk_container_add (GTK_CONTAINER (dnd_ebox), dnd_vbox);
	gtk_container_add (GTK_CONTAINER (button->priv->dnd_widget), dnd_border_ebox);
	
	button->priv->dnd_image = gtk_image_new ();
	gtk_box_pack_start (GTK_BOX (dnd_vbox),
			    button->priv->dnd_image,
			    TRUE,
			    TRUE,
			    0);
	
	button->priv->dnd_label = gtk_label_new ("");
	gtk_label_set_line_wrap (GTK_LABEL (button->priv->dnd_label), TRUE);
	gtk_box_pack_start (GTK_BOX (dnd_vbox),
			    button->priv->dnd_label,
			    FALSE,
			    TRUE,
			    0);

	gtk_widget_show_all (dnd_border_ebox);
	gtk_widget_realize (button->priv->dnd_widget);
}

static void
egg_sidebar_button_finalize (GObject *object)
{
	EggSidebarButton *button;

	g_return_if_fail (object != NULL);
	g_return_if_fail (EGG_IS_SIDEBAR_BUTTON (object));

	button = EGG_SIDEBAR_BUTTON (object);

	g_return_if_fail (button->priv != NULL);
	
	g_object_unref (G_OBJECT (button->priv->entry));
	g_object_unref (G_OBJECT (button->label));

	gtk_widget_destroy (button->priv->dnd_widget);

	g_object_unref (G_OBJECT (button->priv->popup_factory));

	g_free (button->unique_id);
	g_free (button->priv->stock_id);
	g_free (button->priv->text);

	g_free (button->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
egg_sidebar_button_set_property (GObject *object,
			        guint prop_id,
			        const GValue *value,
			        GParamSpec *pspec)
{
	EggSidebarButton *button = EGG_SIDEBAR_BUTTON (object);
	GdkPixbuf *pixbuf;

	switch (prop_id)
	{
	case PROP_UNIQUE_ID:
		button->unique_id = g_strdup (g_value_get_string (value));
		break;
	case PROP_STOCK_ID:
		if (button->priv->stock_id != NULL)
			g_free (button->priv->stock_id);
		button->priv->stock_id = g_strdup (g_value_get_string (value));
		gtk_image_set_from_stock (GTK_IMAGE (button->image),
					  button->priv->stock_id,
					  GTK_ICON_SIZE_DIALOG);
		gtk_image_set_from_stock (GTK_IMAGE (button->priv->dnd_image),
					  button->priv->stock_id,
					  GTK_ICON_SIZE_DIALOG);
		pixbuf = gtk_widget_render_icon (GTK_WIDGET (button),
						 button->priv->stock_id,
						 GTK_ICON_SIZE_DIALOG,
						 NULL);
		gtk_drag_source_set_icon_pixbuf (GTK_WIDGET (button),
						 pixbuf);
		g_object_unref (G_OBJECT (pixbuf));
		break;
	case PROP_TEXT:
		if (button->priv->text != NULL)
			g_free (button->priv->text);
		button->priv->text = g_strdup (g_value_get_string (value));
		gtk_label_set_text_with_mnemonic (GTK_LABEL (button->label),
						  button->priv->text);
		gtk_label_set_text (GTK_LABEL (button->priv->dnd_label),
				    button->priv->text);
		gtk_entry_set_text (GTK_ENTRY (button->priv->entry),
				    button->priv->text);
		gtk_editable_select_region (GTK_EDITABLE (button->priv->entry), 0, -1);
		g_signal_emit (G_OBJECT (button),
			       egg_sidebar_button_signals[EDITED], 0);
		break;
	case PROP_STATIC:
		button->priv->is_static = g_value_get_boolean (value);
		break;
	case PROP_SIDEBAR:
		button->priv->sidebar = g_value_get_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void 
egg_sidebar_button_get_property (GObject *object,
			        guint prop_id,
				GValue *value,
			        GParamSpec *pspec)
{
	EggSidebarButton *button = EGG_SIDEBAR_BUTTON (object);

	switch (prop_id)
	{
	case PROP_UNIQUE_ID:
		g_value_set_string (value, button->unique_id);
		break;
	case PROP_STOCK_ID:
		g_value_set_string (value, button->priv->stock_id);
		break;
	case PROP_TEXT:
		g_value_set_string (value, button->priv->text);
		break;
	case PROP_STATIC:
		g_value_set_boolean (value, button->priv->is_static);
		break;
	case PROP_SIDEBAR:
		g_value_set_object (value, button->priv->sidebar);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

EggSidebarButton *
egg_sidebar_button_new (const char *unique_id)
{
	EggSidebarButton *button;

	button = EGG_SIDEBAR_BUTTON (g_object_new (EGG_TYPE_SIDEBAR_BUTTON,
						  "unique_id", unique_id,
						  NULL));

	g_return_val_if_fail (button->priv != NULL, NULL);

	return button;
}

void
egg_sidebar_button_set (EggSidebarButton *button,
		       const char *stock_id,
		       const char *text,
		       gboolean is_static)
{
	g_return_if_fail (EGG_IS_SIDEBAR_BUTTON (button));
	g_return_if_fail (stock_id != NULL);
	g_return_if_fail (text != NULL);
	
	g_object_set (G_OBJECT (button),
		      "stock_id", stock_id,
		      "text", text,
		      "static", is_static,
		      NULL);
}

void
egg_sidebar_button_get (EggSidebarButton *button,
		       char **stock_id,
		       char **text,
		       gboolean *is_static)
{
	g_return_if_fail (EGG_IS_SIDEBAR_BUTTON (button));
	
	g_object_get (G_OBJECT (button),
		      "stock_id", stock_id,
		      "text", text,
		      "static", is_static,
		      NULL);
}

static gboolean
egg_sidebar_button_button_press_event_cb (GtkWidget *widget,
					 GdkEventButton *event,
					 EggSidebarButton *button)
{
	if (event->button != 3)
		return FALSE;

	if (button->priv->is_static == TRUE)
		return FALSE;

	gtk_item_factory_popup (button->priv->popup_factory,
				event->x_root,
				event->y_root,
				event->button,
				event->time);

	return TRUE;
}

static char *
egg_sidebar_button_item_factory_translate_func (const char *path,
					       gpointer unused)
{
	return (char *) _(path);
}

static void
egg_sidebar_button_popup_rename_cb (EggSidebarButton *button,
				   guint action,
				   GtkWidget *widget)
{
	egg_sidebar_button_set_editing (button, TRUE);
}

static void
egg_sidebar_button_popup_delete_cb (EggSidebarButton *button,
				   guint action,
				   GtkWidget *widget)
{
	g_signal_emit (G_OBJECT (button),
		       egg_sidebar_button_signals[DELETED], 0);
}

static void
egg_sidebar_button_set_editing (EggSidebarButton *button,
			       gboolean editing)
{
	if (editing == button->priv->editing)
		return;

	button->priv->editing = editing;
	
	if (editing == FALSE)
	{
		gtk_container_remove (GTK_CONTAINER (button->priv->box),
				      button->priv->entry);
		gtk_box_pack_start (GTK_BOX (button->priv->box),
				    button->label,
				    FALSE,
				    TRUE,
				    0);
	}
	else
	{
		gtk_container_remove (GTK_CONTAINER (button->priv->box),
				      button->label);
		gtk_box_pack_start (GTK_BOX (button->priv->box),
				    button->priv->entry,
				    FALSE,
				    TRUE,
				    0);
		gtk_widget_grab_focus (button->priv->entry);
	}
}

static void
egg_sidebar_button_entry_activate_cb (GtkWidget *entry,
				     EggSidebarButton *button)
{
	char *text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);

	egg_sidebar_button_set_editing (button, FALSE);

	g_object_set (G_OBJECT (button),
		      "text", text,
		      NULL);

	g_free (text);
}

static gboolean
egg_sidebar_button_entry_focus_out_event_cb (GtkWidget *widget,
					    GdkEventFocus *event,
					    EggSidebarButton *button)
{
	egg_sidebar_button_set_editing (button, FALSE);

	return FALSE;
}

static void
egg_sidebar_button_drag_data_get_cb (GtkWidget *widget,
				    GdkDragContext *context,
				    GtkSelectionData *selection_data,
				    guint info, guint time,
				    EggSidebarButton *button)
{
	switch (info)
	{
	case EGG_SIDEBAR_DND_TYPE_BUTTON:
		gtk_selection_data_set (selection_data, selection_data->target,
					8, button->unique_id, strlen (button->unique_id));
		break;
	default:
		g_warning ("Unknown DND type");
		break;
	}
}

static void
egg_sidebar_button_drag_begin_cb (GtkWidget *widget,
				 GdkDragContext *context,
				 EggSidebarButton *button)
{
	gtk_widget_set_size_request (button->priv->dnd_widget,
				     GTK_WIDGET (button)->allocation.width,
				     GTK_WIDGET (button)->allocation.height);
	gtk_drag_set_icon_widget (context, button->priv->dnd_widget, -2, -2);
}
