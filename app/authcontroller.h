#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H
#pragma once

#include <QObject>
#include <QString>

// Bridges QML (Setup / Lock / Welcome screens) to the core layer
// (SaltStore, CryptoManager, Session, ProfileRepository).
//
// Responsibilities:
//   - Decide first-run (setup) vs. returning-user (lock screen)
//   - Create the account: generate salt, derive key, write a verifier
//     file + profile, unlock the Session
//   - Verify a candidate password on later launches by attempting to
//     decrypt verify.enc (fails closed on wrong password or tampering)
//
// This class owns no crypto logic itself - it only sequences calls into
// core/. Nothing here is UI-specific, so it's safe to bind directly to QML.
class AuthController : public QObject
{
    Q_OBJECT

public:
    explicit AuthController(QObject *parent = nullptr);

    // True until an account has been created on this device.
    // Reflects state at construction/last call; call again after
    // createAccount() if you need a fresh read.
    Q_INVOKABLE bool isFirstRun() const;

    // Creates a new account: generates the salt, derives the session key
    // from `password`, writes verify.enc + profile.enc, and unlocks the
    // Session. Returns false (and emits accountCreationFailed) if an
    // account already exists or any step fails.
    Q_INVOKABLE bool createAccount(const QString &name, const QString &password);

    // Attempts to unlock an existing account with `password`. Returns
    // true and unlocks the Session on success; returns false on wrong
    // password or corrupted data. Never distinguishes the two in its
    // return value - both fail the same way, by design.
    Q_INVOKABLE bool tryUnlock(const QString &password);

    // Convenience for the welcome screen: reads the stored display name.
    // Only valid while the Session is unlocked; returns an empty string
    // otherwise.
    Q_INVOKABLE QString currentUserName() const;

    Q_INVOKABLE bool hashVaultPin() const;
    Q_INVOKABLE bool setVaultPin(const QString &pin);
    Q_INVOKABLE bool verifyVaultPin(const QString &pin) const;

signals:
    void accountCreated();
    void accountCreationFailed(const QString &reason);
    void unlockSucceeded();
    void unlockFailed();
};

#endif // AUTHCONTROLLER_H