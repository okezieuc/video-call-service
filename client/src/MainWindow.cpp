#include "MainWindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    auto* startCallButton = new QPushButton("Start Call", central);

    layout->addWidget(startCallButton, 0, Qt::AlignCenter);
    setCentralWidget(central);
    setWindowTitle("Video Call Client");
    resize(480, 320);

    connect(startCallButton, &QPushButton::clicked, this, []() {});
}
