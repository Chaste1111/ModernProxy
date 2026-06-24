#include "config.h"
#include <cstring>
#include <cstdlib>

void Config::parse(int argc, char* argv[]) {
    for(int i=1; i<argc; ++i)
    {
        if(std::strcmp(argv[i], "-p")==0 && i+1<argc)
        {
            port_=std::atoi(argv[++i]);
        }
    }
}
