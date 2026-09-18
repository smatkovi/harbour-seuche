/*
    Copyright (C) 2026 smatkovi

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#include <sailfishapp.h>

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>

#include "SeucheEngine.h"

int main(int argc, char *argv[])
{
    QGuiApplication *app = SailfishApp::application(argc, argv);

    // Same pair as [X-Sailjail] in sailfish/desktop/harbour-seuche.desktop, so
    // anything the app ever stores lands inside the sandbox path.
    QCoreApplication::setOrganizationName(QStringLiteral("harbour-seuche"));
    QCoreApplication::setApplicationName(QStringLiteral("harbour-seuche"));

    QQuickView *view = SailfishApp::createView();
    view->rootContext()->setContextProperty(QStringLiteral("engine"),
                                            new SeucheEngine(app));
    view->setSource(SailfishApp::pathTo(QStringLiteral("qml/harbour-seuche.qml")));
    view->show();
    return app->exec();
}
