#ifndef SALTSTORE_H
#define SALTSTORE_H
#pragma once

#include <QByteArray>

namespace SaltStore{
bool exists();
bool generateAndSave();
bool save(const QByteArray &salt);
QByteArray load();
}

#endif // SALTSTORE_H
