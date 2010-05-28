/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of libmeegotouch.
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#ifndef UT_MTOOLBARVIEW_H
#define UT_MTOOLBARVIEW_H

#include <QObject>
#include <QtTest/QtTest>

class MToolBar;
class MToolBarView;
class MWidget;
class MWidgetAction;

class Ut_MToolBarView : public QObject
{
    Q_OBJECT
public:

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testInsertingActions();
    void testAddingRemoveActions_data();
    void testAddingRemoveActions();
    void testDeleteAddAction();
    void testDeleteAction();
    void testDeleteAndAddingTextWidget();
    void testTabView();
    void testTabActionExclusivity();
    void testSizeHint_data();
    void testSizeHint();
    void testButtons();
    void testMWidgetAction_data();
    void testMWidgetAction();

private:
    MToolBar *m_toolbar;
    MToolBarView *m_toolbarview;

    MWidgetAction *createTextEditAction(MWidget *parentWidget);
};

#endif

