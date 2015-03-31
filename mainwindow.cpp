/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include "mainwindow.h"
#include "completions.h"
#include <QSyntaxHighlighter>
#include <QHash>
#include <QPrintEngine>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextCharFormat>

// ~~~~~~~~~~ CONSTANTS ~~~~~~~~~~~~~
int NUM_SYMBOLS = 30;
QString WORDS[30] = {QString("AND"),QString("OR"),QString("NOT"),
                     QString("EQUIV"),QString("IMP"),QString("FA"),
                     QString("TE"),QString("MAX"),QString("\":"),
                     QString(":\""),QString("FF"),QString("<="),
                     QString(">="),QString("SUBOF"),QString("SUPOF"),
                     QString("SUBEQUAL"),QString("SUPEQUAL"),QString("!="),
                     QString("SUM"),QString("ELEMOF"),QString("UNION"),
                     QString("INTR"),QString("MTSET"),QString("UNIV"),
                     QString("NOELEM"),QString("THEN"),QString("CROSS"),
                     QString("PROD"),QString("RHO"),QString("SIG")};
QString SYMBOLS[30] = {QString("\u039B"),QString("\u22C1"),QString("\u00AC"),
                       QString("\u2261"),QString("\u21D2"),QString("\u2200"),
                       QString("\u2203"),QString("\u2191"),QString("\u3008"),
                       QString("\u3009"),QString("\u21D0"),QString("\u2264"),
                       QString("\u2265"),QString("\u2282"),QString("\u2283"),
                       QString("\u2286"),QString("\u2287"),QString("\u2260"),
                       QString("\u2211"),QString("\u2208"),QString("\u222A"),
                       QString("\u2229"),QString("\u2205"),QString("U"),
                       QString("\u2209"),QString("\u2192"),QString("\u2A2F"),
                       QString("\u22C5"),QString("\u03C1"),QString("\u03C3")};
QRegularExpression m_pattern;
QTextCharFormat m_format;

int FONT_SIZE = 14;

/* To do:
 * More symbols
 * Autocompletion of :=] and ) and }
 * Settings box
 *  * Specifically, icon/text size
 * Suggestion box for theorems
 * Suggestion box for keywords
 * theorem attachment thing
 * Github
*/
const QIcon icon = QIcon("application.icns");


MainWindow::MainWindow()
{
    QWidget::setWindowIcon(icon);
    QFont font;
    font.setFamily("Cambria Math");
    font.setStyleHint(QFont::Serif);
    font.setFixedPitch(true);
    font.setPointSize(FONT_SIZE);

    theorem_open = false;

    textEdit = new QPlainTextEdit;
    highlighter = new Highlighter(textEdit->document());
    setCentralWidget(textEdit);
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    readSettings();

    connect(textEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

    setCurrentFile("");
    setUnifiedTitleAndToolBarOnMac(true);

    textEdit->setFont(font);
    QFontMetrics metrics(font);
    textEdit->setTabStopWidth(4 * metrics.width(' '));
    //Completer(textEdit->document());
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile("");
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}
bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec())
        files = dialog.selectedFiles();
    else
        return false;

    return saveFile(files.at(0));
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About ProofBot"),
            tr("<b><i>Version 0.0.1</i></b><br><br>ProofBot (C) 2015 Joshua C. Haug.<br>"
               "This application is designed to meet the standard proof-writing syntax requirements"
               "set forth by David Gries and Fred B. Schneider in <i>A Logical Approach to Discrete Math</i>,"
               "and has been designed in particular for Pepperdine University's Formal Methods and Discrete Strutures courses."));
}

void MainWindow::print() {
    QString str = textEdit->toPlainText();
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    dlg->activateWindow();
    if (dlg->exec() == QDialog::Accepted) {
        QTextDocument *doc = new QTextDocument();
        doc->setPlainText(str);
        doc->print(&printer);
        delete doc;
    }
    delete dlg;
}

void MainWindow::typeset() {
    QString duh;
    duh = textEdit->toPlainText().replace(QString(WORDS[0]),QString(SYMBOLS[0]));
    for (int i = 1; i< NUM_SYMBOLS; i++) {
        duh = duh.replace(QString(WORDS[i]),QString(SYMBOLS[i]));
        textEdit->clear();
        textEdit->appendPlainText(duh);
    }
}

void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}

void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));



    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show ProofBot's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    //fontSizeAct = new QAction(QIcon(":/images/paste.png"),tr("&Fontsize"), this);

    theoremsAct = new QAction(QIcon(":/images/theorems.png"),tr("&Theorems"), this);
    theoremsAct->setStatusTip(tr("Show list of theorems"));
    connect(theoremsAct, SIGNAL(triggered()), this, SLOT(theorems()));

    helpAct = new QAction(QIcon(":/images/help.png"),tr("&Help"), this);
    helpAct->setStatusTip(tr("Show ProofBot help"));


    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
    typesetAct = new QAction(QIcon(":/images/assemble.png"), tr("&Typeset"), this);
    typesetAct->setStatusTip(tr("Typeset the document"));
    connect(typesetAct, SIGNAL(triggered()), this, SLOT(typeset()));

    printAct = new QAction(QIcon(":/images/print.png"), tr("&Print"),this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print this document"));
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(newAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->setMovable(false);
    editToolBar = addToolBar(tr("Edit"));
    // ~~~~~~~~~~~Potential features~~~~~~~~~~~
    //editToolBar->addAction(fontSizeIncAct);
    //editToolBar->addAction(fontSizeDecAct);
    //editToolBar->addAction(theoremAct);

    editToolBar->addAction(printAct);
    editToolBar->addAction(helpAct);
    editToolBar->addAction(theoremsAct);
    editToolBar->setMovable(false);

    typesetToolBar = addToolBar(tr("Typeset"));
    //typesetToolBar->isRightToLeft();
    //typesetToolBar->set
    typesetToolBar->addAction(typesetAct);
    typesetToolBar->setMovable(false);

//    QHBoxLayout *toolbarLayout = new QHBoxLayout;
//    toolbarLayout->addWidget(fileToolBar);
//    toolbarLayout->addWidget(editToolBar);
//    toolbarLayout->addWidget(typesetToolBar);


//    MainWindow::setCorner()

    //QToolBar *typesetBar = new QToolBar(ui->toolBar);

    //bar->addMenu(menu);

    //QAction *action = new QAction("Test action", bar);
    //bar->addAction(action);



}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings("QtProject", "Application Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{
    QSettings settings("QtProject", "Application Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

//QCompleter *MainWindow::completer() const {
//    return c;
//}

//void MainWindow::setCompleter(QCompleter *c) {
//    if (c)
//        QObject::disconnect(c, 0, this, 0);
//    c = completer;
//    if (!c)
//        return;

//    c->setWidget(this);
//    c->setCompletionMode(QCompleter::PopupCompletion);
//    c->setCaseSensitivity(Qt::CaseInsensitive);
//    QObject::connect(c, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
//}

//void MainWindow::insertCompletion(const QString &completion) {
//    if (c->widget() != this)
//        return;
//    QTextCursor tc = textCursor();
//    int extra = completion.length() - c->completionPrefix().length();
//    tc.movePosition(QTextCursor::Left);
//    tc.movePosition(QTextCursor::EndOfWord);
//    tc.insertText(completion.right(extra));
//    setTextCursor(tc);
//}

//void MainWindow::keyPressEvent(QKeyEvent *e)
// {
//     if (c && c->popup()->isVisible()) {
//         // The following keys are forwarded by the completer to the widget
//        switch (e->key()) {
//        case Qt::Key_Enter:
//        case Qt::Key_Return:
//        case Qt::Key_Escape:
//        case Qt::Key_Tab:
//        case Qt::Key_Backtab:
//             e->ignore();
//             return; // let the completer do default behavior
//        default:
//            break;
//        }
//     }

//     bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
//     if (!c || !isShortcut) // do not process the shortcut when we have a completer
//         QTextEdit::keyPressEvent(e);

//     const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
//     if (!c || (ctrlOrShift && e->text().isEmpty()))
//         return;

//     static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
//     bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
//     QString completionPrefix = textUnderCursor();

//     if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
//                       || eow.contains(e->text().right(1)))) {
//         c->popup()->hide();
//         return;
//     }

//     if (completionPrefix != c->completionPrefix()) {
//         c->setCompletionPrefix(completionPrefix);
//         c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
//     }
//     QRect cr = cursorRect();
//     cr.setWidth(c->popup()->sizeHintForColumn(0)
//                 + c->popup()->verticalScrollBar()->sizeHint().width());
//     c->complete(cr); // popup it up!
// }

//void MainWindow::focusInEvent(QFocusEvent *e) {
//    if (c)
//        c->setWidget(this);
//    QTextEdit::focusInEvent(e);
//}

//QString MainWindow::textUnderCursor() const {
//    QTextCursor tc = textCursor();
//    tc.select(QTextCursor::WordUnderCursor);
//    return tc.selectedText();
//}

//QCompleter Completer(QTextDocument *doc) {
//    keyword_list << "AND" << "OR" << "NOT" << "EQUIV" << "IMP" << "FA" << "TE" << "MAX" << "FF" << "<=" << ">=" << "SUBOF" << "SUPOF"
//                 << "SUBEQUAL" << "SUPEQUAL" << "!=" << "SUM" << "ELEMOF" << "UNION" << "INTR" << "MTSET" << "UNIV" << "NOELEM" << "THEN"
//                 << "CROSS" << "PROD" << "RHO" << "SIG";
//    QCompleter *keyword_comp = new QCompleter(keyword_list);
//    keyword_comp->setCaseSensitivity(Qt::CaseInsensitive);
//    keyword_comp->setCompletionMode(QCompleter::PopupCompletion);
//    keyword_comp->setModel(new QDirModel(keyword_comp));
//}

bool MainWindow::maybeSave()
{
    if (textEdit->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("ProofBot"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::theorems() {
    if (!theorem_open) {
        //theoremWindow->activateWindow();
        //loadFile();
    }
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("ProofBot"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("ProofBot"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
     out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}


