#ifndef TRANSITIONDIALOG_H
#define TRANSITIONDIALOG_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QCheckBox;
class FSM;
class State;

class TransitionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransitionDialog(FSM *fsm, QWidget *parent = nullptr);
    
    State* sourceState() const;
    State* targetState() const;
    QString eventName() const;
    bool isDefault() const;

private:
    FSM *m_fsm;
    QComboBox *m_sourceCombo;
    QComboBox *m_targetCombo;
    QLineEdit *m_eventEdit;
    QCheckBox *m_defaultCheck;
};

#endif // TRANSITIONDIALOG_H
