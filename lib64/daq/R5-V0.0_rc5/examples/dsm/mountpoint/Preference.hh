/*
** ++
**  Package:
**	
**
**  Abstract:
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - January 09, 2007
**
**  Revision History:
**	None.
**
** --
*/

#ifndef DSM_EDITOR_MOUNTPOINT_PREFERENCE
#define DSM_EDITOR_MOUNTPOINT_PREFERENCE

#include "dsi/Location.hh"
#include "dsm/Mountpoint.hh"

namespace DSM {namespace Editor {namespace Mountpoint {

class Preference : public DSM::Mountpoint { 
public:
  Preference(const char* value);
  Preference();
public:
  Preference(const Preference&) = delete;
public:
 ~Preference() {}
public:
  void process(const DSI::Location&, const char* value, unsigned pending);
public:  
  void summary() const;
private:
  int _total; 
};

}}}

#endif

