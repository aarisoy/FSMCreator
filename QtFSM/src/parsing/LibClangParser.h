#pragma once

#include <QString>
#include <QVector>

namespace FSMParser {

class ClassDecl;

class LibClangParser {
public:
  LibClangParser();

  QVector<ClassDecl *> parse(const QString &code);
  QString lastError() const;

private:
  QString m_lastError;
};

} // namespace FSMParser
