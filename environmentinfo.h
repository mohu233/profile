#ifndef ENVIRONMENTINFO_H
#define ENVIRONMENTINFO_H

#include <QString>
#include <QStringList>

struct EnvironmentInfo
{
    int id;

    QString name;
    QString status;
    QString group;

    QStringList modules;
};

#endif