#ifndef BID_H
#define BID_H

#include "enumerations/BidCall.h"
#include "enumerations/CardSuit.h"
#include "enumerations/PlayerPosition.h"
#include <QJsonObject>
#include <QtGlobal>
#include <optional>

class Bid
{
public:
    Bid();
    Bid(PlayerPosition bidder, BidCall call);
    Bid(PlayerPosition bidder, CardSuit trumpSuit, qint8 tricksAbove);
    PlayerPosition getBidder();
    BidCall getCall();
    CardSuit getTrumpSuit();
    qint8 getTricksAbove();
    void read(const QJsonObject &json);
    void write(const QJsonObject &json);
    QString toString();
private:
    PlayerPosition bidder;
    BidCall call;
    CardSuit trumpSuit;
    qint8 tricksAbove;
};

#endif // BID_H