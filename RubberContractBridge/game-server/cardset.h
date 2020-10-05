#ifndef CARDSET_H
#define CARDSET_H

#include "card.h"

class CardSet
{
public:
    CardSet();
    void addCard(Card card);
    Card removeCard(qint8 position);
    Card removeTopCard();
    Card getCard(qint8 position);
    bool containsCard(const Card &card);
    bool containsSuit(CardSuit suit);
    qint8 getCardCount();
    void orderHand();
    void clearSet();
    void shuffle();
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
private:
    QVector<Card> cards;
};

#endif // CARDSET_H
