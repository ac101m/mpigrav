#include <iostream>

#include <optparse.hpp>

#include "mpi.h"


void AddOptions(OptionParser& opt) {
  opt.Add(Option("gravitation", 'g', ARG_TYPE_FLOAT,
                 "Gravitational constant for the simulation",
                 {"6.67408E-11"}));
}


int main(int argc, char **argv) {
  OptionParser opt(argc, argv, "mpigrav compute engine");
  AddOptions(opt);

  return 0;
}
