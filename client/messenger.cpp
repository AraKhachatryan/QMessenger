#include "messenger.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>


messenger::messenger(QWidget *parent) : QMainWindow(parent)
{
    messenger_QWidget = new QWidget;
    messenger_QWidget->setFixedSize(730,685);
    messenger_QWidget->setWindowTitle("QMessenger");   // TODO set username here

    mainLayout = new QHBoxLayout;
    textbox_bottom_layout = new QHBoxLayout;
    logout_layout = new QHBoxLayout;
    textboxes_layout = new QVBoxLayout;
    right_panel_layout = new QVBoxLayout;


    main_global_TextEdit = new QTextEdit;
    main_global_TextEdit->setFontPointSize(12);
    send_message_global_TextEdit = new QTextEdit;
    send_message_global_TextEdit->setFontPointSize(14);

    online_users_ListWidget = new QListWidget;
    online_users_ListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    show_emojis_PushButton = new  QPushButton;
    emoji_TableWidget = new QTableWidget;
    emoji_TableWidget->setWindowTitle("Emojies");


    send_global_message_PushButton = new QPushButton("Send message");
    select_users_PushButton = new QPushButton("Select users");
    open_private_room_PushButton = new QPushButton("Open private room");
    logout_PushButton = new QPushButton("Logout");

    user_name_QLabel = new QLabel();
    QFont font_user_name( "Arial", 16, QFont::Bold);
    user_name_QLabel->setFont(font_user_name);
    user_name_QLabel->setTextFormat(Qt::RichText);
    user_name_QLabel->setAlignment(Qt::AlignCenter);

    online_users_QLabel = new QLabel("Online Users");
    online_users_QLabel->setProperty("cssClass", QVariant::fromValue<QStringList>( QStringList() << "true" ) );
    online_users_QLabel->setAlignment(Qt::AlignCenter);

    spacer = new QSpacerItem(500, 5, QSizePolicy::Fixed);
    spacer1 = new QSpacerItem(295, 30, QSizePolicy::Fixed);
    spacer2 = new QSpacerItem(500, 7, QSizePolicy::Fixed);
    spacer3 = new QSpacerItem(200, 7, QSizePolicy::Fixed);
    spacer4 = new QSpacerItem(200, 10, QSizePolicy::Fixed);

    show_emojis_PushButton->setIcon(QIcon(QPixmap("icons/emoji_icon.png")));
    show_emojis_PushButton->setIconSize(QSize(30,30));


    textbox_bottom_layout->addWidget(show_emojis_PushButton);
    textbox_bottom_layout->addItem(spacer1);
    textbox_bottom_layout->addWidget(send_global_message_PushButton);


    logout_layout->addWidget(logout_PushButton);

    textboxes_layout->addWidget(main_global_TextEdit);
    textboxes_layout->addItem(spacer2);
    textboxes_layout->addWidget(send_message_global_TextEdit);
    textboxes_layout->addItem(spacer);
    textboxes_layout->addLayout(textbox_bottom_layout);

    right_panel_layout->addWidget(user_name_QLabel);
    right_panel_layout->addWidget(online_users_QLabel);
    right_panel_layout->addWidget(online_users_ListWidget);
    right_panel_layout->addItem(spacer4);
    right_panel_layout->addWidget(select_users_PushButton);
    right_panel_layout->addWidget(open_private_room_PushButton);
    right_panel_layout->addItem(spacer3);
    right_panel_layout->addLayout(logout_layout);

    main_global_TextEdit->setFixedSize(500,500);
    send_message_global_TextEdit->setFixedSize(500,100);

    user_name_QLabel->setFixedSize(200,30);
    online_users_QLabel->setFixedSize(200,30);
    online_users_ListWidget->setFixedSize(200,400);


    send_global_message_PushButton->setFixedSize(140,30);
    logout_PushButton->setFixedSize(100,30);

    show_emojis_PushButton->setFixedSize(40,40);

    mainLayout->addLayout(textboxes_layout);
    mainLayout->addLayout(right_panel_layout);

    main_global_TextEdit->setReadOnly(true);

    QFile stylesheet_file("stylesheets/stylesheet.qss");
    stylesheet_file.open(QFile::ReadOnly);
    stylesheet = QLatin1String(stylesheet_file.readAll());

    QFile mainwindow_stylesheet_file("stylesheets/mainwindow_stylesheet.qss");
    mainwindow_stylesheet_file.open(QFile::ReadOnly);
    mainwindow_stylesheet = QLatin1String(mainwindow_stylesheet_file.readAll());

    show_emojis_PushButton->setStyleSheet("border-radius: 18px ;");

    messenger_QWidget->setStyleSheet(mainwindow_stylesheet);
    messenger_QWidget->setLayout(mainLayout);
    //messenger_QWidget->show(); // for debug, after login or registration must be shown

    // connect slots & signals
    QObject::connect(logout_PushButton, &QPushButton::clicked, this, &messenger::on_logout);

    QObject::connect(show_emojis_PushButton, &QPushButton::clicked, this, &messenger::on_show_emojis);
    QObject::connect(emoji_TableWidget, &QTableWidget::itemClicked, this, &messenger::on_emoji_item_clicked);

    connect_Widget = QSharedPointer<connect_to_server>::create();
    connect_Widget->setStyleSheet(stylesheet);
    connect_Widget->show();
    QObject::connect(connect_Widget.data(), &connect_to_server::connect_signal, this, &messenger::on_connect); // data(): extracts the raw pointer

    QObject::connect(send_global_message_PushButton, &QPushButton::clicked, this, &messenger::on_send_global_message);

    QObject::connect(select_users_PushButton, &QPushButton::clicked, this, &messenger::on_select_users);
    QObject::connect(open_private_room_PushButton, &QPushButton::clicked, this, &messenger::on_open_private_room);

}


messenger::~messenger()
{

}



void messenger::on_connect( QString server_ip_address, QString server_port )
{
    client = QSharedPointer<Client>::create( server_ip_address.toStdString(), server_port.toInt() );

    if ( client->create_socket() && client->connect_to_server() )
    {
        connect_Widget->close();

        login_register_Widget = QSharedPointer<login_register>::create();
        login_register_Widget->setStyleSheet(stylesheet);
        login_register_Widget->show();

        QObject::connect(login_register_Widget.data(), &login_register::re_sign_in_signal, this, &messenger::on_sign_in);
        QObject::connect(login_register_Widget.data(), &login_register::re_sign_up_signal, this, &messenger::on_sign_up);
    }
}


void messenger::on_sign_in(QString username, QString password)
{
    if ( client->execute_login(username.toStdString(), password.toStdString()) )
    {
        QObject::connect(client.data(), &Client::global_message_received, this, &messenger::on_global_message_received);
        QObject::connect(client.data(), &Client::private_message_received, this, &messenger::on_private_message_received);
        QObject::connect(client.data(), &Client::offline_user, this, &messenger::on_offline_user);
        QObject::connect(client.data(), &Client::online_user, this, &messenger::on_online_user);

        login_register_Widget->close();
        messenger_QWidget->show();

        this->username = username;
        user_name_QLabel->setText( username );
    }
}


void messenger::on_sign_up(QString full_name, QString username, QString password)
{
    if ( client->execute_register(full_name.toStdString(), username.toStdString(), password.toStdString()) )
    {
        QObject::connect(client.data(), &Client::global_message_received, this, &messenger::on_global_message_received);
        QObject::connect(client.data(), &Client::private_message_received, this, &messenger::on_private_message_received);
        QObject::connect(client.data(), &Client::offline_user, this, &messenger::on_offline_user);
        QObject::connect(client.data(), &Client::online_user, this, &messenger::on_online_user);

        login_register_Widget->close();
        messenger_QWidget->show();
        this->username = username;
        user_name_QLabel->setText( username );
    }
}


void messenger::on_logout()
{
    messenger_QWidget->close();
    emit close_all_windows();
}


void messenger::on_emoji_item_clicked( QTableWidgetItem * item )
{
    send_message_global_TextEdit->insertPlainText(item->text());
}


void messenger::on_show_emojis()
{
    emoji_TableWidget->setFixedSize(302,542);

    // specifies column and row count
    int row = 18, col = 10;

    // set font size for displaying emojies in table
    QFont font;
    font.setPointSize(16);  //set the font point size to 10

    // hides headers
    emoji_TableWidget->verticalHeader()->hide();
    emoji_TableWidget->horizontalHeader()->hide();
    emoji_TableWidget->verticalHeader()->setDefaultSectionSize(30);
    emoji_TableWidget->horizontalHeader()->setDefaultSectionSize(30);

    // set the resize mode to fixed, so the user cannot change the height/width
    emoji_TableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    emoji_TableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    emoji_TableWidget->setRowCount(row);
    emoji_TableWidget->setColumnCount(col);
    emoji_TableWidget->setFocusPolicy(Qt::StrongFocus);
    emoji_TableWidget->setFont(font);

    QFile emoji_data_file("stylesheets/emoji_final.dat");
    if (!emoji_data_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "failed to open :stylesheets/emoji_final.dat";
    }
    QTextStream stream(&emoji_data_file);
    QString emoji_string = stream.readAll();
    QStringList emoji_list = emoji_string.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QList<QString>::const_iterator emoji_it = emoji_list.begin();

    // iterate tough cells and set emojies fo each cell
    for (int r = 0; r < emoji_TableWidget->rowCount(); r++)
    {
        for (int c = 0; c < emoji_TableWidget->columnCount(); c++)
        {
            QTableWidgetItem* item = new QTableWidgetItem;
            //item->setFlags(Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled);

            item->setText(*emoji_it);
            ++emoji_it;
            emoji_TableWidget->setItem(r, c, item);
        }
    }

     emoji_TableWidget->show();
}


void messenger::on_send_global_message()
{
    QString q_message = send_message_global_TextEdit->toPlainText();
    std::string message = q_message.toStdString();

    if( !message.empty() )
    {
        client->send_global_message( message );

        main_global_TextEdit->append( q_message );
        send_message_global_TextEdit->clear();
    } else {
        QMessageBox::information(this, tr("Usage error"), tr("Message must not be empty." ));
    }


}


void messenger::on_global_message_received( QString sender, QString message )
{
    QString line = sender + "-> " + message;
    main_global_TextEdit->append( line );
}


void messenger::on_private_message_received( QString sender, QString message )
{
    (void)message;
    dest_names_vector dest_names;
    dest_names.push_back(sender.toStdString());

    private_rooms_QMap::iterator it = private_rooms.find(dest_names);
    if( it == private_rooms.end() )
    {
        it = private_rooms.insert( dest_names, QSharedPointer<private_window>::create( client, dest_names ) );
        // re emit the signal from clinet after new private room creation
        emit client->private_message_received( sender, message );
        it.value()->show();
    } else {
        it.value()->show();
    }

}


void messenger::on_select_users()
{
    online_users_ListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
}


void messenger::on_open_private_room()
{
    QList<QListWidgetItem *> online_users_ListWidgetItem_List = online_users_ListWidget->selectedItems();

    dest_names_vector dest_names;

    for( const auto& online_user_ListWidgetItem: online_users_ListWidgetItem_List )
    {
        dest_names.push_back( online_user_ListWidgetItem->text().toStdString() );
    }

    std::sort(dest_names.begin(), dest_names.end());


    if( !dest_names.empty() )
    {
        private_rooms_QMap::iterator it = private_rooms.find(dest_names);
        if( it == private_rooms.end() )
        {
            private_room new_room = QSharedPointer<private_window>::create( client, dest_names );
            it = private_rooms.insert( dest_names, new_room );
            it.value()->show();
        } else {
            it.value()->show();
        }

    } else {
        QMessageBox::information(this, tr("Usage error"), tr("For entering private "
                "room click and select users at first. For group conversation click "
                "'Select users' button, then select multiple users from list and "
                "click 'Open private room'" ));
    }


    online_users_ListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    online_users_ListWidget->clearSelection();
}


void messenger::on_online_user( QString new_online_user )
{
    online_users_ListWidget->addItem(new_online_user);
}


void messenger::on_offline_user( QString gone_offline_user )
{
    for ( int i = 0; i < online_users_ListWidget->count(); ++i )
    {
        if ( online_users_ListWidget->item(i)->text() == gone_offline_user )
        {
            online_users_ListWidget->takeItem(i);
            break;
        }
    }
}

