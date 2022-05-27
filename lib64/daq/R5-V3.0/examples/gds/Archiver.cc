
#include "Archiver.hh"

using namespace GDS;

const char DIRECTORY[] = "/tmp/jgt";

Example::Archiver::Archiver(const char* partition, const GDS::LocationSet& locs, bool raw) : 
  GDS::Decoder(partition, locs),
  _raw(raw)
{
  int remaining = Set::SIZE;
  while(remaining--)
  {
    _dfile[remaining] = NULL;
    _rfile[remaining] = NULL;
  }
};

Example::Archiver::~Archiver()
{
  printf("here\n");
  int remaining = Set::SIZE;
  while(remaining--)
  {
    if(_dfile[remaining]) fclose(_dfile[remaining]);
    if(_rfile[remaining]) fclose(_rfile[remaining]);
  }
}

const char MODE[] = "wb";

void Example::Archiver::start  (const GDS::StateMetadata& state, const GDS::SeriesMetadata& series)
{
  state.dump(); 

  int idx = state.sensor().index();
  
  char dfile[512];
  sprintf(dfile, "%s/%05i_%02i_%1i_%1i.stamp",  DIRECTORY, state.sequence(), state.sensor().bay(), state.sensor().board(), state.sensor().sensor());
  _dfile[idx] = fopen(dfile, MODE); 
  //fwrite(&state, sizeof(StateMetadata), 1, _dfile[idx]);
  fwrite(&series, sizeof(SeriesMetadata), 1, _dfile[idx]);
  
  if(!_raw) return;

  char rfile[512];
  sprintf(rfile, "%s/%05i_%02i_%1i_%1i.rstamp", DIRECTORY, state.sequence(), state.sensor().bay(), state.sensor().board(), state.sensor().sensor());
  _rfile[idx] = fopen(rfile, MODE); 
  //fwrite(&state, sizeof(StateMetadata), 1, _rfile[idx]);
  fwrite(&series, sizeof(SeriesMetadata), 1, _rfile[idx]);
}

void Example::Archiver::stop   (const GDS::StateMetadata& state)
{
  int idx = state.sensor().index();
  if(_dfile[idx]) {fclose(_dfile[idx]); _dfile[idx]=NULL;}
  if(_rfile[idx]) {fclose(_rfile[idx]); _rfile[idx]=NULL;}

  state.dump(); 
}

void Example::Archiver::raw_stamp(const GDS::StateMetadata& state, const GDS::RawStamp& stamp)
{
  if(!_raw) return;

  FILE* file = _rfile[state.sensor().index()];
  //fwrite(&state, sizeof(StateMetadata), 1, file);
  fwrite(stamp.content(), stamp.size(), 1, file);
}

void Example::Archiver::stamp(const GDS::StateMetadata& state, const GDS::Stamp& stamp)
{
  FILE* file = _dfile[state.sensor().index()];
  //fwrite(&state, sizeof(StateMetadata), 1, file);
  fwrite(stamp.content(), stamp.size(), 1, file);
}

uint8_t* Example::Archiver::allocate(unsigned size)
{
  return _stamp_buf;
}

