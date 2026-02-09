#include "LibClangParser.h"

namespace FSMParser {

LibClangParser::LibClangParser() {}

QVector<ClassDecl *> LibClangParser::parse(const QString &code) {
  Q_UNUSED(code);
  m_lastError = "libclang parser not enabled. Rebuild with FSM_ENABLE_LIBCLANG=ON.";
  return {};
}

QString LibClangParser::lastError() const { return m_lastError; }

} // namespace FSMParser
