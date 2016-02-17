/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#ifndef LRPREVIEWREPORTWINDOW_H
#define LRPREVIEWREPORTWINDOW_H

#include <QMainWindow>
#include <QDomComment>
#include <QSpinBox>

#include "lrpagedesignintf.h"
#include "lrreportrender.h"
#include "serializators/lrstorageintf.h"
#include "serializators/lrxmlreader.h"

namespace Ui {
class PreviewReportWindow;
}
using namespace LimeReport;
class PreviewReportWindow : public QMainWindow
{
    Q_OBJECT   
public:
    explicit PreviewReportWindow(ReportEnginePrivate *report, QWidget *parent = 0, QSettings* settings=0, Qt::WindowFlags flags=0);
    ~PreviewReportWindow();
    void setReportReader(ItemsReaderIntf::Ptr reader);
    void setPages(ReportPages pages);
    void exec();
    void initPreview(int pagesCount);
    void setSettings(QSettings* value);
    void setErrorMessages(const QStringList& value);
    QSettings* settings();
protected:
    void writeSetting();
    void restoreSetting();
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *e);
    void moveEvent(QMoveEvent *e);
public slots:
    void slotPrint();
    void slotPriorPage();
    void slotNextPage();
    void slotZoomIn();
    void slotZoomOut();
    void slotPageNavigatorChanged(int value);
    void slotShowErrors();
    void on_actionSaveToFile_triggered();
    void slotFirstPage();
    void slotLastPage();
    void slotPrintToPDF();
private slots:
    void slotSliderMoved(int value);
private:
    ItemsReaderIntf* reader();
    bool pageIsVisible(PageItemDesignIntf::Ptr page);
    QRectF calcPageShift(PageItemDesignIntf::Ptr page);
private:
    Ui::PreviewReportWindow *ui;
    QSpinBox* m_pagesNavigator;
    QSharedPointer<ItemsReaderIntf> m_reader;
    int m_currentPage;
    PageDesignIntf* m_previewPage;
    QGraphicsScene* m_simpleScene;
    ReportPages m_reportPages;
    QEventLoop m_eventLoop;
    bool m_changingPage;
    QSettings* m_settings;
    bool m_ownedSettings;
    int m_priorScrolValue;
};

#endif // LRPREVIEWREPORTWINDOW_H