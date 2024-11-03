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
#include "database.hpp"

AdminInterface::AdminInterface(DatabaseManager *manager, QWidget *parent) : QWidget(parent), db(manager) {
    setupUI();
    updateTable();
}

void AdminInterface::setupUI() {
    addGroupButton = new QPushButton("Добавить группу", this);

    tableView = new QTableView(this);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


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
}

void AdminInterface::addGroup(){

}
void AdminInterface::findUsers(){
    QString userName = findPanel->text();
    if(userName != ""){
        QString request ="SELECT * FROM " USERS_TABLE " WHERE " LOGIN " LIKE ? || '%'";
        QVariantList values;
        values.append(QVariant(userName));
        QSqlQuery query = db->execPreparedQuery(request, values);
        if (!query.exec()) {
            qDebug() << "Error find of file:" << query.lastError().text();
            return;
        }
        model->setQuery(std::move(query));
    }else{
        model->setQuery("SELECT * FROM " USERS_TABLE);
    }
}

void AdminInterface::updateTable() {
    model->setQuery("SELECT " ID " , " LOGIN " , " PASSWORD " , " TEAM " FROM " USERS_TABLE);
}
