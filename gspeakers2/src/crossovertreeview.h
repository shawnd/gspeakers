/*
  crossovertreeview Copyright (C) 2002 Daniel Sundberg

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

#ifndef __GSPEAKERS_CROSSOVERTREEVIEW
#define __GSPEAKERS_CROSSOVERTREEVIEW

#include <gtkmm.h>
#include "crossover.h"
#include "cellitemcrossover.h"

using namespace SigC;
using namespace std;

/*
 * This is a TreeView that displays the currently selected crossover
 */
class CrossoverTreeView : public Gtk::Window {
public:
  CrossoverTreeView();
  virtual ~CrossoverTreeView();
  /* Callback */
  void on_crossover_selected(SigC::Object *new_crossover); // SigC::Object&);
  
protected:
  //vfunc overrides:
  virtual void on_realize();
  
  /* callbacks */
  void on_cell_edited_value(const Glib::ustring& path_string, const Glib::ustring& new_text);
  void on_cell_edited_unit(const Glib::ustring& path_string, const Glib::ustring& new_text);
  bool on_delete_event(GdkEventAny *event);

  /* Helper functions */
  virtual void create_model();
  virtual void add_columns();
  virtual void add_items();
  virtual void treestore_add_item(const CellItem_Crossover& foo);
  
  //Member widgets:
  Gtk::VBox m_VBox;
  Gtk::ScrolledWindow m_ScrolledWindow;
  Gtk::TreeView m_TreeView;
  Glib::RefPtr<Gtk::TreeStore> m_refTreeStore;
  vector<CellItem_Crossover> m_vecItems;
  
  /* Pointer to currently selected crossover */
  Crossover *cover;

  struct ModelColumns : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn<Glib::ustring>  id_string;
    Gtk::TreeModelColumn<int>            id;
    Gtk::TreeModelColumn<int>            type;
    Gtk::TreeModelColumn<double>         value;
    Gtk::TreeModelColumn<Glib::ustring>  unit;
    Gtk::TreeModelColumn<bool>           editable;
    Gtk::TreeModelColumn<bool>           visible;
    ModelColumns();
  };

  const ModelColumns m_columns;

};

#endif
