/* eel-extension.h - Copy & paste code from libeel
 *
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License VERSION 2 as
 * published by the Free Software Foundation.  You are not allowed to
 * use any other version of the license; unless you got the explicit
 * permission from the author to do so.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gtk/gtk.h>

/**
 * eel_gtk_button_new_with_stock_icon:
 *
 * @label: the button label, which may include a mnemonic accelerator.
 * @stock_id: the stock icon to use.
 *
 * Creates a GtkButton with a mnemonic label and a stock icon.
 *
 * Return value: a new GtkButton widget.
 **/
GtkWidget*
eel_gtk_button_new_with_stock_icon (const gchar *label, const gchar *stock_id)
{
        GtkWidget *button, *l, *image, *hbox, *align;

        /* This is mainly copied from gtk_button_construct_child(). */
        button = gtk_button_new ();
        l = gtk_label_new_with_mnemonic (label);
        gtk_label_set_mnemonic_widget (GTK_LABEL (l), GTK_WIDGET (button));
        image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
        hbox = gtk_hbox_new (FALSE, 2);
        align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
        gtk_box_pack_end (GTK_BOX (hbox), l, FALSE, FALSE, 0);
        gtk_container_add (GTK_CONTAINER (button), align);
        gtk_container_add (GTK_CONTAINER (align), hbox);
        gtk_widget_show_all (align);

        return button;
}

