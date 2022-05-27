#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osa/Tokens.hh"
#include "rms/Errors.hh"
#include "rms/Client.hh"

#include "Write.hh"

using namespace RMS;

static const char COMMAND[] = "write";

/*
** ++
**
**
** --
*/

Access::Write::Write(Client& client) :
  Command(COMMAND, client)
{ 
  
}

/*
** ++
**
**
** --
*/

#define REGISTERS argv[0]
#define VALUE     argv[1]
#define TARGETS   argv[2]

static const char NO_REGISTER[] = "No Valid Register Address supplied\n";
static const char NO_VALUE[]    = "No Valid Register Value supplied\n";
static const char NO_TARGETS[]  = "No valid targets given\n";

void Access::Write::process(int argc, const char** argv, int constant)   
{
  unsigned base;
  unsigned number;

  if(0 == argc)  { help(argc, argv, constant); return; }

  if(0 == strncmp(argv[0], "-h", 2))
  {
    help(argc, argv, constant);
    return;
  }

  if(!_parse_regs(REGISTERS, base, number)) {printf(NO_REGISTER); return;}
  
  char* last;
  unsigned value = strtoll(VALUE, &last, 0);
  if(last == VALUE) {printf(NO_VALUE); return;}

  InstructionList instructions(number);
  
  _build_list(base, Instruction::PUT, value, instructions);

  DAQ::LocationSet targets = _parse_targets(argc-2, (const char**)&TARGETS);

  if(!targets) { printf(NO_TARGETS); return; }

  _client.access(targets, instructions, _harvest);

  _harvest.summary();
  _harvest.clear();

  return; 
}

/*
** ++
**
**
** --
*/

static const char SUMMARY[] = 
  " Write a value to one or more registers in one or more Sources.\n\n"
  "  Usage: <command> <registers> <value> [DAQ Loacations]\n\n";

void Access::Write::help(int argc, const char** argv, int qualifier)   
{
  printf(SUMMARY);
  Access::Command::help(argc, argv, qualifier); 
 
  return;
}
