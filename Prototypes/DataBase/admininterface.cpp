#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlQueryModel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QSortFilterProxyModel>
#include <QHeaderView>


#include "admininterface.hpp"
#include "usersdatabase.hpp"

AdminInterface::AdminInterface(DatabaseManager *manager, QWidget *parent) : QWidget(parent), db(manager) {
    setupUI();
    updateTable();
}

AdminInterface::~AdminInterface() {
    // db.close();
}

void AdminInterface::setupUI() {
    addGroupButton = new QPushButton("Добавить группу", this);

    tableView = new QTableView(this);
    tableView->setSortingEnabled(true);
    model = new QSqlQueryModel(this);

    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setSortRole(Qt::EditRole);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    tableView->setModel(proxyModel);

    connect(tableView->horizontalHeader(), &QHeaderView::sectionClicked, [=](int index) {
        Qt::SortOrder order = proxyModel->sortOrder();
        if (proxyModel->sortColumn() == index) {
            order = (order == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
        } else {
            order = Qt::AscendingOrder;
        }
        proxyModel->sort(index, order);
    });

    findPanel = new QLineEdit(this);
    findPanel->setPlaceholderText("Введите логин пользователя");
    connect(findPanel, &QLineEdit::textChanged, this, &AdminInterface::findUsers);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(findPanel);
    layout->addWidget(addGroupButton);
    layout->addWidget(tableView);

    setLayout(layout);
    setWindowTitle("Admin Manager");
    resize(500, 400);
}

void AdminInterface::addGroup(){

}

void AdminInterface::setupDatabase(){
    // QDir dir = QCoreApplication::applicationDirPath();
    // QString dbPath = dir.absoluteFilePath("../../DataBaseFiles/" USERS_DATABASE);


    // db = QSqlDatabase::addDatabase("QSQLITE");
    // db.setDatabaseName(dbPath);


    // if (!db.open()) {
    //     qDebug() << "Error: connection with database failed";
    //     return;
    // }

    // QSqlQuery query;
    // QString createTableQuery = "CREATE TABLE IF NOT EXISTS " USERS_TABLE " ("
    //     ID " INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
    //     LOGIN " TEXT,"
    //     PASSWORD " TEXT,"
    //     TEAM " INTEGER,"
    //     STATUS " TEXT,"
    //     RIGHT " TEXT)";
    // if (!query.exec(createTableQuery)) {
    //     qDebug() << "Error creating table:" << query.lastError().text();
    // }else{
    //     qDebug() << "i here";
    // }
}

void AdminInterface::findUsers(){
    QSqlQuery query;
    QString userLogin = findPanel->text();
    if(userLogin != ""){
        query.prepare(
            "SELECT * FROM " USERS_TABLE " WHERE "
            LOGIN " LIKE :login || '%'");
        query.bindValue(":login", userLogin);
        query.exec();
        // model->setQuery(query);
        if (!query.exec()) {
            qDebug() << "Error find login in database:" << query.lastError().text();
            return;
        }
    }else{
        model->setQuery("SELECT * FROM " USERS_TABLE);
    }
}

void AdminInterface::updateTable() {
    model->setQuery("SELECT " ID " , " LOGIN " , " PASSWORD " , " TEAM " FROM " USERS_TABLE);
}
