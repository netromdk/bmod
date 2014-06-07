#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>

#include "Util.h"

QString Util::cpuTypeString(CpuType type) {
  switch (type) {
  default:
  case CpuType::X86:
    return "x86";
    
  case CpuType::X86_64:
    return "x86 64";
    
  case CpuType::HPPA:
    return "HPPA";
    
  case CpuType::ARM:
    return "ARM";
    
  case CpuType::SPARC:
    return "SPARC";
    
  case CpuType::I860:
    return "i860";
    
  case CpuType::PowerPC:
    return "PowerPC";
    
  case CpuType::PowerPC_64:
    return "PowerPC 64";

  case CpuType::I386:
    return "i386";

  case CpuType::I486:
    return "i486";

  case CpuType::I486_SX:
    return "i486 SX";

  case CpuType::Pentium:
    return "Pentium";

  case CpuType::PentiumPro:
    return "Pentium Pro";

  case CpuType::PentiumII_M3:
    return "Pentium II M3";

  case CpuType::PentiumII_M5:
    return "Pentium II M5";

  case CpuType::Celeron:
    return "Celeron";

  case CpuType::CeleronMobile:
    return "Celeron Mobile";

  case CpuType::Pentium_3:
    return "Pentium 3";

  case CpuType::Pentium_3_M:
    return "Pentium 3 M";

  case CpuType::Pentium_3_Xeon:
    return "Pentium 3 Xeon";

  case CpuType::Pentium_M:
    return "Pentium M";

  case CpuType::Pentium_4:
    return "Pentium 4";

  case CpuType::Pentium_4_M:
    return "Pentium 4 M";

  case CpuType::Itanium:
    return "Itanium";

  case CpuType::Itanium_2:
    return "Itanium 2";

  case CpuType::Xeon:
    return "Xeon";

  case CpuType::Xeon_MP:
    return "Xeon MP";
  }
}

QString Util::fileTypeString(FileType type) {
  switch (type) {
  case FileType::Object:
    return "Object";

  default:
  case FileType::Execute:
    return "Executable";

  case FileType::Core:
    return "Core";

  case FileType::Preload:
    return "Preloaded Program";

  case FileType::Dylib:
    return "Dylib";

  case FileType::Dylinker:
    return "Dylinker";

  case FileType::Bundle:
    return "Bundle";
  }
}

QString Util::sectionTypeString(SectionType type) {
  switch (type) {
  default:
  case SectionType::Text:
    return "Text";

  case SectionType::CString:
    return "CString";
  }
}

void Util::centerWidget(QWidget *widget) {
  widget->move(QApplication::desktop()->screen()->rect().center()
               - widget->rect().center());
}

QString Util::formatSize(qint64 bytes, int digits) {
  constexpr double KB = 1024, MB = 1024 * KB, GB = 1024 * MB, TB = 1024 * GB;
  QString unit{"B"};
  double size = bytes;
  if (size >= TB) {
    size /= TB;
    unit = "TB";
  }
  else if (size >= GB) {
    size /= GB;
    unit = "GB";
  }
  else if (size >= MB) {
    size /= MB;
    unit = "MB";
  }
  else if (size >= KB) {
    size /= KB;
    unit = "KB";
  }
  return QString("%1 %2").arg(QString::number(size, 'f', digits)).arg(unit);
}

QString Util::addrDataString(quint64 addr, QByteArray data) {
  // Pad data to a multiple of 16.
  quint64 rest = data.size() % 16;
  if (rest != 0) {
    int amount = 16 - rest;
    for (int i = 0; i < amount; i++) {
      data += (char) 0;
    }
  }

  QString output = QString::number(addr, 16).toUpper() + ": ";
  QString ascii;
  for (int i = 0; i < data.size(); i++) {
    char c = data[i];
    int ic = c;
    unsigned char uc = c;
    QString hc = QString::number(uc, 16).toUpper();
    if (hc.size() == 1) {
      hc = "0" + hc;
    }
    output += hc + " ";
    ascii += (ic >= 33 && ic <= 126 ? c : '.');
    if ((i + 1) % 16 == 0 || i == data.size() - 1) {
      output += "  " + ascii;
      ascii.clear();
      if (i != data.size() - 1) {
        addr += 16;
        output += "\n" + QString::number(addr, 16).toUpper() + ": ";
      }
    }
  }
  return output;
}
