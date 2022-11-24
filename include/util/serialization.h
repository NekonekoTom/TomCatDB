#ifndef SERIALIZATION_H_
#define SERIALIZATION_H_

#include "base.h"

class Serializable
{
public:
  static const U_LONG serialID = 0;
  virtual char* Serialize() = 0;
  virtual void Deserialize(char*) = 0;
};

# endif