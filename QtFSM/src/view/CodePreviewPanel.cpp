#include "CodePreviewPanel.h"
#include "../codegen/CodeGenerator.h"
#include "../model/FSM.h"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

CodePreviewPanel::CodePreviewPanel(QWidget *parent)
    : QWidget(parent), m_isInternalUpdate(false) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(5);
  layout->setContentsMargins(5, 5, 5, 5);

  // Title bar with controls
  QHBoxLayout *titleLayout = new QHBoxLayout();
  QLabel *titleLabel = new QLabel("Generated C++ Code", this);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(11);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);

  // Generate Code Button ( Manual Trigger )
  QPushButton *generateBtn = new QPushButton("Generate Code ->", this);
  generateBtn->setToolTip("Generate C++ code from current FSM model");
  connect(generateBtn, &QPushButton::clicked, this,
          &CodePreviewPanel::generateCodeRequested);

  // Style the generate button
  generateBtn->setStyleSheet(
      "QPushButton { background-color: #28a745; color: white; border: none; "
      "padding: 5px 10px; border-radius: 3px; font-weight: bold; }"
      "QPushButton:hover { background-color: #218838; }"
      "QPushButton:pressed { background-color: #1e7e34; }");

  // Update Diagram Button (Reverse)
  QPushButton *updateDiagramBtn = new QPushButton("Update Diagram <-", this);
  updateDiagramBtn->setToolTip("Parse this code and update the diagram");
  connect(updateDiagramBtn, &QPushButton::clicked, this,
          &CodePreviewPanel::updateDiagramRequested);

  // Style the update diagram button
  updateDiagramBtn->setStyleSheet(
      "QPushButton { background-color: #007acc; color: white; border: none; "
      "padding: 5px 10px; border-radius: 3px; }"
      "QPushButton:hover { background-color: #0098ff; }"
      "QPushButton:pressed { background-color: #005a9e; }");

  titleLayout->addWidget(titleLabel);
  titleLayout->addStretch();
  titleLayout->addWidget(generateBtn);
  titleLayout->addWidget(updateDiagramBtn);

  layout->addLayout(titleLayout);

  // Code editor (editable)
  m_codeEdit = new QTextEdit(this);
  m_codeEdit->setReadOnly(false); // User can edit now
  m_codeEdit->setLineWrapMode(QTextEdit::NoWrap);

  // Connect text change signal
  connect(m_codeEdit, &QTextEdit::textChanged, [this]() {
    if (!m_isInternalUpdate) {
      emit codeChanged(m_codeEdit->toPlainText());
    }
  });

  // Set monospace font
  QFont codeFont("Courier New");
  codeFont.setPointSize(14); // Increased from 9 to 14
  m_codeEdit->setFont(codeFont);

  m_codeEdit->setPlaceholderText("");

  layout->addWidget(m_codeEdit);

  // Style
  setStyleSheet(R"(
        QWidget {
            background-color: #ffffff;
        }
        QTextEdit {
            background-color: #f8f8f8;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 8px;
        }
    )");
}

CodePreviewPanel::~CodePreviewPanel() {}

void CodePreviewPanel::updateCode(FSM *fsm) {
  if (!fsm || fsm->states().isEmpty()) {
    clearCode();
    return;
  }

  // Generate code
  CodeGenerator generator;
  QString code = generator.generate(fsm);

  // Display code - use flag to prevent circular signal
  m_isInternalUpdate = true;

  // Only update if content is different to preserve cursor if possible
  if (m_codeEdit->toPlainText() != code) {
    m_codeEdit->setPlainText(code);
  }

  m_isInternalUpdate = false;
}

void CodePreviewPanel::clearCode() {
  m_isInternalUpdate = true;
  m_codeEdit->clear();
  m_isInternalUpdate = false;
}

QString CodePreviewPanel::code() const { return m_codeEdit->toPlainText(); }

void CodePreviewPanel::setReadOnly(bool readOnly) {
  m_codeEdit->setReadOnly(readOnly);
}

void CodePreviewPanel::setCode(const QString &code) {
  m_isInternalUpdate = true;
  m_codeEdit->setPlainText(code);
  m_isInternalUpdate = false;
}
