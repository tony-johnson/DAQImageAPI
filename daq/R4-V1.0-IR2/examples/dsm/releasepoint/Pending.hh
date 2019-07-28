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

#ifndef DSM_RELEASEPOINTEDITOR_PENDING
#define DSM_RELEASEPOINTEDITOR_PENDING

#include "net/ipv4/Address.hh"
#include "dsi/Location.hh"
#include "dsm/ReleasepointProcessor.hh"

namespace DSM {namespace ReleasepointEditor {

class Pending : public DSM::ReleasepointProcessor { 
public:
  Pending();
public:
 ~Pending() {}
public:
  void process(const DSI::Location&, const IPV4::Address&, const DSM::Releasepoint&, int32_t modified);
public:  
  void summary() const;
private:
  int _total; 
};

}}

#endif

