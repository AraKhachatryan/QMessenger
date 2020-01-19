

#include "private_window.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>


private_window::private_window( QSharedPointer<Client> client, group_dest_names_vector dest_names, QWidget *parent )
    : QWidget(parent), client(client), dest_names(dest_names)
{
    this->setFixedSize(530,685);
    this->setWindowTitle("Private room");

    mainLayout = new QHBoxLayout;
    textbox_bottom_layout = new QHBoxLayout;
    textboxes_layout = new QVBoxLayout;

    main_private_TextEdit = new QTextEdit;
    main_private_TextEdit->setFontPointSize(12);
    main_private_TextEdit->setFixedSize(500,450);
    main_private_TextEdit->setReadOnly(true);

    send_message_private_TextEdit = new QTextEdit;
    send_message_private_TextEdit->setFontPointSize(14);
    send_message_private_TextEdit->setFixedSize(500,100);

    show_emojis_PushButton = new  QPushButton;
    emoji_TableWidget = new QTableWidget;
    emoji_TableWidget->setWindowTitle("Emojies");


    send_private_message_PushButton = new QPushButton("Send message");
    send_private_message_PushButton->setFixedSize(140,30);

    QString dest_names_string;
    for( const auto& name: dest_names )
    {
        dest_names_string += QString::fromStdString(name) + " ";
    }

    user_name_QLabel = new QLabel(dest_names_string);
    QFont font_user_name( "Arial", 16, QFont::Bold);
    user_name_QLabel->setFont(font_user_name);
    user_name_QLabel->setTextFormat(Qt::RichText);
    user_name_QLabel->setAlignment(Qt::AlignCenter);
    user_name_QLabel->setFixedSize(500,30);


    spacer = new QSpacerItem(500, 5, QSizePolicy::Fixed);
    spacer1 = new QSpacerItem(295, 30, QSizePolicy::Fixed);
    spacer2 = new QSpacerItem(500, 7, QSizePolicy::Fixed);

    show_emojis_PushButton->setIcon(QIcon(QPixmap("icons/emoji_icon.png")));
    show_emojis_PushButton->setIconSize(QSize(30,30));
    show_emojis_PushButton->setFixedSize(40,40);
    show_emojis_PushButton->setStyleSheet("border-radius: 18px ;");

    textbox_bottom_layout->addWidget(show_emojis_PushButton);
    textbox_bottom_layout->addItem(spacer1);
    textbox_bottom_layout->addWidget(send_private_message_PushButton);

    textboxes_layout->addWidget(user_name_QLabel);
    textboxes_layout->addWidget(main_private_TextEdit);
    textboxes_layout->addItem(spacer2);
    textboxes_layout->addWidget(send_message_private_TextEdit);
    textboxes_layout->addItem(spacer);
    textboxes_layout->addLayout(textbox_bottom_layout);

    mainLayout->addLayout(textboxes_layout);

    QFile mainwindow_stylesheet_file("stylesheets/mainwindow_stylesheet.qss");
    mainwindow_stylesheet_file.open(QFile::ReadOnly);
    mainwindow_stylesheet = QLatin1String(mainwindow_stylesheet_file.readAll());

    this->setStyleSheet(mainwindow_stylesheet);
    this->setLayout(mainLayout);

    // connect slots & signals
    QObject::connect(show_emojis_PushButton, &QPushButton::clicked, this, &private_window::on_show_emojis);
    QObject::connect(emoji_TableWidget, &QTableWidget::itemClicked, this, &private_window::on_emoji_item_clicked);

    QObject::connect(send_private_message_PushButton, &QPushButton::clicked, this, &private_window::on_send_private_message);
    QObject::connect(client.data(), &Client::private_message_received, this, &private_window::on_private_message_received);

    QObject::connect(client.data(), &Client::offline_user, this, &private_window::on_offline_user);

}


private_window::~private_window()
{

}


void private_window::on_emoji_item_clicked( QTableWidgetItem * item )
{
    send_message_private_TextEdit->insertPlainText(item->text());
}


void private_window::on_show_emojis()
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


void private_window::on_send_private_message()
{
    QString q_message = send_message_private_TextEdit->toPlainText();
    std::string message = q_message.toStdString();

    if( !dest_names.empty() && !message.empty() )
    {
        client->send_private_message( dest_names, message );

        main_private_TextEdit->append( q_message );
        send_message_private_TextEdit->clear();
    } else {
        QMessageBox::information(this, tr("Usage error"), tr("Message must not be empty." ));
    }


}


void private_window::on_private_message_received( QString sender, QString message )
{
    QString line = sender + "-> " + message;
    main_private_TextEdit->append( line );
}


void private_window::on_offline_user( QString gone_offline_user )
{
    dest_names.erase(std::find( dest_names.begin(), dest_names.end(), gone_offline_user.toStdString() ));

    QString dest_names_string;
    for( const auto& name: dest_names )
    {
        dest_names_string += QString::fromStdString(name) + " ";
    }

    user_name_QLabel->setText(dest_names_string);

}


