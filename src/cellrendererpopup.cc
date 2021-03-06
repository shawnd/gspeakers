/*
  $Id$

  Copyright (C) 1998-2002 The gtkmm Development Team & Daniel Sundberg
  
  The file is pretty much ripped right out of the gtkmm-2.0 
  example custom cellrenderer with small modifications by 
  Daniel Sundberg <dss@home.se>.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "cellrendererpopup.h"
#include "popupentry.h"
#include <gdkmm/general.h>
#include <gtk/gtkmain.h>
#include <algorithm>
#include <memory>
#include <iostream>

using namespace std;
using namespace sigc;

namespace
{

bool grab_on_window(const Glib::RefPtr<Gdk::Window>& window, guint32 activate_time)
{
  cout << "grab_on_window" << endl;
/*
  if(gdk_pointer_grab(
        Glib::unwrap(window), true,
        GdkEventMask(GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK),
        0, 0, activate_time) == 0)
  {
    if(gdk_keyboard_grab(Glib::unwrap(window), true, activate_time) == 0)
      return true;
    else
      gdk_pointer_ungrab(activate_time);
  }
*/
  //return false;
  return true;
}

} // anonymous namespace


CellRendererPopup::CellRendererPopup()
:
  Glib::ObjectBase      (typeid(CellRendererPopup)),
  Gtk::CellRendererText (),
  button_width_         (-1),
  popup_window_         (Gtk::WINDOW_POPUP),
  focus_widget_         (0),
  popup_entry_          (0),
  shown_                (false),
  editing_canceled_     (false)
{
  cout << "CellRendererPopup::CellRendererPopup" << endl;
  
  signal_show_popup_.connect(mem_fun(*this, &Self::on_show_popup));
  signal_hide_popup_.connect(mem_fun(*this, &Self::on_hide_popup));

  popup_window_.signal_button_press_event().connect(mem_fun(*this, &Self::on_button_press_event));
  popup_window_.signal_key_press_event   ().connect(mem_fun(*this, &Self::on_key_press_event));
  popup_window_.signal_style_changed     ().connect(mem_fun(*this, &Self::on_style_changed));
}

CellRendererPopup::~CellRendererPopup()
{}

PopupEntry* CellRendererPopup::get_popup_entry()
{
  return popup_entry_;
}

Gtk::Window* CellRendererPopup::get_popup_window()
{
  return &popup_window_;
}

void CellRendererPopup::set_focus_widget(Gtk::Widget& focus_widget)
{
  focus_widget_ = &focus_widget;
}

Gtk::Widget* CellRendererPopup::get_focus_widget()
{
  return focus_widget_;
}

signal5<void,const Glib::ustring&,int,int,int,int>& CellRendererPopup::signal_show_popup()
{
  return signal_show_popup_;
}

signal0<void>& CellRendererPopup::signal_hide_popup()
{
  return signal_hide_popup_;
}

void CellRendererPopup::hide_popup()
{
  signal_hide_popup_();
}

void CellRendererPopup::get_size_vfunc(Gtk::Widget& widget,
                                       const Gdk::Rectangle* cell_area,
                                       int* x_offset, int* y_offset,
                                       int* width,    int* height)
{
  //cout << "CellRendererPopup::get_size_vfunc" << endl;
  Gtk::CellRendererText::get_size_vfunc(widget, cell_area, x_offset, y_offset, width, height);

  // We cache this becuase it takes really long to get the width.
  if(button_width_ < 0)
    button_width_ = PopupEntry::get_button_width();

  if(width)
    *width += button_width_;
}

Gtk::CellEditable* CellRendererPopup::start_editing_vfunc(GdkEvent*,
                                                          Gtk::Widget&,
                                                          const Glib::ustring& path,
                                                          const Gdk::Rectangle&,
                                                          const Gdk::Rectangle&,
                                                          Gtk::CellRendererState)
{
  cout << "CellRendererPopup::start_editing_vfunc" << endl;

  // If the cell isn't editable we return 0.
  if(!property_editable())
    return 0;

  std::auto_ptr<PopupEntry> popup_entry (new PopupEntry(path));

  popup_entry->signal_editing_done ().connect(mem_fun(*this, &Self::on_popup_editing_done));
  //popup_entry->signal_arrow_clicked().connect(mem_fun(*this, &Self::on_popup_arrow_clicked));
  popup_entry->signal_hide         ().connect(mem_fun(*this, &Self::on_popup_hide));

  cout << property_text() << endl;

  popup_entry->set_text(property_text());
  popup_entry->show();

  // Release auto_ptr<> ownership, and let gtkmm manage the widget.
  popup_entry_ = Gtk::manage(popup_entry.release());

  return popup_entry_;
}

void CellRendererPopup::on_show_popup(const Glib::ustring& path, int x1, int y1, int x2, int y2)
{
  cout << "CellRendererPopup::on_show_popup" << endl;
  // I'm not sure this is ok to do, but we need to show the window to be
  // able to get the allocation right.
  popup_window_.move(-500, -500);
  popup_window_.show();

  const Gtk::Allocation alloc = popup_window_.get_allocation();

  int x = x2;
  int y = y2;

  const int button_height = y2 - y1;

  int       screen_height = Gdk::screen_height() - y;
  const int screen_width  = Gdk::screen_width();

  // Check if it fits in the available height.
  if(alloc.get_height() > screen_height)
  {
    // It doesn't fit, so we see if we have the minimum space needed.
    if((alloc.get_height() > screen_height) && (y - button_height > screen_height))
    {
      // We don't, so we show the popup above the cell instead of below it.
      screen_height = y - button_height;
      y -= (alloc.get_height() + button_height);
      y = std::max(0, y);
    }
  }

  // We try to line it up with the right edge of the column, but we don't
  // want it to go off the edges of the screen.
  x = std::min(x, screen_width);

  x -= alloc.get_width();
  x = std::max(0, x);

  gtk_grab_add(popup_window_.Gtk::Widget::gobj());

  popup_window_.move(x, y);
  popup_window_.show();

  shown_ = true;

  if(focus_widget_)
    focus_widget_->grab_focus();

  grab_on_window(popup_window_.get_window(), gtk_get_current_event_time());
}

void CellRendererPopup::on_hide_popup()
{
  cout << "CellRendererPopup::on_hide_popup" << endl;
  gtk_grab_remove(popup_window_.Gtk::Widget::gobj());
  popup_window_.hide();

  if(popup_entry_)
    popup_entry_->editing_done();

  // This may look weird (the test), but the weak pointer will actually
  // be nulled out for some cells, like the date cell.
  if (popup_entry_)
    popup_entry_->remove_widget();

  shown_ = false;
  editing_canceled_ = false;
}

bool CellRendererPopup::on_button_press_event(GdkEventButton* event)
{
    cout << "CellRendererPopup::on_button_press" << endl;
  
  if(event->button != 1)
    return false;

  // If the event happened outside the popup, cancel editing.

  //gdk_event_get_root_coords ((GdkEvent *) event, &x, &y);
  const double x = event->x_root;
  const double y = event->y_root;

  int xoffset = 0, yoffset = 0;
  popup_window_.get_window()->get_root_origin(xoffset, yoffset);

  const Gtk::Allocation alloc = popup_window_.get_allocation();

  xoffset += alloc.get_x();
  yoffset += alloc.get_y();

  const int x1 = alloc.get_x() + xoffset;
  const int y1 = alloc.get_y() + yoffset;
  const int x2 = x1 + alloc.get_width();
  const int y2 = y1 + alloc.get_height();

  if(x > x1 && x < x2 && y > y1 && y < y2)
    return false;

  editing_canceled_ = true;
  signal_hide_popup_();
  
  return false;
}

bool CellRendererPopup::on_key_press_event(GdkEventKey* event)
{
  cout << "CellRendererPopup::on_key_press" << endl;
  switch(event->keyval)
  {
    case GDK_Escape:
      editing_canceled_ = true; break;

    case GDK_Return:
    case GDK_KP_Enter:
    case GDK_ISO_Enter:
    case GDK_3270_Enter:
      editing_canceled_ = false; break;

    default:
      return true;
  }

  signal_hide_popup_();

  return true;
}

void CellRendererPopup::on_style_changed(const Glib::RefPtr<Gtk::Style>&)
{
  // Invalidate the cache.
  button_width_ = -1;
}

void CellRendererPopup::on_popup_editing_done()
{
  cout << "CellRendererPopup::on_popup_editing_done" << endl;
  if(editing_canceled_ || popup_entry_->get_editing_canceled())
    return;

  edited(popup_entry_->get_path(), popup_entry_->get_text());
}

void CellRendererPopup::on_popup_arrow_clicked()
{
  cout << "CellRendererPopup::on_popup_arrow_clicked" << endl;
  if(shown_)
  {
    editing_canceled_ = true;
    signal_hide_popup_();
    return;
  }

  if(!grab_on_window(popup_entry_->get_window(), gtk_get_current_event_time()))
    return;

  popup_entry_->select_region(0, 0);

  int x = 0, y = 0;
  popup_entry_->get_window()->get_origin(x, y);

  const Gtk::Allocation alloc = popup_entry_->get_allocation();

  signal_show_popup_(popup_entry_->get_path(), x, y, x + alloc.get_width(), y + alloc.get_height());
}

void CellRendererPopup::on_popup_hide()
{
  cout << "CellRendererPopup::on_popup_hide" << endl;
  popup_entry_ = 0;
}

