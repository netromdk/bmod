#include "Section.h"

Section::Section(SectionType type, const QString &name, quint64 addr,
                 quint64 size, quint32 offset)
  : type{type}, name{name}, addr{addr}, size{size}, offset{offset}
{ }
