/*
  boxeditor Copyright (C) 2002 Daniel Sundberg

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

#ifndef __GSPEAKERS_BOXEDITOR
#define __GSPEAKERS_BOXEDITOR

#include <gtkmm.h>
#include "box.h"
#include "speakerlist.h"
#include "gspeakerscolor.h"

using namespace std;
using namespace SigC;

/*
 * This class will contain the control panel where you can adjust 
 * settings for the enclosure you're working with.
 */
class BoxEditor : public Gtk::Window
{
public:
  BoxEditor();
  ~BoxEditor();
  
protected:
  void on_optimize_button_clicked();
  void on_append_to_boxlist_clicked();
  void on_append_to_plot_clicked();
  void on_calc_port_clicked();

  /* Callbacks */
  void on_box_selected(Box *b);
  void on_speaker_list_loaded(string);
  void on_combo_entry_changed();
  void on_box_data_changed(int i);
  bool on_delete_event(GdkEventAny *event);

  /* Member widgets */
  Gtk::Table m_table;
  Gtk::VBox m_vbox;
  Gtk::HBox m_hbox;
  Gtk::Combo m_bass_speaker_combo;
  Gtk::Entry m_speaker_qts_entry, m_speaker_vas_entry, m_speaker_fs_entry;
  Gtk::Entry m_id_string_entry, m_vb1_entry, m_fb1_entry, m_vb2_entry, m_fb2_entry;
  Gtk::OptionMenu m_box_type_optionmenu;
  Gtk::Button m_optimize_button, m_append_button, m_plot_button, m_calc_port_button;
  Gtk::Menu m_option_menu;

private: 
  Box *m_box;
  SpeakerList m_speaker_list;
  Speaker m_current_speaker;
  bool disable_signals;
  GSpeakersColor m_color_list;
};

#endif
