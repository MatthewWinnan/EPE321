#include "servernetwork.h"

ServerNetwork::ServerNetwork(QObject *parent, QString nameOfAI) : QObject(parent)
{
    // Init all variables
    this->nameOfAI = nameOfAI;
    tcpServer = nullptr;

    // Set a default password, this should be changed by the server.
    password = "12340000";
    playerNames.clear();
    clientSoc.clear();
    clientSocTemp.clear();
    tcpServer = nullptr;

    // Init unit test
    bUnitTest.clear();
    bUnitTest.fill(false,40);

    //getPlayerSoc can use 0 - 9
    // <open> can use 10 - 19
    //connectClient can use 20 - 29
    //validateClient can use 30 - 39
}

ServerNetwork::~ServerNetwork()
{
    qInfo() << "ServerNetwork destructed";
    // Stop the server
    if (tcpServer != nullptr) {
        disconnect(tcpServer, &QTcpServer::newConnection, 0, 0);
        tcpServer->close();
    }
}

QTcpSocket *ServerNetwork::getPlayerSoc(QString playerName) const
{
    // Return the socket that corresponds to the playerName.
    // Throw an error if playerName can not be found and return nullptr.
    return nullptr;
}

void ServerNetwork::setPassword(QString password)
{
    // Password strength and validness should be checked in the serverGUI.
    this->password = password;
}

void ServerNetwork::initServer(QHostAddress ip, quint16 port)
{
    // Open a port on the given ip address.
    // Use port 61074.
    // Start listening for clients that want to connect.
    // The IP address can only be set once after the program has started.

    // Prepare bUnitTest
    for (int i = 10; i <= 19; i++){
        bUnitTest[i] = false;
    }

    // Check if the server has been initialized.
    if (tcpServer != nullptr){
        emit connectionResult(3, ip, port, "");
        return;

    }

    // Test if ip address is valid.

    // Find all valid ipAdresses.
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

    if ((ip.toString().isEmpty() == true) || (ipAddressesList.contains(ip) == false)) {
        emit connectionResult(1, ip, port, "");
        return;
    }

    // Disconnect the client
    //    if (clientConnection != nullptr){
    //        disconnect(clientConnection, &QIODevice::readyRead,0,0);
    //        clientConnection->disconnectFromHost();
    //        clientConnection = nullptr;
    //    }

    //    disconnect(tcpServer, &QTcpServer::newConnection, 0, 0);


    tcpServer = new QTcpServer(this);

    if (!tcpServer->listen(ip, port)) {
        emit connectionResult(2, ip, port, tcpServer->errorString());

        // Set tcpServer to null, so that a new QTcpServer can be constructed on the second try.
        tcpServer->deleteLater();
        tcpServer = nullptr;

        return;
    }

    // The connection was successfull.
    connect(tcpServer, &QTcpServer::newConnection, this, &ServerNetwork::connectClient);

    qInfo() << "The server is listening for clients on " << ip.toString() << " with port: " << tcpServer->serverPort();
    emit connectionResult(0,ip,port,"");

}


void ServerNetwork::stopListening()
{
    // Stop listen for new client connections.

}

QVector<bool> ServerNetwork::getUnitTest() const
{
    return bUnitTest;
}

void ServerNetwork::connectClient()
{
    // Accept new client connections.
    // Add client to clientSocTemp.
    // Make signal slot connections.

    QTcpSocket* clientConnection = tcpServer->nextPendingConnection();


    if (!clientConnection) {
        emit generalError("Got invalid pending connection! Ignoring the connection.");
        return;
    }

    connect(clientConnection, &QIODevice::readyRead,this, &ServerNetwork::validateClient);
    connect(clientConnection, &QAbstractSocket::disconnected,this, &ServerNetwork::disconnectClient);
    connect(clientConnection, &QAbstractSocket::disconnected,clientConnection, &QObject::deleteLater);

    // Add to temp list.
    clientSocTemp.append(clientConnection);

    qInfo() << "client added to clientSocTemp";
}

/*!
 * Validate the password and username.
 * If valid, add username and client socket.
 * Signal the GUI that a client has logged in.
 * If invalid, disconnect the client.
 */

void ServerNetwork::validateClient()
{
    qInfo() << "Incomming connection.";
    // If avlidateRes is empty at the end of this function,
    // no errors occured and the login request has been accepted.
    QString validateRes = "";

    // Get the sender's QTcpSocket
    QObject* obj = sender();
    QTcpSocket* tempSocket = qobject_cast<QTcpSocket*>(obj);

    // Remove clientSock from the clientSocTemp.
    int numRemoved = clientSocTemp.removeAll(tempSocket);

    // Check if tempSocket is the same as previously saved.
    if (numRemoved == 0){
        emit generalError("An unexpected client connected. Client forcefully removed.");
        tempSocket->abort();
        return;
    }

    // Check for unexpected sockets
    if (numRemoved > 1) {
        emit generalError("Duplicate clients existed and was removed. The error will be ignored.");
    }

    qInfo() << "Sender object safely converted.";

    // Read data from the socket
    in.setDevice(tempSocket);
    in.setVersion(QDataStream::Qt_5_10);

    in.startTransaction();

    // Get the data from the
    QJsonObject rxObj;
    in >> rxObj;

    if (!in.commitTransaction()){
        // Read error occured: this should not happen.
        // If this occurs, the connection to the client must be terminated.
        emit generalError("Datastream read error occured. Client forcefully removed.");
        qWarning() << "Datastream error occured.";
        tempSocket->abort();
        return;
    }

    // Validate the QJsonObject
    // ID should be 0
    // It should contain a Type field, with valid information in the string part.
    if (rxObj.contains("Type") && rxObj["Type"].isString() && rxObj.contains("ID") && rxObj["ID"].isDouble()){
        // QJsonObject received contained the expected data.
        // Test if ID number is larger than prevID
        if (rxObj["ID"].toInt() <= -1) {
            emit generalError("Outdated data was received. The data will be ignored. Client forcefully removed.");
//            validateRes.append("The login request was too old and thus not accepted.");
            tempSocket->abort();
            return;
        }

        if (rxObj["Type"].toString() != "LOGIN_REQUEST" || !rxObj.contains("Alias") || !rxObj.contains("Password")){
             emit generalError("The expected data has not been received. Client forcefully removed.");
            tempSocket->abort();
            return;
        }
    } else {
        // QJsonObject received had errors
        emit generalError("Data received from server has been incorrectly formatted. Client forcefully removed.");
        qWarning() << "Data received from client has been incorrectly formatted.";
        tempSocket->abort();
        return;
    }

    // Read the data. (Password and username.)
    validateRes.append(validateLogin(rxObj["Alias"].toString(), rxObj["Password"].toString()));

    // Create QJsonObject
    QJsonObject txObj;
    txObj["Type"] = "LOGIN_RESULT";
    txObj["ID"] = 0;

    if (validateRes.isEmpty()){
        // The login has been approved.
        // Add client to logged-in pool and notify the client that the login was accepted.
        clientSoc.append(tempSocket);
        playerNames.append(rxObj["Alias"].toString());

        // Notify the server GUI that a player successfully logged in.
        emit playerJoined(playerNames.last());

        // Notify the client of the successfull login.
        txObj["loginSuccessful"] = true;
        txObj["reason"] = "";

    } else {
        // The login was not accpeted.
        // Notify the client with a reason and disconnect the client.
        txObj["loginSuccessful"] = false;
        txObj["reason"] = validateRes;
    }

    // Send data to clients
    // Create transmitting communication data stream
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    // Send the login request to the server
    out << txObj;
    int tempVal = clientSoc.last()->write(block);

    qInfo() << "Number of bytes expected to be sent to the client: " << block.size();
    qInfo() << "Number of bytes sent to client: " << tempVal;

    if (tempVal == -1) {
        // An error occured when writing the data block
        emit generalError("An error occured with sending data to the client. It is suggested to restart the game.");
    } else if (tempVal < block.size()) {
        // The block written was too small (did not contain enough bytes).
        emit generalError("An error occured with sending data to the client. It is suggested to restart the game.");
    }

    // After transmitting the data, disconnect the client if the login was not succsessfull.
    if (!validateRes.isEmpty()){
        tempSocket->disconnectFromHost();
    }
}

void ServerNetwork::disconnectClient()
{
    // When a client disconnects, check if the client logged in.
    // If logged in, remove username and client socket.
    // Signal the GUI to also remove the client.

    qInfo() << "Socket disconnected";

    // Get the sender's QTcpSocket
    QObject* obj = sender();
    QTcpSocket* tempSocket = qobject_cast<QTcpSocket*>(obj);

    if (clientSocTemp.contains(tempSocket)) {
        // The client was not logged in
        // Remove from clientSocTemp
        clientSocTemp.removeAll(tempSocket);
    }

    if (clientSoc.contains(tempSocket)){
        // The client was logged in
        // Remove from clientSoc and playerNames
        // Inform the serverGUI that the player disconnected.
        // TODO: This signal emit can cause problems if all emitted when disconnecting, so check this out.
        // Add description in header files.
        QString tempPlayerName = playerNames.at(clientSoc.indexOf(tempSocket));
        playerNames.removeAll(tempPlayerName);
        clientSoc.removeAll(tempSocket);

        emit playerDisconnected(tempPlayerName);
    }

}

/*!
 * Validate the password and playerName.
 * If both are valid, return an empty string.
 * Else return the reason for faliure.
 * This message will be displayed to the user.
 * Check if password match.
 * Check if username is empty.
 * Check if username is longer than 15 chars.
 * Check if username correspons to an AI player.
 * Check if the username has been used before.
 * \param playerName QString
 * \param password QString
 * \return Error msg. If no error occured, an empty string will be returned.
 */

QString ServerNetwork::validateLogin(QString playerName, QString password)
{

    if (this->password != password){
        return "The password is incorrect.";
    }

    if (playerName.isEmpty()) {
        return "The player name must contain at least one character.";
    }

    if (playerName.length() > 15) {
        return "The player name may not be longer than 15 chars.";
    }

    if (playerName == nameOfAI) {
        return "The palyer may not be given the same name as an AI. AI's name is: " + nameOfAI;
    }

    if (playerNames.contains(playerName)){
        return "The palyer's name has already been used, please choose another username.";
    }

    // General
    return "";

}
