#include "DiagramViewModel.h"
#include "../model/FSM.h"

DiagramViewModel::DiagramViewModel(QObject *parent)
    : QObject(parent)
    , m_fsm(nullptr)
{
}

DiagramViewModel::~DiagramViewModel()
{
}

FSM* DiagramViewModel::fsm() const
{
    return m_fsm;
}

void DiagramViewModel::setFSM(FSM *fsm)
{
    m_fsm = fsm;
}
