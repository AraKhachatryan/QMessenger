#ifndef MESSENGER_HPP
#define MESSENGER_HPP

#include <QMainWindow>
#include <QApplication>
#include <QWidget>
#include <QTableWidget>
#include <QDebug>
#include <QSharedPointer>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QHeaderView>
#include <QDirIterator>
#include <QFile>
//#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QVector>

#include "connect_login_register.hpp"
#include "private_window.hpp"
#include "client.hpp"


typedef std::vector<std::string> dest_names_vector;
typedef QSharedPointer<private_window> private_room;
typedef QMap<dest_names_vector, private_room> private_rooms_QMap;


class messenger : public QMainWindow
{
    Q_OBJECT

public:

    explicit messenger(QWidget *parent = 0);
    ~messenger();

private:
    QString username;
    QString password;
    QString server_ip_address;
    QString server_port;

    QString stylesheet;
    QString mainwindow_stylesheet;

    QSharedPointer<connect_to_server> connect_Widget;
    QSharedPointer<login_register> login_register_Widget;
    QSharedPointer<Client> client;

    private_rooms_QMap private_rooms;

private:

    QWidget      * messenger_QWidget;
    QTextEdit    * main_global_TextEdit;
    QTextEdit    * send_message_global_TextEdit;
    QTableWidget * emoji_TableWidget;

    QLabel       * online_users_QLabel;
    QLabel       * user_name_QLabel;
    QListWidget  * online_users_ListWidget;

    QPushButton * send_global_message_PushButton;
    QPushButton * select_users_PushButton;
    QPushButton * open_private_room_PushButton;
    QPushButton * logout_PushButton;

    QPushButton * show_emojis_PushButton;

    QSpacerItem * spacer;
    QSpacerItem * spacer1;
    QSpacerItem * spacer2;
    QSpacerItem * spacer3;
    QSpacerItem * spacer4;

    QHBoxLayout * mainLayout;
    QHBoxLayout * textbox_bottom_layout;
    QHBoxLayout * logout_layout;
    QVBoxLayout * textboxes_layout;
    QVBoxLayout * right_panel_layout;

signals:
    void close_all_windows();

public slots:
    void on_connect( QString server_ip_address, QString server_port );
    void on_sign_in( QString username, QString password );
    void on_sign_up( QString full_name, QString username, QString password );
    void on_logout();

    void on_emoji_item_clicked( QTableWidgetItem * item );
    void on_show_emojis();

    void on_send_global_message();
    void on_global_message_received( QString sender, QString message );

    void on_private_message_received( QString sender, QString message );
    void on_select_users();
    void on_open_private_room();

    void on_online_user( QString new_online_user );
    void on_offline_user( QString gone_offline_user );

    void on_close_event( QCloseEvent * ) { emit close_all_windows(); };

};

#endif // MESSENGER_HPP
