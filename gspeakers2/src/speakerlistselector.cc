/*
  speakerselector Copyright (C) 2002 Daniel Sundberg

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

#include "speakerlistselector.h"
#include "speakereditor.h"
#include "common.h"
#include "../config.h"

SpeakerListSelector::SpeakerListSelector() :
  m_SpeakerXmlFilenameEntry(),
  m_OpenButton("Open..."), 
  m_EditButton("Edit speakers..."), 
  m_hbox1(), 
  m_hbox2(),
  m_vbox()
{
  Gtk::Label *l = manage(new Gtk::Label("Speaker xml: "));
  m_hbox1.pack_start(*l, Gtk::FILL);
  m_hbox1.pack_start(m_SpeakerXmlFilenameEntry);
  m_hbox2.pack_end(m_EditButton, Gtk::FILL);
  m_hbox2.pack_end(m_OpenButton, Gtk::FILL);
  
  m_vbox.pack_start(m_hbox1);
  m_vbox.pack_start(m_hbox2);
  m_hbox1.set_spacing(5);
  m_hbox2.set_spacing(5);
  m_vbox.set_spacing(5);
  add(m_vbox);
  
  set_border_width(8);
  set_title("Select speaker xml");
  
  m_SpeakerXmlFilenameEntry.set_sensitive(false);
  m_OpenButton.signal_clicked().connect(slot(*this, &SpeakerListSelector::on_open));
  m_EditButton.signal_clicked().connect(slot(*this, &SpeakerListSelector::on_edit_speakers));
  
  show_all();
  m_EditButton.set_sensitive(false);
  speaker_liststore = NULL;
  
  f_open = NULL;
  signal_speakerlist_loaded.connect(slot(*this, &SpeakerListSelector::on_speakerlist_loaded));

/* Fix some initstuff since we don't have any settings */
  f_open = new Gtk::FileSelection("Open speaker xml");
  f_open->set_filename(string(GSPEAKERS_PREFIX) + "/share/xml/vifa.xml");
  f_open->get_ok_button()->signal_clicked().connect(bind<Gtk::FileSelection *>(slot(*this, &SpeakerListSelector::on_open_ok), f_open));
  f_open->get_cancel_button()->signal_clicked().connect(slot(*f_open, &Gtk::Widget::hide));
  on_open_ok(f_open);
  
}

SpeakerListSelector::~SpeakerListSelector()
{
  if (f_open != NULL) delete f_open;
  if (speaker_liststore != NULL) delete speaker_liststore;
}
  
void SpeakerListSelector::on_open()
{
  cout << "on open" << endl;
  if (f_open == NULL) {
    f_open = new Gtk::FileSelection("Open speaker xml");
    f_open->get_ok_button()->signal_clicked().connect(bind<Gtk::FileSelection *>(slot(*this, &SpeakerListSelector::on_open_ok), f_open));
    f_open->get_cancel_button()->signal_clicked().connect(slot(*f_open, &Gtk::Widget::hide));
  } else {
    f_open->show();
    f_open->set_title("Open speaker xml");
  }
  f_open->run();
}

void SpeakerListSelector::on_open_ok(Gtk::FileSelection *f)
{
  try {
    m_speaker_list = SpeakerList(f->get_filename());
    signal_speakerlist_loaded(f->get_filename());
    f_open->hide();
  } catch (GSpeakersException e) {
    Gtk::MessageDialog m(e.what(), Gtk::MESSAGE_ERROR);
    m.run();
  }
}

void SpeakerListSelector::on_speakerlist_loaded(string speaker_list_filename)
{
  set_title("[" + speaker_list_filename + "]");
  m_speaker_list = SpeakerList(speaker_list_filename);
  m_SpeakerXmlFilenameEntry.set_text(speaker_list_filename);
  m_EditButton.set_sensitive(true);
}

void SpeakerListSelector::on_edit_speakers()
{
  cout << "on edit" << endl;
  speaker_liststore = new Speaker_ListStore(&m_speaker_list, m_SpeakerXmlFilenameEntry.get_text());

}
  
