
/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	    000 - April 06, 2011
**
**  Revision History:
**	    None.
**
** --
*/
 
#ifndef DSM_SOURCEPROCESSOR
#define DSM_SOURCEPROCESSOR

#include "net/ipv4/Address.hh"
#include "dsi/Location.hh"
#include "dsm/Source.hh"

namespace DSM {

class SourceProcessor {
public:
  SourceProcessor() {}  
public:
  virtual ~SourceProcessor() {}
public: 
  virtual void process(const DSI::Location&, const IPV4::Address&, const DSM::Source sources[], int32_t modified) = 0;  
 };

}

#endif

