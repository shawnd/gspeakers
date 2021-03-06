/* $Id$
 * 
 * Handle loading/parsing/saving of settings to
 * a configuration file
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * Modified by Daniel Sundberg 2002
 *
 *   - added save()
 *   - added virtual destructor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <map>
#include <sigc++/sigc++.h>
#include <stdexcept>

using std::string;
using std::map;

class Settings : public sigc::trackable {
 private:
  std::map<const string,string> m_map;
  std::string m_filename;
  
 public:
  Settings();
  virtual ~Settings();
  
  void load(const string& filename) throw(std::runtime_error);
  void save(const string& filename) throw(std::runtime_error);
  void save() throw (std::runtime_error);
  
  string         getValueString       (const string& k);
  int            getValueInt          (const string& k);
  unsigned int   getValueUnsignedInt  (const string& k);
  unsigned short getValueUnsignedShort(const string& k);
  unsigned char  getValueUnsignedChar (const string& k);
  bool           getValueBool         (const string& k);

  void setValue(const string& k, const string& v);
  void setValue(const string& k, int v);
  void setValue(const string& k, unsigned int v);
  void setValue(const string& k, unsigned short v);
  void setValue(const string& k, unsigned char v);
  void setValue(const string& k, bool v);

  void defaultValueUnsignedInt  (const string& k, unsigned int dflt, unsigned int lower = 0, unsigned int upper = 0xffffffff);
  void defaultValueUnsignedShort(const string& k, unsigned short dflt, unsigned short lower = 0, unsigned short upper = 0xffff);
  void defaultValueUnsignedChar (const string& k, unsigned char dflt, unsigned char lower = 0, unsigned char upper = 0xff);
  void defaultValueBool         (const string& k, bool dflt);
  void defaultValueString       (const string& k, const string& dflt);

  bool exists(const string& k);

  virtual void defaultSettings();

  static string Escape(const string& t);
  static string Unescape(const string& t);

  sigc::signal1<void,const string &> settings_changed;
};

#endif

