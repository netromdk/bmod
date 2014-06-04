#ifndef BMOD_FORMAT_H
#define BMOD_FORMAT_H

class QString;

class Format {
public:
  enum Kind {
    MachO
  };

  Format(Kind kind) : kind{kind} { }

  Kind getKind() const { return kind; }
  
  /**
   * Detect whether the magic code of the file corresponds to the
   * format.
   */
  virtual bool detect() =0;

private:
  Kind kind;
};

#endif // BMOD_FORMAT_H
