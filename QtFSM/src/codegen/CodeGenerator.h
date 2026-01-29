#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QObject>
#include <QString>

class FSM;

/**
 * @brief The CodeGenerator class - Generates C++ code from FSM
 */
class CodeGenerator : public QObject
{
    Q_OBJECT

public:
    explicit CodeGenerator(QObject *parent = nullptr);
    ~CodeGenerator();

    QString generate(const FSM *fsm);

private:
    QString sanitizeName(const QString &name);
};

#endif // CODEGENERATOR_H
