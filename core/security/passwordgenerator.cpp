#include "passwordgenerator.h"

#include <QRandomGenerator>
QString PasswordGenerator::generate(int length, bool useSymbols, bool useNumbers){
    if (length < 8) length = 8;

    static const QString letters = "acdefghijklmnopqrstuvwxyz";
    static const QString numbers = "0123456789";
    static const QString symbols = "!@#$%^&*()-_=+[]{}";

    QString pool = letters;
    if (useNumbers) pool += numbers;
    if (useSymbols) pool += symbols;

    QRandomGenerator *rng = QRandomGenerator::system();

    QString result;
    result.reserve(length);
    for (int i = 0; i < length; i++){
        int index = rng->bounded(pool.size());
        result.append(pool.at(index));
    }

    return result;
}