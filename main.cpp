#include <iostream>
#include <cstring>
#include "UDTP.h"

using namespace std;

int main()
{
    UDTPSetup MySetup(6666);
    MySetup.set_debug_enabled(1);

    UDTP MyServer(MySetup);
    std::cout << MyServer.start(HOST) << std::endl;
    UDTPSetup MyOtherSetup("127.0.0.1", 6666);
    UDTP MyClient(MyOtherSetup);
    std::cout << MyClient.start(PEER) << std::endl;
    while(MyClient.alive());
    return 0;
}
