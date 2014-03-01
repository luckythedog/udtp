#include <iostream>
#include <cstring>
#include "UDTP.h"

using namespace std;

int main()
{
    UDTPSetup MySetup(6666);
    UDTP MyServer(MySetup);
    MyServer.start(HOST);
    while(MyServer.alive());
    return 0;
}
