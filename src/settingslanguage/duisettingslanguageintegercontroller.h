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

#ifndef DUISETTINGSLANGUAGEINTEGERCONTROLLER_H_
#define DUISETTINGSLANGUAGEINTEGERCONTROLLER_H_

#include <QObject>

//! \internal
/*!
 * An interaction controller for integer items in settings language.
 */
class DuiSettingsLanguageIntegerController : public QObject
{
    Q_OBJECT

public:
    /*!
     * Constructor.
     * \param parent the parent object for this object.
     */
    DuiSettingsLanguageIntegerController(QObject *parent = NULL);

    /*!
     * Destructor.
     */
    virtual ~DuiSettingsLanguageIntegerController();

public slots:
    /*!
     * Reacts to value changes. Stores the new value to the storage backend.
     *
     * The widget should have the following properties set:
     * - key: the name of the value
     *
     * \param newValue the new value
     */
    void changeValue(int newValue);
};
//! \internal_end

#endif /* DUISETTINGSLANGUAGEINTEGERCONTROLLER_H_ */
