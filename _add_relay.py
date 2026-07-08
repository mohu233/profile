import os, re

fp = r"E:\Documents\untitled2\mainwindow.cpp"
with open(fp, "rb") as f:
    data = f.read()

# The block to insert relay logic BEFORE the QStringList arguments line
# Find: QStringList arguments;
# Insert relay startup code right before it

old = b'    QStringList arguments;\n'

# The new relay block to insert
relay_block = b'''    // ========== Proxy relay startup (auth support) ==========
    QString effectiveProxyServer = config.proxyServer;
    QProcess *relayProcess = nullptr;
    if (!config.proxyUsername.isEmpty() && !config.proxyPassword.isEmpty()
        && !config.proxyHost.isEmpty() && config.proxyPort > 0) {

        // 1. Pick a free local port
        quint16 relayPort = 0;
        QTcpServer portPicker;
        if (portPicker.listen(QHostAddress::LocalHost, 0)) {
            relayPort = portPicker.serverPort();
            portPicker.close();
        }
        if (relayPort == 0) relayPort = 50000; // fallback

        // 2. Find Python
        QString python = QStandardPaths::findExecutable("python");
        if (python.isEmpty()) {
            python = QStandardPaths::findExecutable("python3");
        }

        // 3. Find proxy_relay.py
        QString relayScript = QCoreApplication::applicationDirPath() + "/proxy_relay.py";
        if (!QFile::exists(relayScript)) {
            relayScript = QCoreApplication::applicationDirPath() + "/../../proxy_relay.py";
        }

        if (!python.isEmpty() && QFile::exists(relayScript)) {
            relayProcess = new QProcess(this);
            QStringList relayArgs;
            relayArgs << relayScript
                      << QString::number(relayPort)
                      << config.proxyHost
                      << QString::number(config.proxyPort)
                      << config.proxyUsername
                      << config.proxyPassword;

            relayProcess->start(python, relayArgs);
            if (relayProcess->waitForStarted(3000)) {
                // Use local relay as Chrome proxy target
                effectiveProxyServer = QString("http://127.0.0.1:%1").arg(relayPort);
                writeLog("System", QString("Proxy relay started on 127.0.0.1:%1 -> %2:%3")
                         .arg(relayPort).arg(config.proxyHost).arg(config.proxyPort));
            } else {
                writeLog("System", QString("Proxy relay failed to start: %1").arg(relayProcess->errorString()));
                relayProcess->deleteLater();
                relayProcess = nullptr;
            }
        } else {
            writeLog("System", "Cannot start proxy relay: Python or proxy_relay.py not found");
        }
    }

    QStringList arguments;\n'''

data = data.replace(old, relay_block)

# Now replace config.proxyServer usage in args with effectiveProxyServer
data = data.replace(
    b'arguments << QString("--proxy-server=%1").arg(config.proxyServer);',
    b'arguments << QString("--proxy-server=%1").arg(effectiveProxyServer);'
)

# Add relay cleanup in the chrome finished callback
old_finish = b'        writeLog("System", QString("...: %1, ...: %2 (Stopped)").arg(envName).arg(deadPort));\n        chromeProcess->deleteLater();'
new_finish = b'        writeLog("System", QString("Stopped: %1, port: %2").arg(envName).arg(deadPort));\n        if (relayProcess && relayProcess->state() == QProcess::Running) {\n            relayProcess->terminate();\n            if (!relayProcess->waitForFinished(3000)) {\n                relayProcess->kill();\n            }\n            relayProcess->deleteLater();\n        }\n        chromeProcess->deleteLater();'

if old_finish in data:
    data = data.replace(old_finish, new_finish)
else:
    print("Warning: relay cleanup pattern not found, checking alt patterns...")
    # Try alternative pattern
    alt = b'chromeProcess->deleteLater();\n        updateEnvCardStatus(envName, false);'
    if alt in data:
        new_alt = b'if (relayProcess && relayProcess->state() == QProcess::Running) {\n            relayProcess->terminate();\n            if (!relayProcess->waitForFinished(3000)) {\n                relayProcess->kill();\n            }\n            relayProcess->deleteLater();\n        }\n        chromeProcess->deleteLater();\n        updateEnvCardStatus(envName, false);'
        data = data.replace(alt, new_alt)
        print("Relay cleanup added via alt pattern")

with open(fp, "wb") as f:
    f.write(data)
print("Relay logic injected")
