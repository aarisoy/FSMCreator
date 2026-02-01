#include "PropertiesPanel.h"
#include "../model/FSM.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include <QDebug>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

PropertiesPanel::PropertiesPanel(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(10);
  layout->setContentsMargins(10, 10, 10, 10);

  // Title
  QLabel *titleLabel = new QLabel("Properties", this);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(12);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  layout->addWidget(titleLabel);

  // Separator
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  layout->addWidget(line);

  // FSM Properties Section Container
  m_fsmSettingsContainer = new QWidget(this);
  QVBoxLayout *fsmVLayout = new QVBoxLayout(m_fsmSettingsContainer);
  fsmVLayout->setContentsMargins(0, 0, 0, 0);

  QLabel *fsmSectionLabel = new QLabel("FSM Settings", m_fsmSettingsContainer);
  QFont sectionFont = fsmSectionLabel->font();
  sectionFont.setBold(true);
  fsmSectionLabel->setFont(sectionFont);
  fsmVLayout->addWidget(fsmSectionLabel);

  m_formLayout = new QFormLayout();

  // Name row with button
  QHBoxLayout *nameLayout = new QHBoxLayout();
  m_nameEdit = new QLineEdit(m_fsmSettingsContainer);
  m_nameEdit->setPlaceholderText("MyFSM");
  nameLayout->addWidget(m_nameEdit);

  QPushButton *applyBtn = new QPushButton("Apply", m_fsmSettingsContainer);
  applyBtn->setToolTip("Apply FSM Name Change");
  nameLayout->addWidget(applyBtn);

  m_formLayout->addRow("Name:", nameLayout);
  fsmVLayout->addLayout(m_formLayout);

  // States list
  QLabel *statesLabel = new QLabel("States:", m_fsmSettingsContainer);
  QFont stateFont = statesLabel->font();
  stateFont.setBold(true);
  statesLabel->setFont(stateFont);
  fsmVLayout->addWidget(statesLabel);

  m_statesTree = new QTreeWidget(m_fsmSettingsContainer);
  m_statesTree->setHeaderHidden(true);
  m_statesTree->setMinimumHeight(60);
  m_statesTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  m_statesTree->setStyleSheet(
      "QTreeWidget { background-color: #f9f9f9; border: 1px solid #ddd; }");
  fsmVLayout->addWidget(m_statesTree);

  // Transitions list
  QLabel *transitionsLabel = new QLabel("Transitions:", m_fsmSettingsContainer);
  QFont transFont = transitionsLabel->font();
  transFont.setBold(true);
  transitionsLabel->setFont(transFont);
  fsmVLayout->addWidget(transitionsLabel);

  m_transitionsTree = new QTreeWidget(m_fsmSettingsContainer);
  m_transitionsTree->setHeaderHidden(true);
  m_transitionsTree->setMinimumHeight(60);
  m_transitionsTree->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Minimum);
  m_transitionsTree->setStyleSheet(
      "QTreeWidget { background-color: #f9f9f9; border: 1px solid #ddd; }");
  fsmVLayout->addWidget(m_transitionsTree);

  layout->addWidget(m_fsmSettingsContainer);
  m_fsmSettingsContainer->hide(); // Hide initially

  // Connect name edit (on Enter or Focus Loss)
  connect(m_nameEdit, &QLineEdit::editingFinished, this, [this]() {
    if (m_fsm) {
      m_fsm->setName(m_nameEdit->text());
    }
  });

  // Connect Apply button
  connect(applyBtn, &QPushButton::clicked, this, [this]() {
    if (m_fsm) {
      m_fsm->setName(m_nameEdit->text());
      m_fsm->forceUpdate(); // Force code regeneration
    }
  });

  // Separator 2
  QFrame *line2 = new QFrame(this);
  line2->setFrameShape(QFrame::HLine);
  line2->setFrameShadow(QFrame::Sunken);
  layout->addWidget(line2);

  // Info label (for context selection)
  QLabel *infoLabel =
      new QLabel("Select a state or transition\nto edit its properties", this);
  infoLabel->setWordWrap(true);
  infoLabel->setAlignment(Qt::AlignCenter);
  infoLabel->setStyleSheet("color: #666; padding: 20px;");
  QFont infoFont = infoLabel->font();
  infoFont.setPointSize(10);
  infoLabel->setFont(infoFont);
  layout->addWidget(infoLabel);

  layout->addStretch();

  // Style the panel
  setStyleSheet(R"(
        QWidget {
            background-color: #ffffff;
            border-left: 1px solid #ddd;
        }
    )");

  setMinimumWidth(280);
}

PropertiesPanel::~PropertiesPanel() {}

void PropertiesPanel::setFSM(FSM *fsm) {
  if (m_fsm == fsm)
    return;

  if (m_fsm) {
    m_fsm->disconnect(this);
  }

  m_fsm = fsm;

  if (m_fsm) {
    m_fsmSettingsContainer->show();
    m_nameEdit->setText(m_fsm->name());
    connect(m_fsm, &FSM::nameChanged, m_nameEdit, &QLineEdit::setText);

    // Connect to FSM signals for updates
    connect(m_fsm, &FSM::stateAdded, this, &PropertiesPanel::updateStates);
    connect(m_fsm, &FSM::stateRemoved, this, &PropertiesPanel::updateStates);
    connect(m_fsm, &FSM::transitionAdded, this,
            &PropertiesPanel::updateTransitions);
    connect(m_fsm, &FSM::transitionRemoved, this,
            &PropertiesPanel::updateTransitions);

    // Initial update
    updateStates();
    updateTransitions();
  } else {
    m_fsmSettingsContainer->hide();
    m_nameEdit->clear();
    m_statesTree->clear();
    m_transitionsTree->clear();
  }
}

void PropertiesPanel::updateStates() {
  if (!m_fsm)
    return;

  m_statesTree->clear();

  for (State *state : m_fsm->states()) {
    QTreeWidgetItem *item = new QTreeWidgetItem(m_statesTree);
    item->setText(0, state->name());
    item->setIcon(0, QIcon());
  }
}

void PropertiesPanel::updateTransitions() {
  if (!m_fsm)
    return;

  m_transitionsTree->clear();

  for (Transition *trans : m_fsm->transitions()) {
    QTreeWidgetItem *item = new QTreeWidgetItem(m_transitionsTree);
    QString text =
        QString("%1 → %2")
            .arg(trans->sourceState() ? trans->sourceState()->name() : "?")
            .arg(trans->targetState() ? trans->targetState()->name() : "?");

    if (!trans->event().isEmpty()) {
      text += QString(" [%1]").arg(trans->event());
    }

    item->setText(0, text);
    item->setIcon(0, QIcon());
  }
}
