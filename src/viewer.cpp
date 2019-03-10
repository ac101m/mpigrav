#include <iostream>
#include <unistd.h>

#include <GLT/Window.hpp>
#include <optparse.hpp>


void AddOptions(OptionParser& opt) {
  opt.Add(Option("host", 'h', ARG_TYPE_STRING,
                 "IP address of host to connect to.",
                 {"127.0.0.1"}));
}


int main(int argc, char **argv) {
  OptionParser opt(argc, argv, "mpigrav viewer application");
  AddOptions(opt);
  return 0;
}
