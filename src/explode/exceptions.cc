#include "explode/exceptions.hh"

namespace explode
{
  input_error::input_error ()
    : std::runtime_error ("Input error")
  {
    
  }
  input_error::~input_error () throw ()
  {
  }
  // --------------------------------------------------------------
  exefile_error::exefile_error ()
    : std::runtime_error ("Bad EXE file")
  {
    
  }
  exefile_error::~exefile_error () throw ()
  {
  }
  // --------------------------------------------------------------
  decoder_error::decoder_error (const char* msg)
    : std::runtime_error (msg)
  {
  }
  decoder_error::~decoder_error () throw ()
  {
  }
} // ns explode
