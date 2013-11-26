/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
*
* Some code was taken from qtabwidget.cpp
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#include "tabstackedwidget.h"
#include "combotabbar.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QKeyEvent>
#include <QApplication>

// Note: just some of QTabWidget's methods were implemented

TabStackedWidget::TabStackedWidget(QWidget* parent)
    : QWidget(parent)
    , m_stack(0)
    , m_tabBar(0)
{
    m_stack = new QStackedWidget(this);
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(m_stack);
    setLayout(m_mainLayout);

    setTabBar(new ComboTabBar);
    connect(m_stack, SIGNAL(widgetRemoved(int)), this, SLOT(tabWasRemoved(int)));
}

TabStackedWidget::~TabStackedWidget()
{
}

ComboTabBar* TabStackedWidget::tabBar()
{
    return m_tabBar;
}

void TabStackedWidget::setTabBar(ComboTabBar* tb)
{
    Q_ASSERT(tb);

    if (tb->parentWidget() != this) {
        tb->setParent(this);
        tb->show();
    }

    delete m_tabBar;
    m_dirtyTabBar = true;
    m_tabBar = tb;
    setFocusProxy(m_tabBar);

    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(showTab(int)));
    connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(tabWasMoved(int,int)));

    if (m_tabBar->tabsClosable()) {
        connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SIGNAL(tabCloseRequested(int)));
    }

    setDocumentMode(m_tabBar->documentMode());

    m_tabBar->installEventFilter(this);
    setUpLayout();
}

bool TabStackedWidget::isValid(int index)
{
    return (index < m_stack->count() && index >= 0);
}

void TabStackedWidget::tabWasRemoved(int index)
{
    m_tabBar->removeTab(index);
}

void TabStackedWidget::tabWasMoved(int from, int to)
{
    m_stack->blockSignals(true);
    QWidget* w = m_stack->widget(from);
    m_stack->removeWidget(w);
    m_stack->insertWidget(to, w);
    m_stack->setCurrentIndex(currentIndex());
    m_stack->blockSignals(false);
}

void TabStackedWidget::setUpLayout()
{
    if (!m_tabBar->isVisible()) {
        m_dirtyTabBar = true;
        return;
    }

    m_tabBar->setElideMode(m_tabBar->elideMode());
    m_dirtyTabBar = false;
}

bool TabStackedWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (m_dirtyTabBar && obj == m_tabBar && event->type() == QEvent::Show) {
        setUpLayout();
    }

    return false;
}

void TabStackedWidget::keyPressEvent(QKeyEvent* event)
{
    if (((event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) &&
            count() > 1 && event->modifiers() & Qt::ControlModifier)
#ifdef QT_KEYPAD_NAVIGATION
            || QApplication::keypadNavigationEnabled() && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) && count() > 1
#endif
       ) {
        int pageCount = count();
        int page = currentIndex();
        int dx = (event->key() == Qt::Key_Backtab || event->modifiers() & Qt::ShiftModifier) ? -1 : 1;
#ifdef QT_KEYPAD_NAVIGATION
        if (QApplication::keypadNavigationEnabled() && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)) {
            dx = event->key() == (isRightToLeft() ? Qt::Key_Right : Qt::Key_Left) ? -1 : 1;
        }
#endif
        for (int pass = 0; pass < pageCount; ++pass) {
            page += dx;
            if (page < 0
#ifdef QT_KEYPAD_NAVIGATION
                    && !event->isAutoRepeat()
#endif
               ) {
                page = count() - 1;
            }
            else if (page >= pageCount
#ifdef QT_KEYPAD_NAVIGATION
                     && !event->isAutoRepeat()
#endif
                    ) {
                page = 0;
            }
            if (m_tabBar->isTabEnabled(page)) {
                setCurrentIndex(page);
                break;
            }
        }
        if (!QApplication::focusWidget()) {
            m_tabBar->setFocus();
        }
    }
    else {
        event->ignore();
    }
}

void TabStackedWidget::showTab(int index)
{
    if (isValid(index)) {
        m_stack->setCurrentIndex(index);
    }

    emit currentChanged(index);
}

bool TabStackedWidget::documentMode() const
{
    return m_tabBar->documentMode();
}

void TabStackedWidget::setDocumentMode(bool enabled)
{
    m_tabBar->setDocumentMode(enabled);
    m_tabBar->setExpanding(!enabled);
    m_tabBar->setDrawBase(enabled);
}

int TabStackedWidget::addTab(QWidget* widget, const QString &label, bool pinned)
{
    return insertTab(-1, widget, label, pinned);
}

int TabStackedWidget::insertTab(int index, QWidget* w, const QString &label, bool pinned)
{
    if (!w) {
        return -1;
    }

    if (pinned) {
        index = index < 0 ? m_tabBar->pinnedTabsCount() : qMin(index, m_tabBar->pinnedTabsCount());
        index = m_stack->insertWidget(index, w);
        m_tabBar->insertTab(index, QIcon(), label, true);
    }
    else {
        index = index < 0 ? -1 : qMax(index, m_tabBar->pinnedTabsCount());
        index = m_stack->insertWidget(index, w);
        m_tabBar->insertTab(index, QIcon(), label, false);
    }

    return index;
}

QString TabStackedWidget::tabText(int index) const
{
    return m_tabBar->tabText(index);
}

void TabStackedWidget::setTabText(int index, const QString &label)
{
    m_tabBar->setTabText(index, label);
}

QString TabStackedWidget::tabToolTip(int index) const
{
    return m_tabBar->tabToolTip(index);
}

void TabStackedWidget::setTabToolTip(int index, const QString &tip)
{
    m_tabBar->setTabToolTip(index, tip);
}

int TabStackedWidget::pinUnPinTab(int index, const QString &title)
{
    int newIndex = -1;
    if (QWidget* w = m_stack->widget(index)) {
        QWidget* button = m_tabBar->tabButton(index, m_tabBar->iconButtonPosition());
        m_tabBar->setTabButton(index, m_tabBar->iconButtonPosition(), 0);
        if (index < m_tabBar->pinnedTabsCount()) {
            // Unpin
            // fix selecting and loading a tab after removing the tab that contains 'w'
            // by blocking ComboTabBar::currentChanged()
            m_tabBar->blockSignals(true);
            m_stack->removeWidget(w);
            m_tabBar->blockSignals(false);
            newIndex = insertTab(m_tabBar->pinnedTabsCount(), w, title, false);
        }
        else {
            // Pin // same as above
            m_tabBar->blockSignals(true);
            m_stack->removeWidget(w);
            m_tabBar->blockSignals(false);
            newIndex = insertTab(0, w, QString(), true);
        }
        m_tabBar->setTabButton(newIndex, m_tabBar->iconButtonPosition(), button);
    }

    return newIndex;
}

void TabStackedWidget::removeTab(int index)
{
    if (QWidget* w = m_stack->widget(index)) {
        m_stack->removeWidget(w);
    }
}

int TabStackedWidget::currentIndex() const
{
    return m_tabBar->currentIndex();
}

void TabStackedWidget::setCurrentIndex(int index)
{
    m_tabBar->setCurrentIndex(index);
}

QWidget* TabStackedWidget::currentWidget() const
{
    return m_stack->currentWidget();
}

void TabStackedWidget::setCurrentWidget(QWidget* widget)
{
    m_tabBar->setCurrentIndex(indexOf(widget));
}

QWidget* TabStackedWidget::widget(int index) const
{
    return m_stack->widget(index);
}

int TabStackedWidget::indexOf(QWidget* widget) const
{
    return m_stack->indexOf(widget);
}

int TabStackedWidget::count() const
{
    return m_tabBar->count();
}
