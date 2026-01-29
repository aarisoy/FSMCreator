#include "TransitionDialog.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>

TransitionDialog::TransitionDialog(FSM *fsm, QWidget *parent)
    : QDialog(parent)
    , m_fsm(fsm)
{
    setWindowTitle(tr("Add Transition"));
    resize(400, 250);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Source State
    QHBoxLayout *sourceLayout = new QHBoxLayout();
    sourceLayout->addWidget(new QLabel(tr("Source State:")));
    m_sourceCombo = new QComboBox();
    sourceLayout->addWidget(m_sourceCombo);
    layout->addLayout(sourceLayout);
    
    // Target State
    QHBoxLayout *targetLayout = new QHBoxLayout();
    targetLayout->addWidget(new QLabel(tr("Target State:")));
    m_targetCombo = new QComboBox();
    targetLayout->addWidget(m_targetCombo);
    layout->addLayout(targetLayout);
    
    // Populate combos
    if (m_fsm) {
        for (State *state : m_fsm->states()) {
            m_sourceCombo->addItem(state->name(), QVariant::fromValue(static_cast<void*>(state)));
            m_targetCombo->addItem(state->name(), QVariant::fromValue(static_cast<void*>(state)));
        }
    }
    
    // Event Name
    QHBoxLayout *eventLayout = new QHBoxLayout();
    eventLayout->addWidget(new QLabel(tr("Event:")));
    m_eventEdit = new QLineEdit();
    m_eventEdit->setPlaceholderText(tr("e.g. ButtonPressed"));
    eventLayout->addWidget(m_eventEdit);
    layout->addLayout(eventLayout);
    
    // Default Transition Checkbox
    m_defaultCheck = new QCheckBox(tr("Default Transition (no event required)"));
    layout->addWidget(m_defaultCheck);
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    // Logic: Disable event edit if default is checked
    connect(m_defaultCheck, &QCheckBox::toggled, [this](bool checked) {
        m_eventEdit->setEnabled(!checked);
        if (checked) {
            m_eventEdit->clear();
        }
    });
}

State* TransitionDialog::sourceState() const
{
    return static_cast<State*>(m_sourceCombo->currentData().value<void*>());
}

State* TransitionDialog::targetState() const
{
    return static_cast<State*>(m_targetCombo->currentData().value<void*>());
}

QString TransitionDialog::eventName() const
{
    return m_eventEdit->text();
}

bool TransitionDialog::isDefault() const
{
    return m_defaultCheck->isChecked();
}
