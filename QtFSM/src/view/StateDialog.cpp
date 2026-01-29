#include "StateDialog.h"
#include "../model/State.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

StateDialog::StateDialog(State *state, QWidget *parent)
    : QDialog(parent)
    , m_state(state)
{
    setWindowTitle(tr("Edit State"));
    setMinimumSize(450, 400);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Form layout for properties
    QFormLayout *formLayout = new QFormLayout();
    
    // State name
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(m_state->name());
    m_nameEdit->setPlaceholderText("Enter state name...");
    formLayout->addRow(tr("State Name:"), m_nameEdit);
    
    // State type checkboxes
    QGroupBox *typeGroup = new QGroupBox(tr("State Type"), this);
    QVBoxLayout *typeLayout = new QVBoxLayout(typeGroup);
    
    m_initialCheckBox = new QCheckBox(tr("Initial State"), this);
    m_initialCheckBox->setChecked(m_state->isInitial());
    typeLayout->addWidget(m_initialCheckBox);
    
    m_finalCheckBox = new QCheckBox(tr("Final State"), this);
    m_finalCheckBox->setChecked(m_state->isFinal());
    typeLayout->addWidget(m_finalCheckBox);
    
    formLayout->addRow(typeGroup);
    
    // Entry action
    QLabel *entryLabel = new QLabel(tr("Entry Action (C++ code):"), this);
    m_entryActionEdit = new QTextEdit(this);
    m_entryActionEdit->setPlainText(m_state->entryAction());
    m_entryActionEdit->setPlaceholderText("// Code executed when entering this state\nstd::cout << \"Entering state\" << std::endl;");
    m_entryActionEdit->setMaximumHeight(80);
    formLayout->addRow(entryLabel, m_entryActionEdit);
    
    // Exit action
    QLabel *exitLabel = new QLabel(tr("Exit Action (C++ code):"), this);
    m_exitActionEdit = new QTextEdit(this);
    m_exitActionEdit->setPlainText(m_state->exitAction());
    m_exitActionEdit->setPlaceholderText("// Code executed when exiting this state");
    m_exitActionEdit->setMaximumHeight(80);
    formLayout->addRow(exitLabel, m_exitActionEdit);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    QPushButton *okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Focus on name edit
    m_nameEdit->selectAll();
    m_nameEdit->setFocus();
}

StateDialog::~StateDialog()
{
}

void StateDialog::accept()
{
    // Update state properties
    m_state->setName(m_nameEdit->text());
    m_state->setEntryAction(m_entryActionEdit->toPlainText());
    m_state->setExitAction(m_exitActionEdit->toPlainText());
    m_state->setInitial(m_initialCheckBox->isChecked());
    m_state->setFinal(m_finalCheckBox->isChecked());
    
    QDialog::accept();
}
