#ifndef PASSWORDGENERATOR_H
#define PASSWORDGENERATOR_H

#pragma once
#include <QString>

namespace PasswordGenerator {
QString generate(int length = 16, bool useSymboles = true, bool useNumbers = true);
}

#endif // PASSWORDGENERATOR_H
