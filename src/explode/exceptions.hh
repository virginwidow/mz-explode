#ifndef __EXPLODE_EXCEPTIONS_HH__
#define __EXPLODE_EXCEPTIONS_HH__

#include <stdexcept>

#include "explode/proper_export.hh"

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4275)
#endif

namespace explode
{
  class EXPLODE_API input_error : public std::runtime_error
  {
  public:
    input_error ();
    ~input_error () throw ();
  };

  class EXPLODE_API exefile_error : public std::runtime_error
  {
  public:
    exefile_error ();
    ~exefile_error () throw ();
  };

  class EXPLODE_API decoder_error : public std::runtime_error
  {
  public:
    explicit decoder_error (const char* msg);
    ~decoder_error () throw ();
  };
} // ns explode

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif
