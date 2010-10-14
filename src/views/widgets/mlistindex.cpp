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

#include "mlistindex.h"
#include "mlistindexview.h"
#include "mlistindexfloatingview.h"
#include "mlist.h"

#include <QGraphicsSceneContextMenuEvent>

#include "mwidgetcreator.h"
M_REGISTER_WIDGET(MListIndex)

MListIndex::MListIndex(MList *parent) : MWidgetController(new MListIndexModel, parent)
{
    setView(new MListIndexView(this));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    model()->setList(parent);

    grabGesture(Qt::TapAndHoldGesture);
}

MListIndex::~MListIndex()
{
}

void MListIndex::setShortcuts(const QMap<QModelIndex, QString> &shortcuts)
{
    model()->beginTransaction();
    model()->setShortcutIndexes(shortcuts.keys());
    model()->setShortcutLabels(shortcuts.values());
    model()->commitTransaction();
}

void MListIndex::setList(MList *list)
{
    model()->setList(list);
}

void MListIndex::setDisplayMode(MList::DisplayMode displayMode)
{
    if (displayMode == MList::Floating)
        setView(new MListIndexFloatingView(this));
    else if (QString::compare(view()->metaObject()->className(), "MListIndexView") != 0)
        setView(new MListIndexView(this));

    model()->setDisplayMode(displayMode);
}

MList::DisplayMode MListIndex::displayMode()
{
    return (MList::DisplayMode)model()->displayMode();
}

void MListIndex::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();
}
