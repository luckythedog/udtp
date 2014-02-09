#include "UDTPHandshake.h"
/*Implementations after class forward declarations*/
#include "UDTPData.h"

UDTPHandshake::UDTPHandshake(UDTPData& data)
{
}
unsigned short& UDTPHandshake::operator[](const unsigned int index)
{
    if(index>0 && index<=sizeof(tokensValue)/sizeof(unsigned short)) return tokensValue[index];
}
