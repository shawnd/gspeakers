/*
  $Id$
  
  crossoverpaned Copyright (C) 2002 Daniel Sundberg

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

#include "crossoverpaned.h"

#define MENU_INDEX_SAVE 6
#define TOOLBAR_INDEX_SAVE 4

CrossoverPaned::CrossoverPaned()
{
  m_tbar = NULL;
  g_settings.settings_changed.connect(slot(*this, &CrossoverPaned::on_settings_changed));
  
  add1(crossover_wizard);
  g_settings.defaultValueUnsignedInt("CrossoverPaned1Position", 220);
  set_position(g_settings.getValueUnsignedInt("CrossoverPaned1Position"));
  add2(m_hpaned2);
  m_hpaned2.add1(crossover_treeview);
  m_vpaned.add1(m_plot_notebook);
  m_plot_notebook.append_page(filter_plot, "Crossover freq resp");
  m_plot_notebook.append_page(total_filter_plot, "Total crossover freq resp");
  m_vpaned.add2(crossover_history);
  m_hpaned2.add2(m_vpaned);
  g_settings.defaultValueUnsignedInt("CrossoverPaned2Position", 220);
  m_hpaned2.set_position(g_settings.getValueUnsignedInt("CrossoverPaned2Position"));
  g_settings.defaultValueUnsignedInt("CrossoverPlotVPanedPosition", 220);
  m_vpaned.set_position(g_settings.getValueUnsignedInt("CrossoverPlotVPanedPosition"));
  
  signal_crossover_set_save_state.connect( slot(*this, &CrossoverPaned::set_save_state) );
}

void CrossoverPaned::select_first_crossover()
{
  crossover_history.select_first_row();
}

CrossoverPaned::~CrossoverPaned()
{
  g_settings.setValue("CrossoverPaned1Position", get_position());
  g_settings.setValue("CrossoverPaned2Position", m_hpaned2.get_position());
  g_settings.setValue("CrossoverPlotVPanedPosition", m_vpaned.get_position());
  g_settings.save();
}

Gtk::Menu& CrossoverPaned::get_menu()
{
  Gtk::Menu::MenuList& menulist = m_menu.items();
  Gtk::Menu *new_crossover_submenu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList& sub_menulist = new_crossover_submenu->items();
  
  /* New crossover submenu */
  //sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("_New Driver", GSpeakers::image_widget("stock_new_driver_16.png"), 
  //                  slot(*this, &CrossoverHistory::on_new) ) );
  //sub_menulist.push_back( Gtk::Menu_Helpers::SeparatorElem() );
  sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New _highpass crossover", 
                            GSpeakers::image_widget("stock_new_crossover_16.png"), 
                            bind<int>(slot(*this, &CrossoverPaned::on_new_crossover_menu_action), CROSSOVER_TYPE_LOWPASS) ) );
  sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New _subsonic crossover", 
                            GSpeakers::image_widget("stock_new_crossover_16.png"),
                            bind<int>(slot(*this, &CrossoverPaned::on_new_crossover_menu_action) , CROSSOVER_TYPE_SUBSONIC) ) );
  sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New _highpass crossover", 
                            GSpeakers::image_widget("stock_new_crossover_16.png"),
                            bind<int>(slot(*this, &CrossoverPaned::on_new_crossover_menu_action), CROSSOVER_TYPE_HIGHPASS) ) );
  sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New _2-way crossover", 
                            GSpeakers::image_widget("stock_new_crossover_16.png"),
                            bind<int>(slot(*this, &CrossoverPaned::on_new_crossover_menu_action), CROSSOVER_TYPE_TWOWAY) ) );
  sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New 2._5-way crossover", 
                            GSpeakers::image_widget("stock_new_crossover_16.png"),
                            bind<int>(slot(*this, &CrossoverPaned::on_new_crossover_menu_action), CROSSOVER_TYPE_LOWPASS | CROSSOVER_TYPE_TWOWAY) ) );
  sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New _3-way crossover", 
                            GSpeakers::image_widget("stock_new_crossover_16.png"),
                            bind<int>(slot(*this, &CrossoverPaned::on_new_crossover_menu_action), CROSSOVER_TYPE_THREEWAY) ) );
  sub_menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New _4-way crossover", 
                            GSpeakers::image_widget("stock_new_crossover_16.png"),
                            bind<int>(slot(*this, &CrossoverPaned::on_new_crossover_menu_action), CROSSOVER_TYPE_FOURWAY) ) );
  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("_New Crossover", GSpeakers::image_widget("stock_new_crossover_16.png"), 
                                                       *new_crossover_submenu) );
  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New _Copy", GSpeakers::image_widget("stock_new_crossover_copy_16.png"), 
                                                       slot(crossover_history, &CrossoverHistory::on_new_copy)) );
  menulist.push_back( Gtk::Menu_Helpers::SeparatorElem() );
  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("New Crossover _Xml", GSpeakers::image_widget("stock_new_crossover_xml_16.png"), 
                                                       slot(crossover_history, &CrossoverHistory::on_new_xml)) );
  menulist.push_back( Gtk::Menu_Helpers::MenuElem("Append Crossover _Xml..", slot(crossover_history, &CrossoverHistory::on_append_xml)) );

  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("_Open Xml", GSpeakers::image_widget("open_xml_16.png"),  
                                                        slot(crossover_history, &CrossoverHistory::on_open_xml)) );
  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("_Save Xml", GSpeakers::image_widget("save_xml_16.png"),  
                                                        slot(crossover_history, &CrossoverHistory::on_save)) );
  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("Save Xml _As", GSpeakers::image_widget("save_as_xml_16.png"),  
                                                        slot(crossover_history, &CrossoverHistory::on_save_as)) );
  menulist.push_back( Gtk::Menu_Helpers::SeparatorElem() );
  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("_Delete Crossover", GSpeakers::image_widget("delete_crossover_16.png"),  
                                                        slot(crossover_history, &CrossoverHistory::on_remove)) );
  menulist.push_back( Gtk::Menu_Helpers::SeparatorElem() );
  menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem("_Plot Crossover", GSpeakers::image_widget("stock_plot_crossover_16.png"), 
                                                       slot(*this, &CrossoverPaned::on_plot_crossover)) );
  
  //menulist.push_back( Gtk::Menu_Helpers::MenuElem("Update crossover", slot(*this, &MainWindow::on_update_crossover)) );
  //menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Plot crossover", slot(*this, &MainWindow::on_plot_crossover)) );


  menulist[MENU_INDEX_SAVE].set_sensitive(false);
  //menulist[MENU_INDEX_DELETE].set_sensitive(false);
  return m_menu;
}



Gtk::Widget& CrossoverPaned::get_toolbar()
{
  if (m_tbar == NULL) {
    m_tbar = manage(new Gtk::Toolbar());
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::ButtonElem("New Copy", GSpeakers::image_widget("stock_new_crossover_copy_24.png"), 
                             slot(crossover_history, &CrossoverHistory::on_new_copy), "Create new crossover copy") );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::Space() );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::ButtonElem("New Xml", GSpeakers::image_widget("stock_new_crossover_xml_24.png"), 
                             slot(crossover_history, &CrossoverHistory::on_new_xml), "Create new crossover xml (list)") );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::ButtonElem("Open Xml", GSpeakers::image_widget("open_xml_24.png"),
                             slot(crossover_history, &CrossoverHistory::on_open_xml), "Open crossover xml (list)") );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::ButtonElem("Save Xml", GSpeakers::image_widget("save_xml_24.png"),   
                             slot(crossover_history, &CrossoverHistory::on_save), "Save crossover xml (list)") );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::Space() );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::ButtonElem("Delete Crossover", GSpeakers::image_widget("delete_crossover_24.png"),
                             slot(crossover_history, &CrossoverHistory::on_remove), "Delete currently selected crossover") );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::Space() );
    m_tbar->tools().push_back( Gtk::Toolbar_Helpers::ButtonElem("Plot Crossover", GSpeakers::image_widget("stock_plot_crossover_24.png"), 
                             slot(*this, &CrossoverPaned::on_plot_crossover), "Plot frequency response for currently selected crossover") );

    m_toolbar.add(*m_tbar);
    m_tbar->set_toolbar_style((Gtk::ToolbarStyle)g_settings.getValueUnsignedInt("ToolbarStyle"));
    m_tbar->tools()[TOOLBAR_INDEX_SAVE].get_widget()->set_sensitive(false);
    //tbar->tools()[TOOLBAR_INDEX_DELETE].get_widget()->set_sensitive(false);
    g_settings.settings_changed.connect(slot(*this, &CrossoverPaned::on_settings_changed));
  }
  return m_toolbar;
}

void CrossoverPaned::on_plot_crossover()
{
  filter_plot.clear();
  signal_plot_crossover();
}

void CrossoverPaned::on_settings_changed(const string& s)
{
  if (s == "ToolbarStyle") {
    m_tbar->set_toolbar_style((Gtk::ToolbarStyle)g_settings.getValueUnsignedInt("ToolbarStyle"));
  }
}

void CrossoverPaned::on_new_crossover_menu_action(int i)
{
  signal_new_crossover(i);
}


void CrossoverPaned::set_save_state(bool b)
{
  m_tbar->tools()[TOOLBAR_INDEX_SAVE].get_widget()->set_sensitive(b);
  m_menu.items()[MENU_INDEX_SAVE].set_sensitive(b);
}
