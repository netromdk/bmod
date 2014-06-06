#ifndef BMOD_FORMAT_H
#define BMOD_FORMAT_H

#include <QList>
#include <QString>

#include <memory>

#include "../Section.h"
#include "../CpuType.h"
#include "../FileType.h"
#include "../BinaryObject.h"

class Format;
typedef std::unique_ptr<Format> FormatPtr;

class Format {
public:
  enum class Kind {
    MachO
  };

  Format(Kind kind) : kind{kind} { }

  Kind getKind() const { return kind; }

  virtual QString getName() const =0;

  /**
   * Detect whether the magic code of the file corresponds to the
   * format. Only reads the first chunk of the file and not all of it!
   */
  virtual bool detect() =0;

  /**
   * Parses the file into the various sections and so on.
   */
  virtual bool parse() =0;

  /**
   * Get the list of probed binary objects of the file.
   */
  virtual QList<BinaryObjectPtr> getObjects() const =0;

  /**
   * Try each of the known formats and see if any of them are
   * compatible with the file.
   */
  static FormatPtr detect(const QString &file);

private:
  Kind kind;
};

#endif // BMOD_FORMAT_H
