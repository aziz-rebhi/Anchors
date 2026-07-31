import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: window
    width: 480
    height: 720
    visible: true
    title: "Anchor"

    Theme { id: theme }

    color: theme.background
    font.family: theme.bodyFont



    StackView {
        id: stack
        anchors.fill: parent

        Component.onCompleted: {
            if (authController.isFirstRun()) {
                stack.push(setupPageComponent)
            } else {
                stack.push(lockPageComponent)
            }
        }
    }

    // Any time the session locks - manual lock, auto-lock timeout, whatever -
    // always fall back to the lock screen, no matter what's on the stack.
    Connections {
        target: session
        function onLocked() {
            stack.clear()
            stack.push(lockPageComponent)
        }
    }

    Component {
        id: setupPageComponent
        SetupPage {
            onAccountReady: function (name) {
                stack.push(welcomePageComponent, { "userName": name })
            }
        }
    }

    Component {
        id: lockPageComponent
        LockPage {
            onUnlocked: function (name) {
                stack.push(welcomePageComponent, { "userName": name })
            }
        }
    }

    Component {
        id: welcomePageComponent
        WelcomePage {
            onContinueRequested: {
                stack.clear()
                stack.push(homePageComponent)
            }
        }
    }

    Component {
        id: homePageComponent
        HomePage {}
    }
}
