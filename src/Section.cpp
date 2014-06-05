#include "Section.h"

Section::Section(SectionType type, quint64 addr, quint64 size, quint32 offset)
  : type{type}, addr{addr}, size{size}, offset{offset}
{ }
