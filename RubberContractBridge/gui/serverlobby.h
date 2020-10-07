#ifndef SERVERLOBBY_H
#define SERVERLOBBY_H


#include <QWidget>
#include <QObject>
#include <QtWidgets>

namespace Ui {
class serverLobby;
}

class serverLobby : public QWidget
{
    Q_OBJECT

public:
    explicit serverLobby(QWidget *parent = nullptr);
    ~serverLobby();
    void windowSetup();
    void lobbyStart();
    void changeIcon();

public slots:
    void removePlayer (QString user);
    void addPlayer (QString user);

private slots:
    void on_south_clicked();

    void on_west_clicked();

    void on_east_clicked();

    void on_north_clicked();

    void on_addPlayer1_stateChanged(int arg1);

    void on_addPlayer2_stateChanged(int arg1);

    void on_addPlayer3_stateChanged(int arg1);

    void on_addPlayer4_stateChanged(int arg1);

    void on_addPlayer5_stateChanged(int arg1);

private:
    Ui::serverLobby *ui;
};

#endif // SERVERLOBBY_H
