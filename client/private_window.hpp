#ifndef PRIVATE_WINDOW_HPP
#define PRIVATE_WINDOW_HPP

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

#include "client.hpp"

typedef std::vector<std::string> group_dest_names_vector;


class private_window : public QWidget
{
    Q_OBJECT

public:

    explicit private_window( QSharedPointer<Client> client, group_dest_names_vector dest_names, QWidget *parent = 0 );
    ~private_window();

private:
    QString username;

    QString stylesheet;
    QString mainwindow_stylesheet;

    QSharedPointer<Client> client;

    group_dest_names_vector dest_names;

private:

    QLabel       * user_name_QLabel;
    QTextEdit    * main_private_TextEdit;
    QTextEdit    * send_message_private_TextEdit;

    QTableWidget * emoji_TableWidget;

    QPushButton * send_private_message_PushButton;
    QPushButton * show_emojis_PushButton;

    QSpacerItem * spacer;
    QSpacerItem * spacer1;
    QSpacerItem * spacer2;

    QHBoxLayout * mainLayout;
    QVBoxLayout * textboxes_layout;
    QHBoxLayout * textbox_bottom_layout;

signals:
    void close_all_windows();

public slots:

    void on_emoji_item_clicked( QTableWidgetItem * item );
    void on_show_emojis();

    void on_send_private_message();
    void on_private_message_received( QString sender, QString message );

    void on_offline_user( QString gone_offline_user );

    void on_close_event( QCloseEvent * ) { this->hide(); };

};

#endif // PRIVATE_WINDOW_HPP
