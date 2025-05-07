#include "PatWriter.h"
#include "mmedia/base/BytesWriter.h"
using namespace tmms::mm;

PatWriter::PatWriter()  {
  pid_ = 0x0000;  
  table_id_ = 0x00;
}

void PatWriter::WritePat(StreamWriter *w)  {
 
  //section data
  uint8_t section[kSectionMaxSize], *q;
  q = section;
  BytesWriter::WriteUint16T((char*)q, program_number_);//table_id
  q += 2;
  //program_number != 0时填充program_map_pid
  BytesWriter::WriteUint16T((char*)q, 0xe000 | pmt_pid_);//1101 13
  q += 2;

  PSIWriter::WriteSection(w, transport_stream_id_, 0, 0, section, q - section);
}
 

