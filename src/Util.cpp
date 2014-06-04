#include "Util.h"

QString Util::cpuTypeString(CpuType type) {
  switch (type) {
  default:
  case CpuType::X86:
    return "x86";
    
  case CpuType::X86_64:
    return "x86_64";
    
  case CpuType::HPPA:
    return "HPPA";
    
  case CpuType::ARM:
    return "ARM";
    
  case CpuType::SPARC:
    return "SPARC";
    
  case CpuType::I860:
    return "i860";
    
  case CpuType::PowerPc:
    return "PowerPC";
    
  case CpuType::PowerPc_64:
    return "PowerPC_64";
  }
}
