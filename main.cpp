#include <iostream>
#include <cstdint>
#include <vector>
#include <string>

#include "server.hpp"

Server server;
int main(int argc, char **argv)
{
    server.init();	
	//actual main loop
	while(true)
	{
		server.loop();
    }
}
