#include "playernetwork.h"

PlayerNetwork::PlayerNetwork(QObject *parent, QString playerName, QTcpSocket *clientSoc) : QObject(parent)
{
    // Once ingerited, init the playerName.
    PlayerNetwork::clientSoc = clientSoc;

}

PlayerNetwork::~PlayerNetwork()
{

}

void PlayerNetwork::notifyBidTurn()
{

}

void PlayerNetwork::notifyMoveTurn()
{

}

void PlayerNetwork::updateGameState(PlayerNetwork::PlayerGameState gameState)
{

}

void PlayerNetwork::notifyBidRejected(QString reason)
{

}

void PlayerNetwork::notifyMoveRejected(QString reason)
{

}

void PlayerNetwork::message(QString source, QString msg)
{

}

void PlayerNetwork::gameTerminated(QString reason)
{

}

QString PlayerNetwork::getPlayerName() const
{

}

PlayerNetwork::PlayerPosition PlayerNetwork::getPosition() const
{

}

PlayerNetwork::Team PlayerNetwork::getTeam() const
{

}

void PlayerNetwork::setPosition(PlayerNetwork::PlayerPosition position)
{

}

QString PlayerNetwork::toString() const
{

}

void PlayerNetwork::rxAll()
{

}

void PlayerNetwork::pingClient()
{

}

void PlayerNetwork::txAll(QJsonObject data)
{

}

void PlayerNetwork::rxBidSelected(QJsonObject bidObj)
{

}

void PlayerNetwork::rxMoveSelected(QJsonObject moveObj)
{

}

void PlayerNetwork::rxMessage(QJsonObject msgObj)
{

}

void PlayerNetwork::rxPingReturn()
{

}

void PlayerNetwork::internalClientDisconnected()
{

}