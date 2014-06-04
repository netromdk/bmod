#ifndef BMOD_FORMAT_H
#define BMOD_FORMAT_H

#include <QString>

#include <memory>

class Format {
public:
  enum class Kind {
    MachO
  };

  Format(Kind kind) : kind{kind} { }

  /**
   * Try each of the known formats and see if any of them are
   * compatible with the file.
   */
  static std::unique_ptr<Format> detect(const QString &file);

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

private:
  Kind kind;
};

#endif // BMOD_FORMAT_H
