/* eel-extension.h - Copy & paste code from libeel
 *
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#define EEL_TRASH_URI "trash:"

/* GtkButton */
GtkWidget*            
eel_gtk_button_new_with_stock_icon (const gchar *label,
		                    const gchar *stock_id);

void 
eel_gtk_button_set_padding (GtkButton *button,
		            int pad_amount);
