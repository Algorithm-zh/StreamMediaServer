#include "PmtWriter.h"
#include "mmedia/base/BytesWriter.h"
using namespace tmms::mm;

PmtWriter::PmtWriter()  {
  table_id_ = 0x02;
  pid_ = 0x1001;
}

void PmtWriter::WritePmt(StreamWriter *w)  {
  uint8_t section[kSectionMaxSize], *q;
  q = section;
  //1110 0000 0000 0000 reserved(3bits 保留位置为1) PCR_PID(13bits)
  BytesWriter::WriteUint16T((char*)q, 0xe000 | pcr_pid_);
  q += 2;
  //1111 0000 0000 0000 reserved(4bits 保留位置为1) program_info_length(12bits)
  BytesWriter::WriteUint16T((char*)q, 0xf000 | 0);
  q += 2;
  //原始流
  for(auto const &p : programs_)
  {
    *q ++ = p->stream_type;
    //1110 0000 0000 0000 reserved(3bits 保留位置为1) elementary_pid(13bits)
    BytesWriter::WriteUint16T((char*)q, 0xe000 | p->elementary_pid);
    q += 2;
    //1111 0000 0000 0000 reserved(4bits 保留位置为1) ES_info_length(12bits)
    BytesWriter::WriteUint16T((char*)q, 0xf000 | 0);
    q += 2;
  }
  PSIWriter::WriteSection(w, 0x0001, 0, 0, section, q - section);

}
 
void PmtWriter::AddProgramInfo(ProgramInfoPtr &program)  {
  programs_.emplace_back(program);
}
 
void PmtWriter::SetPcrPid(int32_t pid)  {
  pcr_pid_ = pid;
}
 

