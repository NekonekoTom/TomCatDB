#include "variable_int.h"

int main() {
  unsigned long ul = 0xF53400001245A380;
  
  char* seq = VariableInteger::EncodeULong(ul);

  unsigned long decoded = VariableInteger::DecodeULong(seq);

  U_LONG l;

  return 0;
}