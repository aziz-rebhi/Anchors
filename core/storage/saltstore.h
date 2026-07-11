#ifndef SALTSTORE_H
#define SALTSTORE_H
#pragma once

#include <QByteArray>

namespace SaltStore{
bool exists();
bool generateAndSave();
QByteArray load();
}

#endif // SALTSTORE_H
