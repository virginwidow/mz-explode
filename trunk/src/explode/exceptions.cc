#include "explode/exceptions.hh"

namespace explode
{
  input_error::input_error ()
    : std::runtime_error ("Input error")
  {
    
  }
  // --------------------------------------------------------------
  exefile_error::exefile_error ()
    : std::runtime_error ("Bad EXE file")
  {
    
  }
  // --------------------------------------------------------------
  decoder_error::decoder_error (const char* msg)
    : std::runtime_error (msg)
  {
  }
} // ns explode
