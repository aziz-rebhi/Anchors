#include "../core/crypto/cryptomanager.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!CryptoManager::init()) {
        qCritical() << "FATAL: libsodium failed to initialize";
        return 1;
    }

    int passed = 0;
    int failed = 0;
    auto check = [&](bool condition, const QString &name) {
        if (condition) {
            qInfo() << "PASS:" << name;
            ++passed;
        } else {
            qWarning() << "FAIL:" << name;
            ++failed;
        }
    };

    // --- Test 1: key derivation produces the expected key size ---
    QByteArray salt = CryptoManager::generateSalt();
    QByteArray key = CryptoManager::deriveKey("correct horse battery staple", salt);
    check(key.size() == 32, "deriveKey() produces a 32-byte key");

    // --- Test 2: encrypt -> decrypt round-trip returns the original data ---
    QByteArray original = "This is a secret vault entry: hunter2";
    QByteArray encrypted = CryptoManager::encrypt(original, key);
    check(!encrypted.isEmpty(), "encrypt() produces non-empty output");

    bool ok = false;
    QByteArray decrypted = CryptoManager::decrypt(encrypted, key, &ok);
    check(ok && decrypted == original, "decrypt() round-trip matches the original plaintext exactly");

    // --- Test 3: wrong password derives a different key, and decryption fails cleanly ---
    QByteArray wrongKey = CryptoManager::deriveKey("wrong password entirely", salt);
    bool wrongOk = false;
    QByteArray wrongResult = CryptoManager::decrypt(encrypted, wrongKey, &wrongOk);
    check(!wrongOk && wrongResult.isEmpty(), "wrong master password fails decryption cleanly (no partial data)");

    // --- Test 4: tampering with even one byte of ciphertext is detected and rejected ---
    QByteArray tampered = encrypted;
    tampered[tampered.size() - 1] = static_cast<char>(tampered[tampered.size() - 1] ^ 0xFF);
    bool tamperedOk = false;
    QByteArray tamperedResult = CryptoManager::decrypt(tampered, key, &tamperedOk);
    check(!tamperedOk && tamperedResult.isEmpty(), "tampered ciphertext is rejected (authentication catches it)");

    // --- Test 5: encrypting the same plaintext twice gives different ciphertext ---
    // (proves the nonce is actually random each time, not reused)
    QByteArray encrypted2 = CryptoManager::encrypt(original, key);
    check(encrypted != encrypted2, "encrypting identical plaintext twice yields different ciphertext (nonce randomness)");

    // --- Test 6: a truncated/garbage buffer is rejected instead of crashing ---
    QByteArray garbage = "not even close to valid ciphertext";
    bool garbageOk = false;
    QByteArray garbageResult = CryptoManager::decrypt(garbage, key, &garbageOk);
    check(!garbageOk && garbageResult.isEmpty(), "garbage/too-short input is rejected without crashing");

    qInfo() << "";
    qInfo() << "-----------------------------------------";
    qInfo().noquote() << QString("Results: %1 passed, %2 failed").arg(passed).arg(failed);
    qInfo() << "-----------------------------------------";

    return failed == 0 ? 0 : 1;
}