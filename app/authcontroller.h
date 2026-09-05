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
    Q_INVOKABLE bool verifyVaultPin(const QString &pin);

    // Master-password throttling: after repeated wrong guesses the unlock
    // path refuses to even run Argon2 until the lockout window lapses.
    Q_INVOKABLE bool lockedOut() const;
    Q_INVOKABLE int lockoutSecondsRemaining() const;

    // Same idea, applied to PIN verification (independent counters).
    Q_INVOKABLE bool pinLockedOut() const;
    Q_INVOKABLE int pinLockoutSecondsRemaining() const;

signals:
    void accountCreated();
    void accountCreationFailed(const QString &reason);
    void unlockSucceeded();
    void unlockFailed();
    void lockStateChanged();
    void pinLockStateChanged();

private:
    static qint64 lockoutDurationMs(int consecutiveFailures);

    int m_failedAttempts = 0;
    qint64 m_lockUntilMs = 0;
    int m_pinFailedAttempts = 0;
    qint64 m_pinLockUntilMs = 0;
};

#endif // AUTHCONTROLLER_H