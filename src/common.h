// common.h
//
// Copyright (C) 2012 - Sebastian Geiger
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

typedef struct {
    GtkWidget    *tasks;
    GtkWidget    *applet;
    GtkWidget    *title;
    GSettings    *settings;
} WinPickerApp;

/** 
 * The window picker app contains our reference to GSettings, putting it into
 * a common header file allow us to share it accross the different classes
 * of window picker.
 */
extern WinPickerApp *mainapp;
