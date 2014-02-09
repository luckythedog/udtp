#include "UDTPChunk.h"
/*Implementations after forward declaration*/
#include "UDTPWhine.h"
#include "UDTPData.h""

UDTPChunk::UDTPChunk(UDTPData&_data){ /*Unpacking of a UDTPChunk from UDTPData*/
}

UDTPWhine& UDTPChunk::create_whine_from_chunk(){
    UDTPWhine newWhine(get_file_identifier(), get_chunk_identifier());
    return newWhine;
}
