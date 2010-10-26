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

#include "mpannableviewportscroller.h"
#include "mpannableviewport.h"

QPoint MPannableViewportScroller::queryScrollingAmount(const QGraphicsWidget *widget,
                                                       const QRect &targetRect,
                                                       const QPoint &originPoint)
{
    const MPannableViewport *viewport = static_cast<const MPannableViewport *>(widget);

    if (viewport->panDirection() == 0) {
        return QPoint(); // unable to scroll
    }
/*
    if (viewport == cachedTopmostPannableViewport) {
        // Have to update pannable viewport's range so that we can pan enough.
        ensureTopmostViewportIsPannable();
    }*/

    // First ensure that target rectangle is inside of area of the pannable viewport.
    // Note: We might even move against the wanted direction but this is required to
    // ensure the visibility of the area marked by target rectangle.
    QRect visibleTargetRect(targetRect);
    moveRectInsideArea(viewport->contentsRect().toRect(), visibleTargetRect);

    // Calculate how much pannable contents should be translated.
    const QPoint contentsOffset(visibleTargetRect.topLeft() - originPoint);

    // Calculate the new panning position, i.e. position of the pannable viewport
    // in panned widget coordinates.
    QPointF panningPos(viewport->position() - contentsOffset);

    // Get allowed range for position to be used with MPannableWidget::setPosition().
    QRectF posRange = viewport->range();

    // ...and limit our panning accordingly.
    panningPos.rx() = qBound(posRange.left(), panningPos.x(), posRange.right());
    panningPos.ry() = qBound(posRange.top(), panningPos.y(), posRange.bottom());

    return panningPos.toPoint() - contentsOffset;
}

void MPannableViewportScroller::applyScrolling(QGraphicsWidget *widget, const QPoint &contentsOffset)
{
    MPannableViewport *viewport = static_cast<MPannableViewport *>(widget);

    viewport->setPosition(viewport->position() - contentsOffset);

    // Disables kinetic scrolling until next pointer event.
    viewport->physics()->stop();
}

void MPannableViewportScroller::restoreScrolling(QGraphicsWidget *widget)
{
    Q_UNUSED(widget);
}

