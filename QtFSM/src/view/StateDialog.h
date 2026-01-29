#ifndef STATEDIALOG_H
#define STATEDIALOG_H

#include <QDialog>

class State;
class QLineEdit;
class QTextEdit;
class QCheckBox;

/**
 * @brief The StateDialog class - Dialog for editing state properties
 */
class StateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StateDialog(State *state, QWidget *parent = nullptr);
    ~StateDialog();

private slots:
    void accept() override;

private:
    State *m_state;
    QLineEdit *m_nameEdit;
    QTextEdit *m_entryActionEdit;
    QTextEdit *m_exitActionEdit;
    QCheckBox *m_initialCheckBox;
    QCheckBox *m_finalCheckBox;
};

#endif // STATEDIALOG_H
