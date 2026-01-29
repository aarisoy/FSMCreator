#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <QString>

class FSM;
class QObject;

/**
 * @brief The CodeParser class - Parses C++ code to recreate FSM model
 */
class CodeParser
{
public:
    CodeParser();
    
    /**
     * @brief Parse C++ code and return a new FSM
     * @param code The C++ source code
     * @param parent Parent object for the FSM
     * @return New FSM instance or nullptr if parsing failed
     */
    FSM* parse(const QString &code, QObject *parent = nullptr);
    
    /**
     * @brief Get last error message
     */
    QString lastError() const;

private:
    QString m_lastError;
};

#endif // CODEPARSER_H
