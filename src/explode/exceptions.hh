#ifndef __EXPLODE_EXCEPTIONS_HH__
#define __EXPLODE_EXCEPTIONS_HH__

#include <stdexcept>

namespace explode
{
  class input_error : public std::runtime_error
  {
  public:
    input_error ();
    ~input_error () throw ();
  };

  class exefile_error : public std::runtime_error
  {
  public:
    exefile_error ();
    ~exefile_error () throw ();
  };

  class decoder_error : public std::runtime_error
  {
  public:
    explicit decoder_error (const char* msg);
    ~decoder_error () throw ();
  };
} // ns explode

#endif
