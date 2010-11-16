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

#ifndef MBUTTONSWITCHVIEW_P_H
#define MBUTTONSWITCHVIEW_P_H

#include "mbuttonview_p.h"

class QVariantAnimation;
class MButtonSwitchStyle;

class MButtonSwitchViewPrivate : public MButtonViewPrivate
{
    Q_DECLARE_PUBLIC(MButtonSwitchView)

public:
    MButtonSwitchViewPrivate();
    ~MButtonSwitchViewPrivate();

    QSizeF thumbSize() const;
    QPointF thumbPos() const;
    QPointF thumbEndPos(bool checked) const;  
    void updateHandle();

    void drawSwitchSlider(QPainter *painter) const;
    void drawSwitchThumb(QPainter *painter) const;

    int mouseOffset;

    bool m_thumbDown;
    bool m_thumbDragged;
    bool m_feedbackOnPlayed;
    QPointF m_thumbPos;
    bool m_thumbPosValid;
    
    QVariantAnimation* m_handleAnimation;
    int m_animationSpeed; //pixels per sec
};

#endif
