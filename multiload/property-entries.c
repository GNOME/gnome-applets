#include <global.h>

/* =======================================================================
 * Multiple adjustments.
 * =======================================================================
 */

typedef struct
{
	gint num_items, total_size;
	glong *values, *temp_values;
} AdjustmentBlockProperties;

typedef struct
{
	glong *value;
	GtkAdjustment *adjustment;
	GnomePropertyObject *object;
} AdjustmentCbData;

static void
_property_entry_adjustments_load_temp (GnomePropertyObject *object)
{
	AdjustmentBlockProperties *prop_ptr = object->prop_data;

	memcpy (prop_ptr->temp_values, prop_ptr->values, prop_ptr->total_size);
}

static gint
_property_entry_adjustments_save_temp (GnomePropertyObject *object)
{
	AdjustmentBlockProperties *prop_ptr = object->prop_data;

	memcpy (prop_ptr->values, prop_ptr->temp_values, prop_ptr->total_size);

	return FALSE;
}

static void
_property_entry_adjustment_changed_cb (GtkWidget *widget,
				       AdjustmentCbData *cb_data)
{
	*(cb_data->value) = cb_data->adjustment->value;

	gnome_property_object_changed (cb_data->object);
	return;
	widget = NULL;
}

GtkWidget *
gnome_property_entry_adjustments (GnomePropertyObject *object,
				  const gchar *label, gint num_items,
				  gint columns, gint internal_columns,
				  gint *table_pos, const gchar *texts [],
				  glong *data_ptr, glong *values)
{
	GtkWidget *frame, *table;
	GnomePropertyDescriptor *descriptor;
	GnomePropertyObject *entry_object;
	AdjustmentBlockProperties *prop_ptr;
	glong *temp_values;
	gint rows, i;

	frame = gtk_frame_new (_(label));

	rows = (num_items < columns) ? 1 : (num_items+columns-1) / columns;

	temp_values = g_memdup (values, num_items * sizeof (glong));

	table = gtk_table_new (rows, columns * internal_columns, TRUE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD << 2);
	gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);

	for (i = 0; i < num_items; i++) {
		glong *data = &(data_ptr [i*8]);
		AdjustmentCbData *cb_data;
		GtkObject *adjustment;
		GtkWidget *spin, *lb;
		gint row, col;

		spin = gtk_spin_button_new (NULL, data [0], data [1]);
		adjustment = gtk_adjustment_new (data [2], data[3], data [4],
						 data [5], data[6], data [7]);
		gtk_spin_button_set_adjustment
			(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
		gtk_spin_button_set_value
			(GTK_SPIN_BUTTON (spin), values [i]);

		cb_data = g_new0 (AdjustmentCbData, 1);
		cb_data->adjustment = GTK_ADJUSTMENT (adjustment);
		cb_data->value = &(temp_values [i]);
		cb_data->object = object;

		gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", 
				    _property_entry_adjustment_changed_cb,
				    cb_data);

		col = table_pos ? table_pos [i] : i;
		row = col / columns;
		col %= columns;
		col *= internal_columns;

		lb = gtk_label_new (_(texts [i]));
		gtk_misc_set_alignment (GTK_MISC (lb), 0.0, 0.5);

		gtk_table_attach_defaults
		    (GTK_TABLE (table), lb, col, col+internal_columns-1,
		     row, row+1);
		
		gtk_table_attach_defaults
		    (GTK_TABLE (table), spin, col+internal_columns-1,
		     col+internal_columns, row, row+1);
	}

	gtk_container_add (GTK_CONTAINER (frame), table);

	descriptor = g_new0 (GnomePropertyDescriptor, 1);
	descriptor->size = sizeof (AdjustmentBlockProperties);

	descriptor->load_temp_func = _property_entry_adjustments_load_temp;
	descriptor->save_temp_func = _property_entry_adjustments_save_temp;

	prop_ptr = g_new0 (AdjustmentBlockProperties, 1);
	prop_ptr->total_size = num_items * sizeof (glong);
	prop_ptr->num_items = num_items;
	prop_ptr->temp_values = temp_values;
	prop_ptr->values = values;

	entry_object = gnome_property_object_new (descriptor, prop_ptr);
	entry_object->user_data = object;

	object->object_list = g_list_append (object->object_list, entry_object);

	g_free (descriptor);
	
	return frame;
}
