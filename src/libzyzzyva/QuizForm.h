//---------------------------------------------------------------------------
// QuizForm.h
//
// A form for quizzing the user.
//
// Copyright 2004, 2005, 2006 Michael W Thelen <mike@pietdepsi.com>.
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

#ifndef ZYZZYVA_QUIZ_FORM_H
#define ZYZZYVA_QUIZ_FORM_H

#include "ActionForm.h"
#include "QuizTimerSpec.h"
#include <QCheckBox>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QSqlDatabase>
#include <QStackedWidget>
#include <QString>
#include <QTimer>
#include <QTimerEvent>

class AnalyzeQuizDialog;
class DefinitionLabel;
class QuizCanvas;
class QuizEngine;
class QuizQuestionLabel;
class QuizSpec;
class WordEngine;
class WordLineEdit;
class WordTableModel;
class WordTableView;
class WordValidator;
class ZPushButton;

class QuizForm : public ActionForm
{
    Q_OBJECT
    public:
    QuizForm (WordEngine* we, QWidget* parent = 0, Qt::WFlags f = 0);
    ~QuizForm();
    QString getStatusString() const;
    bool hasUnsavedChanges() const { return unsavedChanges; }

    public slots:
    bool newQuiz (const QuizSpec& spec);
    void responseEntered();
    void alphaOrderClicked();
    void randomOrderClicked();
    void newQuizClicked();
    void saveQuizClicked();
    void nextQuestionClicked();
    void checkResponseClicked();
    void markMissedClicked();
    void pauseClicked();
    void analyzeClicked();
    void flashcardStateChanged (int state);
    void setTileTheme (const QString& theme);
    void setBackgroundColor (const QColor& color);
    void updateStatusString();
    void updateQuestionDisplay();
    void startDisplayingCorrectAnswers();
    void stopDisplayingCorrectAnswers();
    void displayNextCorrectAnswer();
    void selectInputArea();
    bool promptToSaveChanges();

    protected:
    void keyPressEvent (QKeyEvent* event);

    private:
    void updateStats();
    void clearStats();
    void clearTimerDisplay();
    void startQuestion();
    void startNewTimer();
    void killActiveTimer();
    void pauseTimer();
    void unpauseTimer();
    void clearCanvas();
    void minimizeCanvas();
    void setNumCanvasTiles (int num);
    void setQuestionLabel (const QString& question, const QString& order =
                           QString::null);
    void setQuestionStatus (int correct, int total);
    void setStatusString (const QString& status);
    void setTimerDisplay (int seconds);
    void setQuizName (const QString& name);
    void setQuizNameFromFilename (const QString& filename);
    void setUnsavedChanges (bool b);
    void clearTileTheme();
    void reflowLayout();
    bool responseMatchesQuestion (const QString& response) const;
    void connectToDatabase();
    void disconnectDatabase();
    void recordQuestionStats (bool correct);

    void timerEvent (QTimerEvent* event);

    private:
    WordEngine*   wordEngine;
    QuizEngine*   quizEngine;
    QLabel*       quizTypeLabel;
    QLabel*       timerLabel;
    QLabel*       quizNameLabel;
    QStackedWidget* questionStack;
    QuizCanvas*   questionCanvas;
    QuizQuestionLabel* questionLabel;
    QLabel*       questionSpecLabel;
    WordLineEdit* inputLine;
    WordValidator* inputValidator;
    WordTableView* responseView;
    WordTableModel* responseModel;
    DefinitionLabel* responseStatusLabel;
    DefinitionLabel* questionStatusLabel;
    QLabel*       letterOrderLabel;
    ZPushButton*  alphaOrderButton;
    ZPushButton*  randomOrderButton;
    QCheckBox*    flashcardCbox;
    ZPushButton*  nextQuestionButton;
    ZPushButton*  checkResponseButton;
    ZPushButton*  markMissedButton;
    ZPushButton*  pauseButton;
    ZPushButton*  newQuizButton;
    ZPushButton*  saveQuizButton;
    ZPushButton*  analyzeButton;

    int timerId;
    int timerRemaining, timerPaused;
    QuizTimerSpec timerSpec;
    QString statusString;
    QString tileTheme;
    bool checkBringsJudgment;
    bool recordStatsBlocked;
    bool unsavedChanges;

    QSqlDatabase* db;
    QString dbConnectionName;

    QTimer* displayAnswerTimer;
    int currentDisplayAnswer;

    AnalyzeQuizDialog* analyzeDialog;
};

#endif // ZYZZYVA_QUIZ_FORM_H