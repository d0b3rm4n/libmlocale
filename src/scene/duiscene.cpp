/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of libdui.
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

#include <QList>
#include <QGraphicsItem>
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QFileInfo>

#include "duidebug.h"
#include <duiondisplaychangeevent.h>
#include "duiapplication.h"
#include "duiapplicationwindow.h"
#include "duiscene.h"
#include "duiscenemanager.h"
#include "duiwidget.h"
#include "duiapplicationpage.h"
#include "duiscene_p.h"

const QFont     TextFont                = QFont("Sans", 10);
const QSize     FpsBoxSize              = QSize(100, 40);
const QColor    FpsTextColor            = QColor(0xFFFF00);
const QFont     FpsFont                 = QFont("Sans", 15);
const int       FpsRefreshInterval      = 1000;
const QString   FpsBackgroundColor      = "#000000";
const qreal     FpsBackgroundOpacity    = 0.35;
const QString   BoundingRectLineColor   = "#00F000";
const QString   BoundingRectFillColor   = "#00F000";
const qreal     BoundingRectOpacity     = 0.1;
const qreal     MarginBackgroundOpacity = 0.4;
const QColor    MarginColor             = QColor(Qt::red);
const int       MarginBorderWidth       = 2;
const QColor    GesturePointColor       = QColor(0xEE7621);
const int       GesturePointSize        = 4;


DuiScenePrivate::DuiScenePrivate() :
        emuPoint1(1),
        emuPoint2(2),
        panEmulationEnabled(false),
        pinchEmulationEnabled(false)
{
}

DuiScenePrivate::~DuiScenePrivate()
{
}

void DuiScenePrivate::setSceneManager(DuiSceneManager *sceneManager)
{
    manager = sceneManager;
}

void DuiScenePrivate::onDisplayChangeEvent(DuiOnDisplayChangeEvent *event)
{
    Q_Q(DuiScene);

    if (event->state() == DuiOnDisplayChangeEvent::FullyOnDisplay ||
            event->state() == DuiOnDisplayChangeEvent::FullyOffDisplay) {
        // Simple cases. Just forward the event as it is.
        sendEventToDuiWidgets(filterDuiWidgets(q->items()), event);
        return;
    }

    QList<DuiWidget *> fullyOnWidgets;
    QList<DuiWidget *> partiallyOnWidgets;
    QList<DuiWidget *> fullyOffWidgets;

    QList<DuiWidget *> intersectingWidgets = filterDuiWidgets(q->items(event->viewRect(),
            Qt::IntersectsItemBoundingRect));

    QList<DuiWidget *> allWidgets = filterDuiWidgets(q->items());

    // Find who is fully on, partially on and fully off

    QSet<DuiWidget *> fullyOffWidgetsSet = allWidgets.toSet().subtract(intersectingWidgets.toSet());
    fullyOffWidgets = fullyOffWidgetsSet.toList();

    int intersectingWidgetsCount = intersectingWidgets.count();
    DuiWidget *widget;
    for (int i = 0; i < intersectingWidgetsCount; i++) {
        widget = intersectingWidgets.at(i);
        if (event->viewRect().contains(widget->sceneBoundingRect())) {
            fullyOnWidgets << widget;
        } else {
            partiallyOnWidgets << widget;
        }
    }

    // Send the events to the corresponding DuiWidgets

    if (fullyOnWidgets.count() > 0) {
        DuiOnDisplayChangeEvent fullyOnEvent(DuiOnDisplayChangeEvent::FullyOnDisplay,
                                             event->viewRect());

        sendEventToDuiWidgets(fullyOnWidgets, &fullyOnEvent);
    }

    if (fullyOffWidgets.count() > 0) {
        DuiOnDisplayChangeEvent fullyOffEvent(DuiOnDisplayChangeEvent::FullyOffDisplay,
                                              event->viewRect());

        sendEventToDuiWidgets(fullyOffWidgets, &fullyOffEvent);
    }

    if (partiallyOnWidgets.count() > 0) {
        DuiOnDisplayChangeEvent partiallyOnEvent(DuiOnDisplayChangeEvent::PartiallyOnDisplay,
                event->viewRect());

        sendEventToDuiWidgets(partiallyOnWidgets, &partiallyOnEvent);
    }

}

QList<DuiWidget *> DuiScenePrivate::filterDuiWidgets(QList<QGraphicsItem *> itemsList)
{
    QGraphicsItem *item;
    DuiWidget *duiWidget;
    QList<DuiWidget *> widgetsList;

    int itemsCount = itemsList.count();
    for (int i = 0; i < itemsCount; i++) {
        item = itemsList.at(i);
        if (item->isWidget()) {
            duiWidget = qobject_cast<DuiWidget *>(static_cast<QGraphicsWidget *>(item));
            if (duiWidget) {
                widgetsList << duiWidget;
            }
        }
    }

    return widgetsList;
}

void DuiScenePrivate::sendEventToDuiWidgets(QList<DuiWidget *> widgetsList, QEvent *event)
{
    Q_Q(DuiScene);
    DuiWidget *widget;

    int widgetsCount = widgetsList.count();
    for (int i = 0; i < widgetsCount; i++) {
        widget = widgetsList.at(i);
        q->sendEvent(widget, event);
    }
}

void DuiScenePrivate::touchPointCopyPosToLastPos(QTouchEvent::TouchPoint &point)
{
    point.setLastPos(point.pos());
    point.setLastScenePos(point.scenePos());
    point.setLastScreenPos(point.screenPos());
}

void DuiScenePrivate::touchPointCopyMousePosToPointPos(QTouchEvent::TouchPoint &point, const QGraphicsSceneMouseEvent *event)
{
    point.setPos(event->pos());
    point.setScenePos(event->scenePos());
    point.setScreenPos(event->screenPos());
}

void DuiScenePrivate::touchPointCopyMousePosToPointStartPos(QTouchEvent::TouchPoint &point, const QGraphicsSceneMouseEvent *event)
{
    point.setStartPos(event->pos());
    point.setStartScenePos(event->scenePos());
    point.setStartScreenPos(event->screenPos());
}

bool DuiScenePrivate::eventEmulatePinch(QEvent* event)
{
    Q_Q(DuiScene);

    bool sendTouchEvent = false;
    QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent*>(event);

    QEvent::Type touchEventType;
    Qt::TouchPointState touchPointState;

    if (QEvent::GraphicsSceneMousePress == event->type()) {
        if (Qt::MidButton == e->button() && Qt::AltModifier == e->modifiers()) {
            pinchEmulationEnabled = true;

            touchPointCopyMousePosToPointPos(emuPoint1,e);
            touchPointCopyMousePosToPointStartPos(emuPoint1,e);
            emuPoint1.setState(Qt::TouchPointPressed);

            touchPointCopyMousePosToPointPos(emuPoint2,e);
            touchPointCopyMousePosToPointStartPos(emuPoint2,e);
            emuPoint2.setState(Qt::TouchPointPressed);

            touchEventType = QEvent::TouchBegin;
            touchPointState = Qt::TouchPointPressed;
            sendTouchEvent = true;
        }
    }

    if (pinchEmulationEnabled && QEvent::GraphicsSceneMouseMove == event->type()) {

        touchPointCopyPosToLastPos(emuPoint1);
        emuPoint1.setState(Qt::TouchPointMoved);

        touchPointCopyPosToLastPos(emuPoint2);
        touchPointCopyMousePosToPointPos(emuPoint2,e);
        emuPoint2.setState(Qt::TouchPointMoved);

        touchEventType = QEvent::TouchUpdate;
        touchPointState = Qt::TouchPointMoved;
        sendTouchEvent = true;
    }

    if (pinchEmulationEnabled && QEvent::GraphicsSceneMouseRelease == event->type()) {
        if (Qt::MidButton == e->button()) {

            touchPointCopyPosToLastPos(emuPoint1);
            emuPoint1.setState(Qt::TouchPointReleased);

            touchPointCopyPosToLastPos(emuPoint2);
            touchPointCopyMousePosToPointPos(emuPoint2,e);
            emuPoint2.setState(Qt::TouchPointReleased);

            touchEventType = QEvent::TouchEnd;
            touchPointState = Qt::TouchPointReleased;
            pinchEmulationEnabled = false;
            sendTouchEvent = true;
        }
    }

    if (sendTouchEvent) {
        QList<QTouchEvent::TouchPoint> touchList;
        touchList.append(emuPoint1);
        touchList.append(emuPoint2);

        QTouchEvent touchEvent(touchEventType, QTouchEvent::TouchPad, Qt::NoModifier, touchPointState, touchList);
        QApplication::sendEvent(q->views()[0]->viewport(), &touchEvent);
        q->update();
        return true;
    }

    return false;
}

bool DuiScenePrivate::eventEmulatePan(QEvent* event)
{
    Q_Q(DuiScene);

    bool sendTouchEvent = false;
    QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent*>(event);

    QEvent::Type touchEventType;
    Qt::TouchPointState touchPointState;

    if (QEvent::GraphicsSceneMousePress == event->type()) {
        if (Qt::MidButton == e->button() && Qt::ControlModifier == e->modifiers()) {

            touchPointCopyMousePosToPointPos(emuPoint1,e);
            touchPointCopyMousePosToPointStartPos(emuPoint1,e);
            emuPoint1.setState(Qt::TouchPointPressed);

            touchPointCopyMousePosToPointPos(emuPoint2,e);
            touchPointCopyMousePosToPointStartPos(emuPoint2,e);
            emuPoint2.setState(Qt::TouchPointPressed);

            touchEventType = QEvent::TouchBegin;
            touchPointState = Qt::TouchPointPressed;
            sendTouchEvent = true;
            panEmulationEnabled = true;
        }
    }

    if (panEmulationEnabled && QEvent::GraphicsSceneMouseMove == event->type()) {

        touchPointCopyPosToLastPos(emuPoint1);
        touchPointCopyMousePosToPointPos(emuPoint1,e);
        emuPoint1.setState(Qt::TouchPointMoved);

        touchPointCopyPosToLastPos(emuPoint2);
        touchPointCopyMousePosToPointPos(emuPoint2,e);
        emuPoint2.setState(Qt::TouchPointMoved);

        touchEventType = QEvent::TouchUpdate;
        touchPointState = Qt::TouchPointPressed;
        sendTouchEvent = true;
    }

    if (panEmulationEnabled && QEvent::GraphicsSceneMouseRelease == event->type()) {
        if (Qt::MidButton == e->button()) {

            touchPointCopyPosToLastPos(emuPoint1);
            touchPointCopyMousePosToPointPos(emuPoint1,e);
            emuPoint1.setState(Qt::TouchPointReleased);

            touchPointCopyPosToLastPos(emuPoint2);
            touchPointCopyMousePosToPointPos(emuPoint2,e);
            emuPoint2.setState(Qt::TouchPointReleased);

            touchEventType = QEvent::TouchEnd;
            touchPointState = Qt::TouchPointReleased;

            panEmulationEnabled = false;
            sendTouchEvent = true;
        }
    }

    if (sendTouchEvent) {
        QList<QTouchEvent::TouchPoint> touchList;
        touchList.append(emuPoint1);
        touchList.append(emuPoint2);

        QTouchEvent touchEvent(touchEventType, QTouchEvent::TouchPad, Qt::NoModifier, touchPointState, touchList);
        QApplication::sendEvent(q->views()[0]->viewport(), &touchEvent);
        event->accept();
        q->update();
        return true;
    }

    return false;
}

void DuiScenePrivate::showFpsCounter(QPainter *painter, float fps)
{
    Q_Q(DuiScene);

    QString display = QString("FPS: %1").arg((int)(fps + 0.5f));
    /* Draw a simple FPS counter in the lower right corner */
    static QRectF fpsRect(0, 0, FpsBoxSize.width(), FpsBoxSize.height());
    if (!q->views().isEmpty()) {
        DuiApplicationWindow *window = qobject_cast<DuiApplicationWindow *>(q->views().at(0));
        if (window) {
            if (manager && manager->orientation() == Dui::Portrait)
                fpsRect.moveTo(QPointF(window->visibleSceneSize().height() - FpsBoxSize.width(),
                                       window->visibleSceneSize().width() - FpsBoxSize.height()));
            else
                fpsRect.moveTo(QPointF(window->visibleSceneSize().width() - FpsBoxSize.width(),
                                       window->visibleSceneSize().height() - FpsBoxSize.height()));
        }
    }

    painter->fillRect(fpsRect, fpsBackgroundBrush);

    painter->setPen(FpsTextColor);
    painter->setFont(FpsFont);
    painter->drawText(fpsRect,
                      Qt::AlignCenter,
                      display);
}

void DuiScenePrivate::logFpsCounter(const QTime *timeStamp, float fps)
{
    /* Open fps log-file, if needed */
    if (!fpsLog.output.isOpen()) {
        QFileInfo fi(qApp->arguments().at(0));
        QString appName = fi.baseName();
        QString pid = QString().setNum(getpid());

        QString fileName = QString("%1-%2.fps").arg(appName, pid);
        fpsLog.output.setFileName(fileName);
        if (!fpsLog.output.open(QIODevice::WriteOnly | QIODevice::Text)) {
            duiWarning("DuiScene::logFpsCounter") << "Could not open fps log file " << fileName;
            DuiApplication::setLogFps(false);
            return;
        } else {
            fpsLog.stream.setDevice(&fpsLog.output);
        }
    }
    fpsLog.stream << timeStamp->toString();
    fpsLog.stream << " " << QString("%1").arg(fps, 0, 'f', 1) << endl;
}

DuiScene::DuiScene(QObject *parent)
    : QGraphicsScene(parent),
      d_ptr(new DuiScenePrivate)
{
    Q_D(DuiScene);

    d->q_ptr = this;
    d->manager = 0;
    QColor fpsBackgroundColor(FpsBackgroundColor);
    fpsBackgroundColor.setAlphaF(FpsBackgroundOpacity);
    d->fpsBackgroundBrush = QBrush(fpsBackgroundColor);

    QColor boundingRectLineColor(BoundingRectLineColor);
    d->boundingRectLinePen = QPen(boundingRectLineColor);

    QColor boundingRectFillColor(BoundingRectFillColor);
    d->boundingRectFillBrush = QBrush(boundingRectFillColor);

    setItemIndexMethod(QGraphicsScene::NoIndex);
}

DuiScene::~DuiScene()
{
    delete d_ptr;
}

DuiSceneManager *DuiScene::sceneManager()
{
    Q_D(DuiScene);

    return d->manager;
}

bool DuiScene::event(QEvent *event)
{
    Q_D(DuiScene);

    if (DuiApplication::emulateTwoFingerGestures()) {
        if (d->eventEmulatePinch(event)) return true;
        if (d->eventEmulatePan(event))   return true;
    }

    return QGraphicsScene::event(event);
}

void DuiScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_D(DuiScene);

    /* Overlay information on the widgets in development mode */
    if (DuiApplication::showBoundingRect() || DuiApplication::showSize() || DuiApplication::showPosition() || DuiApplication::showMargins() || DuiApplication::showObjectNames()) {
        QList<QGraphicsItem *> itemList = items(rect);
        QList<QGraphicsItem *>::iterator item;

        painter->setFont(TextFont);
        QFontMetrics metrics(TextFont);
        int fontHeight = metrics.height();

        QTransform rotationMatrix;
        painter->setTransform(rotationMatrix);

        QList<QGraphicsItem *>::iterator end = itemList.end();
        for (item = itemList.begin(); item != end; ++item) {
            QRectF br = (*item)->boundingRect();
            QPolygonF bp = (*item)->mapToScene(br);

            if (DuiApplication::showBoundingRect()) {
                if (!dynamic_cast<DuiApplicationPage *>(*item)) { // filter out page for clarity
                    painter->setOpacity(BoundingRectOpacity);
                    painter->setPen(d->boundingRectLinePen);
                    painter->setBrush(d->boundingRectFillBrush);
                    painter->drawPolygon(bp);
                }
            }

            if (DuiApplication::showMargins()) {
                // Draw content margins
                QGraphicsLayoutItem *layoutItem = dynamic_cast<QGraphicsLayoutItem *>(*item);
                if (layoutItem) {
                    qreal left, top, right, bottom;
                    layoutItem->getContentsMargins(&left, &top, &right, &bottom);
                    if (left != 0 || top != 0 || right != 0 || bottom != 0) {
                        QPainterPath path;
                        path.addPolygon(bp);
                        path.addPolygon((*item)->mapToScene(br.adjusted(left, top, -right, -bottom)));

                        painter->setOpacity(MarginBackgroundOpacity);
                        painter->fillPath(path, QBrush(MarginColor));
                        painter->setOpacity(1.0);
                        painter->fillPath(path, QBrush(MarginColor, Qt::BDiagPattern));
                        QPen pen(Qt::DashLine);
                        pen.setWidth(MarginBorderWidth);
                        pen.setColor(MarginColor);
                        painter->strokePath(path, pen);
                    }
                }

                painter->setOpacity(1.0);
            }

            if (DuiApplication::showPosition()) {
                QPointF topLeft = ((*item)->mapToScene(br.topLeft()));
                QString posStr = QString("(%1, %2)").arg(topLeft.x()).arg(topLeft.y());
                painter->setPen(Qt::black);
                painter->drawText(topLeft += QPointF(5, fontHeight), posStr);
                painter->setPen(Qt::white);
                painter->drawText(topLeft -= QPointF(1, 1),  posStr);
            }

            if (DuiApplication::showSize()) {
                QPointF bottomRight = ((*item)->mapToScene(br.bottomRight()));
                QString sizeStr = QString("%1x%2").arg(br.width()).arg(br.height());
                painter->setPen(Qt::black);
                painter->drawText(bottomRight -= QPointF(metrics.width(sizeStr), 2), sizeStr);
                painter->setPen(Qt::white);
                painter->drawText(bottomRight -= QPointF(1, 1), sizeStr);
            }

            if (DuiApplication::showObjectNames()) {
                QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(*item);
                if (widget) {
                    QRectF boundingRect = bp.boundingRect();
                    QString name = widget->objectName();
                    QRect textBoundingRect = metrics.boundingRect(name);
                    QPointF center = boundingRect.topLeft();
                    center += QPointF((boundingRect.width() - textBoundingRect.width()) / 2, (boundingRect.height() - fontHeight) / 2);
                    painter->fillRect(textBoundingRect.translated(center.toPoint()), d->fpsBackgroundBrush);
                    painter->setPen(FpsTextColor);
                    painter->drawText(center, name);
                }
            }
        }
    }

    bool countingFps = DuiApplication::logFps() || DuiApplication::showFps();
    if (countingFps) {
        if (d->fps.frameCount < 0) {
            d->fps.time.restart();
            d->fps.frameCount = 0;
        }
        ++d->fps.frameCount;

        int ms = d->fps.time.elapsed();
        if (ms > FpsRefreshInterval) {
            float fps = d->fps.frameCount * 1000.0f / ms;
            d->fps.time.restart();
            d->fps.frameCount = 0;
            if (DuiApplication::logFps()) {
                QTime time = d->fps.time.currentTime();
                d->logFpsCounter(&time, fps);
            }
            d->fps.fps = fps;
        }

        if (DuiApplication::showFps()) {
            d->showFpsCounter(painter, d->fps.fps);
        }

        // this update call makes repainting work as fast
        // as possible, and by that prints useful fps numbers
        QTimer::singleShot(0, this, SLOT(update()));
    } else {
        d->fps.frameCount = -1;
    }

    if (DuiApplication::emulateTwoFingerGestures() &&
        (d->panEmulationEnabled || d->pinchEmulationEnabled))
    {
        painter->setBrush(GesturePointColor);
        painter->drawEllipse(d->emuPoint1.screenPos(),GesturePointSize,GesturePointSize);
        painter->drawEllipse(d->emuPoint2.screenPos(),GesturePointSize,GesturePointSize);
    }
}
