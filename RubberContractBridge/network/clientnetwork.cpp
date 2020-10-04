#include "clientnetwork.h"

ClientNetwork::ClientNetwork(QObject *parent) : QObject(parent)
{
    // Init internal variables
    gameStarted = false;
    playerName = "";
    keepAlive = nullptr;
    tcpSoc = nullptr;
    tempPlayerName = "";
    tempPassword = "";
    bLoggedIn = false;
    idCounter = 0;
    prevID = -1; // First ID received from server is 0. prevID should be smaller, thus -1.

    // Init unit test
    bUnitTest.clear();
    bUnitTest.fill(false,40);

    //socketError can use 0 - 4
    //txRequestLogin can use 5 - 9
    //socketConnected can use 10 - 19
    //rxAll can use 20 - 29

    // Prepare the tcpSoc and datastream
    tcpSoc = new QTcpSocket(this);

    in.setDevice(tcpSoc);
    in.setVersion(QDataStream::Qt_5_10);

    // Create connections
    // All reception
    connect(tcpSoc, &QIODevice::readyRead, this, &ClientNetwork::rxAll);
    // Connection errors
    connect(tcpSoc, &QAbstractSocket::errorOccurred, this, &ClientNetwork::socketError);
    // Sucsessfully connected
    connect(tcpSoc, &QAbstractSocket::connected, this, &ClientNetwork::socketConnected);
}

ClientNetwork::~ClientNetwork()
{
    tcpSoc->abort();
}

QVector<bool> ClientNetwork::getUnitTest() const
{
    return bUnitTest;
}

void ClientNetwork::txRequestLogin(QHostAddress serverIP, qint16 port, QString playerName, QString password)
{
    // Connect to the server.
    // Cannot Request a new login while connected to the host?
    if (tcpSoc->state() != QAbstractSocket::UnconnectedState){
        emit connectionResult(3, "");
        return;
    }

    // Store playerName and password for use after port connected.
    tempPlayerName = playerName;
    tempPassword = password;

    // If connection has already been made, abort it.
    tcpSoc->abort();
    tcpSoc->connectToHost(serverIP, port);

    // socketConnected or socketError will be signaled next, depending on connectToHost()'s outcome.

    qInfo() << "Attempt to connect to the host with server IP: " << serverIP.toString() << " and port: " << port;

}

void ClientNetwork::txBidSelected(Bid bid)
{

}

void ClientNetwork::txMoveSelected(Card card)
{

}

void ClientNetwork::txMessage(QString msg)
{

}

/*!
 * Read all data and call relevant function to prcess it.
 * Data is avaliable on the in stream (from the server).
 * Read data, validate QJsonObject.
 * Choose next function to call
 */

void ClientNetwork::rxAll()
{
    in.startTransaction();

    QJsonObject rxObj;
    in >> rxObj;

    if (!in.commitTransaction()){
        emit generalError("Datastream read error occured. It is suggested to restart the game.");
        qWarning() << "Datastream error occured.";
        return;
    }

    // Validate the QJsonObject
    // It should contain a Type field, with valid information in the string part.
    if (rxObj.contains("Type") && rxObj["Type"].isString() && rxObj.contains("ID") && rxObj["ID"].isDouble()){
        // QJsonObject received contained the expected data.
        // Test if ID number is larger than prevID
        if (rxObj["ID"].toInt() <= prevID) {
            emit generalError("Outdated data was received. The data will be ignored.");
            return;
        }

    } else {
        // QJsonObject received had errors
        emit generalError("Data received from server has been incorrectly formatted. It is suggested to restart the game.");
        qWarning() << "Data received from server has been incorrectly formatted.";

        // TODO: Handel case where data received was incorrect.
        // If not logged in, dissconnect and signal GUI

        // If logged in, request a resend of the data? Or ignaor it, or stop the client.
        // Maybe have a last message detail on server side?

        return;
    }

    // The data received is valid.
    // Choose the function that will handel the data.
    QString tempStr = rxObj["Type"].toString();
    if (tempStr == "NOTIFY_BID_TURN") {
        rxNotifyBidTurn();
        return;
    }
    if (tempStr == "NOTIFY_MOVE_TRUN") {
        rxNotifyMoveTurn();
        return;
    }
    if (tempStr == "NOTIFY_BID_REJECTED") {
        rxNotifyBidRejected(rxObj);
        return;
    }
    if (tempStr == "NOTIFY_MOVE_REJECTED") {
        rxNotifyMoveRejected(rxObj);
        return;
    }
    if (tempStr == "LOGIN_RESULT") {
        rxLoginResult(rxObj);
        return;
    }
    if (tempStr == "UPDATE_GAME_STATE") {
        rxUpdateGameState(rxObj);
        return;
    }
    if (tempStr == "MESSAGE") {
        rxMessage(rxObj);
        return;
    }
    if (tempStr == "GAME_TERMINATED") {
        rxGameTerminated(rxObj);
        return;
    }
    if (tempStr == "PING_RECEIVED") {
        rxPingReceived();
        return;
    }

    // Default
    emit generalError("An incorrect 'Type' has been received. The data will be ignored.");
    return;


}

void ClientNetwork::internalServerDisconnected()
{

}

/*!
 * \brief ClientNetwork::socketConnected Executed when sucesfully connected to the host (server).
 *
 * Request to see if playerName and password is correct.
 * Create QJsonObject containting the playerName and password.
 * Send the QJsonObject to the server.
 * Make signal and slot conections.
 */

void ClientNetwork::socketConnected()
{
    // Connection was sucessfull
    emit connectionResult(0, "");

    // The first packet received by the server must have an ID of 0.
    idCounter = 0;
    // First packer sent from the client should have an ID of 0 and must be larger than prevID.
    prevID = -1;

    // Create QJsonObject
    QJsonObject txObj;
    txObj["Type"] = "LOGIN_REQUEST";
    txObj["Password"] = tempPassword;
    txObj["Alias"] = tempPlayerName;

    // Send the login request to the server
    txAll(txObj);
}

/*!
 * \brief ClientNetwork::socketError Signal the client GUI if an connection error with the server occurs. Abort connection.
 * \param socError QAbstractSocket::SocketError
 */

void ClientNetwork::socketError(QAbstractSocket::SocketError socError)
{
    /*
     * TODO:
     * From Qt documentation (when trying to change the port):
     * When this signal is emitted, the socket may not be ready for a reconnect attempt.
     * In that case, attempts to reconnect should be done from the event loop.
     * For example, use a QTimer::singleShot() with 0 as the timeout.
     */

    // Determine the error
    switch (socError) {
    case QAbstractSocket::HostNotFoundError:
        emit connectionResult(1, "The host was not found. Please check the host IP address and port settings.");
        break;

    case QAbstractSocket::ConnectionRefusedError:
        emit connectionResult(1, "The connection was refused by the server. "
                            "Make sure the server is running, "
                            "and check that the host IP address and port settings are correct.");
        break;

    default:
        emit connectionResult(1, "The following error occurred: " + tcpSoc->errorString());
    }

    // TODO: disconnect try to connect again?
    tcpSoc->abort();
//    tcpSoc->close();
}

/*!
 * \brief ClientNetwork::txAll Handel all data that is sent to the server.
 * \param data QJsonObject containing at least the "Type" and "ID" fields.
 */

void ClientNetwork::txAll(QJsonObject data)
{
    // Cannot send data if not connected to the server.
    if (tcpSoc->isValid()){
        emit generalError("Not connected to the server. Cannot send data to the server.");
        return;
    }

    // Test to see if data that must be sent is valid
    if (!data.contains("Type")){
        emit generalError("Data to be sent does not contain 'Type' field. Data was not sent.");
        return;
    }

    if (data.contains("ID")) {
        emit generalError("Data to be sent should not contain an 'ID' field. 'ID' field was removed.");
        data.remove("ID");
    }

    // Add the ID field to the QJsonObject
    data["ID"] = idCounter++;

    // Create transmitting communication data stream
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    // Send the login request to the server
    out << data;
    int tempVal = tcpSoc->write(block);

    qInfo() << "Number of bytes expected to be sent to the server: " << block.size();
    qInfo() << "Number of bytes sent to server: " << tempVal;

    if (tempVal == -1) {
        // An error occured when writing the data block
        emit generalError("An error occured with sending data to the server. It is suggested to restart the game.");
    } else if (tempVal < block.size()) {
        // The block written was too small (did not contain enough bytes).
        emit generalError("An error occured with sending data to the server. It is suggested to restart the game.");
    }
}

/*!
 * \brief ClientNetwork::rxNotifyBidTurn
 */

void ClientNetwork::rxNotifyBidTurn()
{

}

/*!
 * \brief ClientNetwork::rxNotifyMoveTurn
 */

void ClientNetwork::rxNotifyMoveTurn()
{

}

/*!
 * \brief ClientNetwork::rxNotifyBidRejected
 * \param reasonObj QJsonObject with "Type" = "NOTIFY_BID_REJECTED".
 */

void ClientNetwork::rxNotifyBidRejected(QJsonObject reasonObj)
{

}

/*!
 * \brief ClientNetwork::rxNotifyMoveRejected
 * \param reasonObj QJsonObject with "Type" = "NOTIFY_MOVE_REJECTED".
 */

void ClientNetwork::rxNotifyMoveRejected(QJsonObject reasonObj)
{

}

// TODO: handel connection login result
/*!
 * \brief ClientNetwork::rxLoginResult Handel the login result (accepted or rejected).
 * \param resObj QJsonObject with "Type" = "LOGIN_RESULT".
 */

void ClientNetwork::rxLoginResult(QJsonObject resObj)
{
    // The client should not already be logged in.
    if (bLoggedIn){
        emit generalError("The client is already logged in, but a login result message has been received. The message will be ignored.");
        return;
    }

    // The object validation has been done.
    if (!resObj.contains("loginSuccessful") || !resObj.contains("reason")) {
        emit generalError("Data received from server has been incorrectly formatted. It is suggested to restart the game.");
        return;
    }

    bLoggedIn = resObj["loginSuccessful"].toBool();

    // Notify client GUI of login result.
    emit loginResult(resObj["loginSuccessful"].toBool(), resObj["reason"].toString());

    // If login was unsuccessful, disconnect from the host.
    if (!bLoggedIn){
        tcpSoc->abort();
    }
}

/*!
 * \brief ClientNetwork::rxUpdateGameState
 * \param gsObj QJsonObject with "Type" = "UPDATE_GAME_STATE".
 */

void ClientNetwork::rxUpdateGameState(QJsonObject gsObj)
{

}

/*!
 * \brief ClientNetwork::rxMessage
 * \param msgObj QJsonObject with "Type" = "MESSAGE".
 */

void ClientNetwork::rxMessage(QJsonObject msgObj)
{

}

/*!
 * \brief ClientNetwork::rxGameTerminated
 * \param reasonObj QJsonObject with "Type" = "GAME_TERMINATED".
 */

void ClientNetwork::rxGameTerminated(QJsonObject reasonObj)
{

}

/*!
 * \brief ClientNetwork::rxPingReceived
 */

void ClientNetwork::rxPingReceived()
{

}
