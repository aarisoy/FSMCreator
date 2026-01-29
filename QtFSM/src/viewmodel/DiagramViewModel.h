#ifndef DIAGRAMVIEWMODEL_H
#define DIAGRAMVIEWMODEL_H

#include <QObject>

class FSM;

/**
 * @brief The DiagramViewModel class - ViewModel for diagram editor
 */
class DiagramViewModel : public QObject
{
    Q_OBJECT

public:
    explicit DiagramViewModel(QObject *parent = nullptr);
    ~DiagramViewModel();

    FSM* fsm() const;
    void setFSM(FSM *fsm);

private:
    FSM *m_fsm;
};

#endif // DIAGRAMVIEWMODEL_H
