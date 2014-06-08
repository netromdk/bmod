#include "Section.h"

Section::Section(SectionType type, const QString &name, quint64 addr,
                 quint64 size, quint32 offset)
  : type{type}, name{name}, addr{addr}, size{size}, offset{offset}
{ }

void Section::setSubData(const QByteArray &subData, int pos) {
  if (pos < 0 || pos > data.size() - 1) {
    return;
  }
  data.replace(pos, subData.size(), subData);

  // TODO: remember this range that was changed.
}
