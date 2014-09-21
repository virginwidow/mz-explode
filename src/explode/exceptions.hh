#ifndef __EXPLODE_EXCEPTIONS_HH__
#define __EXPLODE_EXCEPTIONS_HH__

#include <stdexcept>

namespace explode
{
  class input_error : public std::runtime_error
  {
  public:
    input_error ();
  };

  class exefile_error : public std::runtime_error
  {
  public:
    exefile_error ();
  };

  class decoder_error : public std::runtime_error
  {
  public:
    decoder_error (const char* msg);
  };
} // ns explode

#endif
