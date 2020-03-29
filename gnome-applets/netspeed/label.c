/*
 * Copyright (C) 2015 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 */

#include "label.h"

struct _NetspeedLabel
{
	GtkLabel parent;

  gboolean dont_shrink;
	gint     width;
};

G_DEFINE_TYPE (NetspeedLabel, netspeed_label, GTK_TYPE_LABEL)

static void
netspeed_label_size_allocate (GtkWidget     *widget,
                              GtkAllocation *allocation)
{
  NetspeedLabel *label;

  label = NETSPEED_LABEL (widget);

  if (allocation->width > label->width)
    label->width = allocation->width;

  GTK_WIDGET_CLASS (netspeed_label_parent_class)->size_allocate (widget, allocation);
}

static void
netspeed_label_get_preferred_width (GtkWidget *widget,
                                    gint      *minimum_width,
                                    gint      *natural_width)
{
  NetspeedLabel *label;

  label = NETSPEED_LABEL (widget);

  GTK_WIDGET_CLASS (netspeed_label_parent_class)->get_preferred_width (widget,
                                                                       minimum_width,
                                                                       natural_width);

  if (label->dont_shrink && (*minimum_width < label->width || *natural_width < label->width))
    *minimum_width = *natural_width = label->width;
}

static void
netspeed_label_class_init (NetspeedLabelClass *label_class)
{
	GtkWidgetClass *widget_class;

	widget_class = GTK_WIDGET_CLASS (label_class);

	widget_class->size_allocate = netspeed_label_size_allocate;
	widget_class->get_preferred_width = netspeed_label_get_preferred_width;
}

static void
netspeed_label_init (NetspeedLabel *label)
{
}

GtkWidget *
netspeed_label_new (void)
{
  return g_object_new (NETSPEED_TYPE_LABEL, NULL);
}

void
netspeed_label_set_dont_shrink (NetspeedLabel *label,
                                gboolean       dont_shrink)
{
  g_return_if_fail (NETSPEED_IS_LABEL (label));
  label->dont_shrink = dont_shrink;
}
