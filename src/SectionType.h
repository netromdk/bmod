#ifndef BMOD_SECTION_TYPE_H
#define BMOD_SECTION_TYPE_H

enum class SectionType {
  Text, // Executable code (__text, .text).
  CString, // Constant C strings (__cstring).
  String // String table constants.
};

#endif // BMOD_SECTION_TYPE_H
