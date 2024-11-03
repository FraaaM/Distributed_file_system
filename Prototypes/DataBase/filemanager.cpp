#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QDir>
#include <QCoreApplication>
#include <filesystem>

#include "filemanager.hpp"
#include "database.hpp"
#include "databasemanager.hpp"



FileManager::FileManager(DatabaseManager* manager,QWidget *parent) : QWidget(parent), db(manager) {
    setupUI();
    updateTable();
}


void FileManager::setupUI() {
    addButton = new QPushButton("Add File", this);
    connect(addButton, &QPushButton::clicked, this, &FileManager::addFile);

    downloadButton = new QPushButton("Download File", this);
    connect(downloadButton, &QPushButton::clicked, this, &FileManager::downloadFile);

    removeButton = new QPushButton("Remove File", this);
    connect(removeButton, &QPushButton::clicked, this, &FileManager::removeFile);

    refreshDataButton = new QPushButton("Refresh Data", this);
    connect(refreshDataButton, &QPushButton::clicked, this, &FileManager::updateTable);

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
    findPanel->setPlaceholderText("Введите название файла");
    connect(findPanel, &QLineEdit::textChanged, this, &FileManager::findFiles);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(findPanel);
    layout->addWidget(addButton);
    layout->addWidget(removeButton);
    layout->addWidget(downloadButton);
    layout->addWidget(refreshDataButton);
    layout->addWidget(tableView);

    setLayout(layout);
    setWindowTitle("File Manager");
}
void FileManager::addFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select File" , "" , "All Files (*)");
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "File Selection" , "No file selected.");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "File Error" , "Could not open file.");
        return;
    }
    QString fileName = QFileInfo(filePath).fileName();

    QDir dir = QCoreApplication::applicationDirPath();
    QString backupFolder = dir.absoluteFilePath("Backup/");

    if(!std::filesystem::exists(backupFolder.toStdString())){
        std::filesystem::create_directory(backupFolder.toStdString());
    }

    if(!std::filesystem::exists(backupFolder.toStdString())){
        QMessageBox::critical(this, "Error" , "Doesn't exist backup folder!");
        return;
    }
    QString backupPath = dir.absoluteFilePath(backupFolder + fileName);

    const std::filesystem::path fromPath = filePath.toStdString().data();
    const std::filesystem::path toPath = backupPath.toStdString().data();

    if(std::filesystem::exists(fromPath))
        std::filesystem::copy(fromPath, toPath);
    else{
        QMessageBox::critical(this, "Error" , "Doesn't exist path!");
        return;
    }


    qint64 fileSize = file.size();
    QString uploadDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    bool isPrivate = false;

    file.close();


    QString request = "INSERT INTO " FILES_TABLE " (" FILE_NAME ", " FILE_SIZE ", " UPLOAD_DATE ", " FILE_PATH ", " FILE_IS_PRIVATE ") "
                      "VALUES (?, ?, ?, ?, ?)";

    QVariantList values;
    values.append(QVariant(fileName));
    values.append(QVariant(fileSize));
    values.append(QVariant(uploadDate));
    values.append(QVariant(backupPath));
    values.append(QVariant(isPrivate));

    QSqlQuery query = db->execPreparedQuery(request, values);
    updateTable();
}

void FileManager::downloadFile(){
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection Error" , "Please select a file to download.");
        return;
    }
    int row = index.row();
    int fieId = model->data(model->index(row, 0)).toInt();

    QString request = "SELECT " FILE_NAME " , " FILE_PATH " FROM " FILES_TABLE " WHERE " ID " = ?";

    QVariantList values;
    values.append(QVariant(fieId));
    QSqlQuery query = db->execPreparedQuery(request, values);

    if (query.next()) {
        QString fileName = query.value(0).toString();
        const std::filesystem::path filePath = query.value(1).toString().toStdString();

        QString savePath = QFileDialog::getSaveFileName(this, "Save File", fileName);
        if (savePath.isEmpty()) {
            return;
        }
        const std::filesystem::path pathTo = savePath.toStdString();

        if(std::filesystem::exists(filePath)){
            std::filesystem::copy(filePath, pathTo);
        }else{
            QMessageBox::critical(this, "Error" , "File is not exist!");
            return;
        }

        QMessageBox::information(this, "Success" , "File downloaded successfully.");
    }
}

void FileManager::removeFile(){
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection Error" , "Please select a file to remove.");
        return;
    }

    int row = index.row();
    int field = model->data(model->index(row, 0)).toInt();

    QString request = "SELECT " FILE_PATH " FROM " FILES_TABLE " WHERE " ID " = ?";
    QVariantList values;
    values.append(QVariant(field));
    QSqlQuery query = db->execPreparedQuery(request, values);

    while(query.next()){
        const std::filesystem::path backupPath = query.value(0).toString().toStdString();
        std::filesystem::remove(backupPath);
        values.clear();
    }

    request = "DELETE FROM " FILES_TABLE " WHERE " ID " = ?";
    values.append(QVariant(field));
    query = db->execPreparedQuery(request, values);

    updateTable();

    QMessageBox::information(this, "Success" , "File removed successfully.");
}

void FileManager::findFiles(){
    QString fileName = findPanel->text();
    if(fileName != ""){
        QString request ="SELECT " ID " , " FILE_NAME " , " FILE_SIZE " , " UPLOAD_DATE " FROM " FILES_TABLE " WHERE " FILE_NAME " LIKE ? || '%'";
        QVariantList values;
        values.append(QVariant(fileName));
        QSqlQuery query = db->execPreparedQuery(request, values);
        if (!query.exec()) {
            qDebug() << "Error find of file:" << query.lastError().text();
            return;
        }
        model->setQuery(std::move(query));
    }else{
        model->setQuery("SELECT " ID " , " FILE_NAME " , " FILE_SIZE " , " UPLOAD_DATE " FROM " FILES_TABLE);
    }
}

void FileManager::updateTable() {
    model->setQuery("SELECT " ID " , " FILE_NAME " , " FILE_SIZE " , " UPLOAD_DATE " FROM " FILES_TABLE);
}
