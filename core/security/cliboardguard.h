#ifndef CLIBOARDGUARD_H
#define CLIBOARDGUARD_H

#pragma once
#include <QString>

namespace CliboardGuard{
    void copyWithAutoClear(const QString &text, int seconds = 15);
}
#endif // CLIBOARDGUARD_H
