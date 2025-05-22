#include <mmedia/webrtc/DtlsCerts.h>
#include <iostream>
using namespace tmms::mm;
int main (int argc, char *argv[]) {
  
  DtlsCerts certs;
  certs.Init();
  std::cout << "fingerprint:" << certs.Fingerprint() << std::endl;
  return 0;
}
