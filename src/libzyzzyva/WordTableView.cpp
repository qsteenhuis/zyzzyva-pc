//---------------------------------------------------------------------------
// WordTableView.cpp
//
// A class derived from QTableView, used to display word lists.
//
// Copyright 2005, 2006 Michael W Thelen <mike@pietdepsi.com>.
//
// This file is part of Zyzzyva.
//
// Zyzzyva is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Zyzzyva is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//---------------------------------------------------------------------------

#include "WordTableView.h"
#include "DefinitionDialog.h"
#include "MainWindow.h"
#include "QuizSpec.h"
#include "SearchSpec.h"
#include "WordEngine.h"
#include "WordTableDelegate.h"
#include "WordTableModel.h"
#include "WordVariationDialog.h"
#include "WordVariationType.h"
#include "Auxil.h"
#include <QAction>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QSignalMapper>
#include <QTextStream>
#include <QToolTip>

using namespace std;

//---------------------------------------------------------------------------
//  WordTableView
//
//! Constructor.
//
//! @param parent the parent object
//---------------------------------------------------------------------------
WordTableView::WordTableView (WordEngine* e, QWidget* parent)
    : QTreeView (parent), wordEngine (e)
{
    setFocusPolicy (Qt::NoFocus);
    setSelectionBehavior (QAbstractItemView::SelectRows);
    setSelectionMode (QAbstractItemView::SingleSelection);
    setRootIsDecorated (false);

    WordTableDelegate* delegate = new WordTableDelegate (this);
    setItemDelegate (delegate);

    // FIXME: Once Trolltech fixes the assert in QHeaderView, continue with
    // statements like these
    //if (!MainSettings::getWordListShowHooks()) {
    //    setColumnHidden (WordTableModel::FRONT_HOOK_COLUMN, true);
    //    setColumnHidden (WordTableModel::BACK_HOOK_COLUMN, true);
    //}
}

//---------------------------------------------------------------------------
//  resizeItemsToContents
//
//! Resize all columns to fit the model contents.
//---------------------------------------------------------------------------
void
WordTableView::resizeItemsToContents()
{
//    for (int i = 0; i < model()->rowCount(); ++i)
//        resizeRowToContents (i);
    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents (i);
}

//---------------------------------------------------------------------------
//  viewDefinition
//
//! Display the definition of the currently selected word.
//---------------------------------------------------------------------------
void
WordTableView::viewDefinition()
{
    QModelIndex index = currentIndex();
    index = index.sibling (index.row(), WordTableModel::WORD_COLUMN);
    QString word = model()->data (index, Qt::EditRole).toString();
    DefinitionDialog* dialog = new DefinitionDialog (wordEngine, word, this);
    Q_CHECK_PTR (dialog);
    dialog->setAttribute (Qt::WA_DeleteOnClose);
    dialog->show();
}

//---------------------------------------------------------------------------
//  viewVariation
//
//! Display a variation of the currently selected word.
//
//! @param variation the variation to display
//---------------------------------------------------------------------------
void
WordTableView::viewVariation (int variation)
{
    QModelIndex index = currentIndex();
    index = index.sibling (index.row(), WordTableModel::WORD_COLUMN);
    QString word = model()->data (index, Qt::EditRole).toString();
    WordVariationType type = static_cast<WordVariationType>(variation);
    WordVariationDialog* dialog = new WordVariationDialog (wordEngine, word,
                                                           type, this);
    dialog->setAttribute (Qt::WA_DeleteOnClose);
    Q_CHECK_PTR (dialog);
    dialog->show();
}

//---------------------------------------------------------------------------
//  exportRequested
//
//! Called when the user indicates that the word list should be exported.
//! Display a dialog to let the user choose where to export the list, then
//! actually do the export.
//---------------------------------------------------------------------------
void
WordTableView::exportRequested()
{
    if (model()->rowCount() == 0) {
        QMessageBox::warning (this, "Cannot Save Word List",
                              "Cannot save word list:\nNo words to save.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName (this, "Save Word List",
        Auxil::getUserWordsDir() + "/saved", "Text Files (*.txt)");

    if (filename.isEmpty())
        return;

    if (!filename.endsWith (".txt", Qt::CaseInsensitive))
        filename += ".txt";

    QString error;
    if (!exportFile (filename, &error)) {
        QMessageBox::warning (this, "Error Saving Word List",
                              "Cannot save word list:\n" + error + ".");
    }
}

//---------------------------------------------------------------------------
//  createQuizRequested
//
//! Called when the user indicates that the word list should be used to create
//! a quiz.  Display a quiz dialog with a single In Word List condition
//! already filled in with the contents of the list.
//---------------------------------------------------------------------------
void
WordTableView::createQuizRequested()
{
    int numWords = model()->rowCount();
    if (numWords == 0) {
        QMessageBox::warning (this, "Error Creating Quiz",
                              "Cannot create quiz:\nNo words in the list.");
    }

    QuizSpec quizSpec;
    SearchSpec searchSpec;
    SearchCondition searchCondition;
    QString searchString;
    QModelIndex index = model()->index (0, WordTableModel::WORD_COLUMN);
    for (int i = 0; i < numWords; ) {
        if (!searchString.isEmpty())
            searchString += " ";
        searchString += model()->data (index, Qt::EditRole).toString();
        index = index.sibling (++i, WordTableModel::WORD_COLUMN);
    }

    searchCondition.type = SearchCondition::InWordList;
    searchCondition.stringValue = searchString;
    searchSpec.conditions.push_back (searchCondition);
    quizSpec.setSearchSpec (searchSpec);
    MainWindow::getInstance()->newQuizFormInteractive (quizSpec);
}

//---------------------------------------------------------------------------
//  exportFile
//
//! Export the words in the list to a file, one word per line.
//
//! @param filename the name of the file
//
//! @return true if successful or if the user cancels, false otherwise
//---------------------------------------------------------------------------
bool
WordTableView::exportFile (const QString& filename, QString* err) const
{
    if (model()->rowCount() == 0) {
        if (err)
            *err = "No words to save";
        return false;
    }

    QFile file (filename);
    if (!file.open (QIODevice::WriteOnly | QIODevice::Text)) {
        if (err)
            *err = file.errorString();
        return false;
    }

    QTextStream stream (&file);
    QModelIndex index = model()->index (0, WordTableModel::WORD_COLUMN);
    for (int i = 0; i < model()->rowCount(); ) {
        QString word = model()->data (index, Qt::EditRole).toString();
        stream << word;
        endl (stream);
        index = index.sibling (++i, WordTableModel::WORD_COLUMN);
    }

    return true;
}

//---------------------------------------------------------------------------
//  hookToolTipText
//
//! This event handler, for event e, can be reimplemented in a subclass to
//! receive widget context menu events.
//
//! @param word the word whose hooks to display
//! @param hooks the hooks to display
//! @param front true if the hooks are fronthooks, false if back hooks
//
//! @return the tool tip text
//---------------------------------------------------------------------------
QString
WordTableView::hookToolTipText (const QString& word, const QString& hooks,
                                bool front) const
{
    QString text;

    QString hook;
    foreach (hook, hooks) {
        QString hookWord = (front ? (hook.toUpper() + word)
                                  : (word + hook.toUpper()));
        if (!text.isEmpty())
            text += "\n\n";

        text += hookWord + " : " + wordEngine->getDefinition (hookWord);
    }

    return text;
}

//---------------------------------------------------------------------------
//  contextMenuEvent
//
//! This event handler, for event e, can be reimplemented in a subclass to
//! receive widget context menu events.
//
//! @param e the context menu event
//
//! @return true if successful or if the user cancels, false otherwise
//---------------------------------------------------------------------------
void
WordTableView::contextMenuEvent (QContextMenuEvent* e)
{
    QModelIndex index = indexAt (e->pos());
    bool wordOptions = (index.isValid() &&
                        (index.row() >= 0) && (index.column() >= 0));

    QMenu* popupMenu = new QMenu;
    Q_CHECK_PTR (popupMenu);

    if (wordOptions) {
        QMenu* wordMenu = new QMenu ("Word");
        Q_CHECK_PTR (wordMenu);
        popupMenu->addMenu (wordMenu);

        QSignalMapper* wordMapper = new QSignalMapper (wordMenu);
        Q_CHECK_PTR (wordMapper);

        // Word Definition
        QAction* definitionAction = new QAction ("Definition", wordMenu);
        Q_CHECK_PTR (definitionAction);
        connect (definitionAction, SIGNAL (triggered()),
                 SLOT (viewDefinition()));
        wordMenu->addAction (definitionAction);

        // Word Anagrams
        QAction* anagramsAction = new QAction ("Anagrams", wordMenu);
        Q_CHECK_PTR (anagramsAction);
        connect (anagramsAction, SIGNAL (triggered()),
                 wordMapper, SLOT (map()));
        wordMapper->setMapping (anagramsAction, VariationAnagrams);
        wordMenu->addAction (anagramsAction);

        // Word Subanagrams
        QAction* subanagramsAction = new QAction ("Subanagrams", wordMenu);
        Q_CHECK_PTR (subanagramsAction);
        connect (subanagramsAction, SIGNAL (triggered()),
                 wordMapper, SLOT (map()));
        wordMapper->setMapping (subanagramsAction, VariationSubanagrams);
        wordMenu->addAction (subanagramsAction);

        // Word Hooks
        QAction* hooksAction = new QAction ("Hooks", wordMenu);
        Q_CHECK_PTR (hooksAction);
        connect (hooksAction, SIGNAL (triggered()), wordMapper, SLOT (map()));
        wordMapper->setMapping (hooksAction, VariationHooks);
        wordMenu->addAction (hooksAction);

        // Word Extensions
        QAction* extensionsAction = new QAction ("Extensions", wordMenu);
        Q_CHECK_PTR (extensionsAction);
        connect (extensionsAction, SIGNAL (triggered()),
                 wordMapper, SLOT (map()));
        wordMapper->setMapping (extensionsAction, VariationExtensions);
        wordMenu->addAction (extensionsAction);

        // Word Anagram Hooks
        QAction* anagramHooksAction = new QAction ("Anagram Hooks", wordMenu);
        Q_CHECK_PTR (anagramHooksAction);
        connect (anagramHooksAction, SIGNAL (triggered()),
                 wordMapper, SLOT (map()));
        wordMapper->setMapping (anagramHooksAction, VariationAnagramHooks);
        wordMenu->addAction (anagramHooksAction);

        // Word Blank Anagrams
        QAction* blankAnagramsAction = new QAction ("Blank Anagrams",
                                                    wordMenu);
        Q_CHECK_PTR (blankAnagramsAction);
        connect (blankAnagramsAction, SIGNAL (triggered()),
                 wordMapper, SLOT (map()));
        wordMapper->setMapping (blankAnagramsAction, VariationBlankAnagrams);
        wordMenu->addAction (blankAnagramsAction);

        // Word Blank Matches
        QAction* blankMatchesAction = new QAction ("Blank Matches", wordMenu);
        Q_CHECK_PTR (blankMatchesAction);
        connect (blankMatchesAction, SIGNAL (triggered()),
                 wordMapper, SLOT (map()));
        wordMapper->setMapping (blankMatchesAction, VariationBlankMatches);
        wordMenu->addAction (blankMatchesAction);

        // Word Transpositions
        QAction* transpositionsAction = new QAction ("Transpositions",
                                                     wordMenu);
        Q_CHECK_PTR (transpositionsAction);
        connect (transpositionsAction, SIGNAL (triggered()),
                 wordMapper, SLOT (map()));
        wordMapper->setMapping (transpositionsAction,
                                VariationTranspositions);
        wordMenu->addAction (transpositionsAction);

        // Connect Word signal mappings to viewVariation
        connect (wordMapper, SIGNAL (mapped (int)),
                 SLOT (viewVariation (int)));
    }

    QAction* exportAction = new QAction ("Save list...", popupMenu);
    Q_CHECK_PTR (exportAction);
    connect (exportAction, SIGNAL (triggered()), SLOT (exportRequested()));
    popupMenu->addAction (exportAction);

    QAction* createQuizAction = new QAction ("Quiz from list...", popupMenu);
    Q_CHECK_PTR (createQuizAction);
    connect (createQuizAction, SIGNAL (triggered()),
             SLOT (createQuizRequested()));
    popupMenu->addAction (createQuizAction);

    popupMenu->exec (QCursor::pos());
    delete popupMenu;
}

//---------------------------------------------------------------------------
//  viewportEvent
//
//! This event handler, for event e, can be reimplemented in a subclass to
//! receive viewport events.
//
//! @param e the viewport event
//
//! @return true if successful, false otherwise
//---------------------------------------------------------------------------
bool
WordTableView::viewportEvent (QEvent* e)
{
    switch (e->type()) {
        case QEvent::ToolTip: {
            QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
            QModelIndex index = indexAt (helpEvent->pos());
            if (!index.isValid())
                break;

            QString toolTipText;
            switch (index.column()) {
                case WordTableModel::DEFINITION_COLUMN:
                toolTipText = index.model()->data (index,
                                                   Qt::DisplayRole).toString();
                break;

                case WordTableModel::FRONT_HOOK_COLUMN:
                case WordTableModel::BACK_HOOK_COLUMN: {
                    QString hooks =
                        index.model()->data (index, Qt::EditRole).toString();
                    QString word = index.model()->data (index.sibling
                            (index.row(), WordTableModel::WORD_COLUMN),
                            Qt::EditRole).toString();
                    bool front = (index.column() ==
                                  WordTableModel::FRONT_HOOK_COLUMN);

                    toolTipText = hookToolTipText (word, hooks, front);
                }
                break;

                default:
                break;
            }

            if (!toolTipText.isEmpty())
                QToolTip::showText (helpEvent->globalPos(), toolTipText);
        }
        break;

        default:
        QTreeView::viewportEvent (e);
        break;
    }

    return true;
}

//---------------------------------------------------------------------------
//  sizeHintForColumn
//
//! Return the width size hint for a column.  Directly call the equivalent
//! function from QAbstractItemView instead of allowing the QTableView version
//! to take precedence, because we want the content of all rows to be
//! considered, not just the visible ones.
//
//! @param column the column index
//! @return the size hint for the column
//---------------------------------------------------------------------------
int
WordTableView::sizeHintForColumn (int column) const
{
    return QAbstractItemView::sizeHintForColumn (column) +
        (2 * WordTableDelegate::ITEM_XPADDING);
}

//---------------------------------------------------------------------------
//  sizeHintForRow
//
//! Return the height size hint for a row.  Directly call the equivalent
//! function from QAbstractItemView instead of allowing the QTableView version
//! to take precedence, because we want the content of all columns to be
//! considered, not just the visible ones.
//
//! @param column the column index
//! @return the size hint for the column
//---------------------------------------------------------------------------
int
WordTableView::sizeHintForRow (int row) const
{
    return QAbstractItemView::sizeHintForRow (row) +
        (2 * WordTableDelegate::ITEM_YPADDING);
}