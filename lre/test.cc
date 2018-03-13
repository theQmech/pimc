#include <iostream>
#include "driver.hh"
#include "symbols.h"
#include "symtab.h"

using namespace std;

symtab_ symtab;

int
main (int argc, char *argv[])
{
  lre_driver driver;

  if (argc <= 1){
    cout<<"arg1"<<endl;
    return 2;
  }

  cout<<driver.parse(argv[1])<<endl;
  cout<<driver.result<<endl;

  return 0;

  // int res = 0;
  // for (int i = 1; i < argc; ++i)
  //   if (argv[i] == std::string ("-p"))
  //     driver.trace_parsing = true;
  //   else if (argv[i] == std::string ("-s"))
  //     driver.trace_scanning = true;
  //   else if (!driver.parse (argv[i]))
  //     std::cout << driver.result << std::endl;
  //   else
  //     res = 1;
  // return res;
}

