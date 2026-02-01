#include "AboutDialog.h"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>


AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("About QtFSM Designer"));
  setModal(true);
  setMinimumWidth(450);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(15);

  // Application Name
  QLabel *titleLabel = new QLabel(tr("<h2>QtFSM Designer</h2>"), this);
  titleLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(titleLabel);

  // Version
  QLabel *versionLabel = new QLabel(tr("<b>Version:</b> 1.0.0"), this);
  versionLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(versionLabel);

  // Description
  QLabel *descLabel = new QLabel(
      tr("<p>A powerful visual Finite State Machine designer and "
         "code generator.</p>"
         "<p>Create, edit, and export FSMs with an intuitive drag-and-drop "
         "interface.</p>"),
      this);
  descLabel->setWordWrap(true);
  descLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(descLabel);

  // Separator line
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  mainLayout->addWidget(line);

  // Features
  QLabel *featuresLabel =
      new QLabel(tr("<b>Features:</b><br>"
                    "• Visual FSM design with drag-and-drop<br>"
                    "• Undo/Redo support<br>"
                    "• C++ code generation<br>"
                    "• JSON import/export<br>"
                    "• Reverse engineering from C++ code"),
                 this);
  featuresLabel->setWordWrap(true);
  mainLayout->addWidget(featuresLabel);

  // Libraries used
  QLabel *libsLabel = new QLabel(tr("<b>Built with:</b><br>"
                                    "• Qt 6 Framework<br>"
                                    "• Google Test (testing)<br>"
                                    "• Modern C++17"),
                                 this);
  libsLabel->setWordWrap(true);
  mainLayout->addWidget(libsLabel);

  // Copyright
  QLabel *copyrightLabel =
      new QLabel(tr("<p style='color: #666;'>© 2026 QtFSM Project</p>"), this);
  copyrightLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(copyrightLabel);

  mainLayout->addStretch();

  // Close button
  QPushButton *closeBtn = new QPushButton(tr("Close"), this);
  closeBtn->setDefault(true);
  connect(closeBtn, &QPushButton::clicked, this, &AboutDialog::accept);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  btnLayout->addWidget(closeBtn);
  btnLayout->addStretch();
  mainLayout->addLayout(btnLayout);

  setLayout(mainLayout);
}

AboutDialog::~AboutDialog() {}
