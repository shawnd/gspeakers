/*
  gspeakers_object Copyright (C) 2002 Daniel Sundberg

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

#ifndef __GSPEAKERSOBJECT_H
#define __GSPEAKERSOBJECT_H

#include <sigc++/object.h>

/*
 * This is a base object for data-containers in this project
 *
 * Since every part and every net will have an unique id this 
 * is probably a pretty nice base-class
 */
class GSpeakersObject : public sigc::trackable
{
public:
  GSpeakersObject();
  
  /*
   * The member variable m_id is a unique id for each instance of GSpeakersObject.
   * We use this as a base class for all classes that needs a id, for example all 
   * parts definately need a unique id.
   */
  int get_id();
  
  /* 
   * Since both Net, Part and crossover will use type constants it's probably pretty safe to put
   * logic for it in the base object so that it'll be easier to change if we want to do it 
   * different in the future.
   */
  int get_type();
  void set_type(int type);
  
protected:
  int m_id;
  int m_type;
  
};

#endif
