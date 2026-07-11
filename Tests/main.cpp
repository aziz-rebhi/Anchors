#include "../core/crypto/cryptomanager.h"
#include "../core/storage/encryptedfilestore.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTemporaryDir>


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

    // ===================================================================
    // EncryptedFileStore tests
    // ===================================================================

    // Use a throwaway temp directory so these tests never touch real
    // app data, and clean up after themselves automatically.
    QTemporaryDir tempDir;
    check(tempDir.isValid(), "temp directory for storage tests created");
    QString testFile = tempDir.path() + "/test_vault.enc";

    // --- Test 7: loading a file that doesn't exist yet is NOT an error ---
    bool loadOk = false;
    QJsonDocument missingDoc = EncryptedFileStore::load(testFile, key, &loadOk);
    check(loadOk && missingDoc.isArray() && missingDoc.array().isEmpty(),
          "loading a non-existent file returns an empty array, not an error");

    // --- Test 8: save -> load round-trip returns identical data ---
    QJsonArray entries;
    QJsonObject entry1;
    entry1["title"] = "Gmail";
    entry1["username"] = "me@example.com";
    entry1["password"] = "hunter2";
    entries.append(entry1);
    QJsonDocument originalDoc(entries);

    bool saveOk = EncryptedFileStore::save(testFile, originalDoc, key);
    check(saveOk, "save() succeeds and writes the file");

    bool reloadOk = false;
    QJsonDocument reloaded = EncryptedFileStore::load(testFile, key, &reloadOk);
    check(reloadOk && reloaded == originalDoc,
          "save() -> load() round-trip returns identical JSON");

    // --- Test 9: loading with the wrong session key fails cleanly ---
    bool wrongKeyLoadOk = true; // start true so a bug that skips setting it is caught
    QJsonDocument wrongKeyResult = EncryptedFileStore::load(testFile, wrongKey, &wrongKeyLoadOk);
    check(!wrongKeyLoadOk && wrongKeyResult.isNull(),
          "loading with the wrong session key fails cleanly (no garbage data)");

    // --- Test 10: a corrupted file is rejected, not silently accepted ---
    {
        QFile f(testFile);
        f.open(QIODevice::ReadWrite);
        QByteArray contents = f.readAll();
        if (!contents.isEmpty()) {
            contents[contents.size() - 1] = contents[contents.size() - 1] ^ 0x7F;
        }
        f.seek(0);
        f.write(contents);
        f.close();
    }
    bool corruptedOk = true;
    QJsonDocument corruptedResult = EncryptedFileStore::load(testFile, key, &corruptedOk);
    check(!corruptedOk && corruptedResult.isNull(),
          "a corrupted/tampered file is rejected on load, not silently accepted");

    // --- Test 11: no leftover .tmp file after a successful save (atomic write) ---
    QJsonDocument another(QJsonArray{QJsonObject{{"title", "Second entry"}}});
    EncryptedFileStore::save(testFile, another, key);
    bool tmpLeftBehind = QFile::exists(testFile + ".tmp");
    check(!tmpLeftBehind, "no leftover .tmp file remains after a successful save");



    qInfo() << "";
    qInfo() << "-----------------------------------------";
    qInfo().noquote() << QString("Results: %1 passed, %2 failed").arg(passed).arg(failed);
    qInfo() << "-----------------------------------------";

    return failed == 0 ? 0 : 1;
}