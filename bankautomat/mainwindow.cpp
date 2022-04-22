#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    objRestApi = new Rest_api;
    objNumPad = new numpad_ui;
    oRfid = new Rfid_dll;

    QFont f( "Comic Sans MS", 25, QFont::Bold);
    ui->otsikkoLabel->setFont(f);

    ui->stackedWidget->setCurrentIndex(tervetuloaPage);

    //Tableviewin asetukset
    QHeaderView *hView;
    hView = ui->tableView->horizontalHeader();
    hView->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->stackedWidget->setCurrentIndex(0);

    ui->summatWidget->setVisible(false);
    ui->valikkoWidget->setVisible(false);

    connect(this, &MainWindow::requestLogin,
            objRestApi, &Rest_api::sendPost);

    connect(this, &MainWindow::requestGet,
            objRestApi, &Rest_api::sendGet);

    connect(this, &MainWindow::requestPost,
            objRestApi, &Rest_api::sendPost);

    connect(this, &MainWindow::requestPut,
            objRestApi, &Rest_api::sendPut);

    connect(this, &MainWindow::login,
            this, &MainWindow::loginHandler);

    connect(objRestApi, &Rest_api::returnData,
            this, &MainWindow::processData);

    connect(objNumPad, &numpad_ui::returnNum,
            this, &MainWindow::numpadHandler);

    connect(oRfid, &Rfid_dll::sendId,
            this, &MainWindow::getRfid);

    connect(this, &MainWindow::muuSumma,
            this, &MainWindow::summaHandler);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete objRestApi;
    objRestApi = nullptr;
    delete oRfid;
    oRfid = nullptr;
    delete objNumPad;
    objNumPad = nullptr;
}

void MainWindow::processData(QString resource, QByteArray data)
{
    if (resource == "login"){
        qDebug()<<"KIRJAUTUMINEN";
        webToken = data;
        qDebug()<<data;

        QString resourceX = "kortti/asiakasjatili/" + kortinnro; //nämä johonkin muualle
        emit requestGet(resourceX, webToken);                    //nämä johonkin muualle

    } else if (resource == "kortti/asiakasjatili/" + kortinnro){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        QJsonObject jsonObj = jsonDoc.object();

        //QString idAsiakas = QString::number(jsonObj["idAsiakas"].toInt());
        nimi = jsonObj["nimi"].toString();
        osoite = jsonObj["osoite"].toString();
        puhnro = jsonObj["puhelinnumero"].toString();
        tilinro = jsonObj["idTilinumero"].toString();
        saldo = QString::number(jsonObj["saldo"].toDouble()); //ei näy oikein

        qDebug()<<nimi;
        qDebug()<<osoite;
        qDebug()<<saldo;
        qDebug()<<tilinro;
        emit login();   //tämä johonkin muualle

    } else if (resource == "tilitapahtuma/"){ //kortin vai tilin perusteella?
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        QJsonArray json_array = jsonDoc.array();

        QStandardItemModel *table_model = new QStandardItemModel(10,4);
        table_model->setHeaderData(0, Qt::Horizontal, QObject::tr("Tilinumero"));
        table_model->setHeaderData(1, Qt::Horizontal, QObject::tr("Aikaleima"));
        table_model->setHeaderData(2, Qt::Horizontal, QObject::tr("Summa"));
        table_model->setHeaderData(3, Qt::Horizontal, QObject::tr("Tyyppi"));

        foreach (const QJsonValue &value, json_array) {
            QJsonObject jsonObj = value.toObject();

            QString date = jsonObj["dateTime"].toString();
            date.replace("-","/").replace("T"," ").chop(5);

            int row = jsonObj["idTilitapahtuma"].toInt() - 1;
            QStandardItem *Tilinumero = new QStandardItem(jsonObj["idTilinumero"].toString());
            table_model->setItem(row, 0, Tilinumero);
            QStandardItem *Aikaleima = new QStandardItem(date);
            table_model->setItem(row, 1, Aikaleima);
            QStandardItem *Summa = new QStandardItem(QString::number(jsonObj["summa"].toDouble()));
            table_model->setItem(row, 2, Summa);
            QStandardItem *Tyyppi = new QStandardItem(jsonObj["tilitapahtuma"].toString());
            table_model->setItem(row, 3, Tyyppi);
        }
        ui->tableView->setModel(table_model);
    }
}

void MainWindow::on_syotaPin_clicked()
{
    ui->stackedWidget->setCurrentIndex(kirjauduPage);
    objNumPad->stringSizeLimiter(true, 4);
    objNumPad->censorInput(true);
    objNumPad->show();
    state = kirjaudu;
}

void MainWindow::on_kirjaudu_clicked()
{
    kortinnro = ui->idKortti->text();

    QJsonObject jsonObj;
    jsonObj.insert("idKortti", kortinnro);
    jsonObj.insert("pin", num);

    QString resource = "login";
    emit requestLogin(resource, webToken, jsonObj);

    num.clear();
}


void MainWindow::on_tilitapahtumat_clicked()
{
    ui->stackedWidget->setCurrentIndex(tilitapahtumaPage);
    ui->summatWidget->setVisible(false);
    ui->otsikkoLabel->setText("Tilitapahtumat");

    QString resource = "tilitapahtuma/"; //kortin vai tilin perusteella?
    emit requestGet(resource, webToken);
}


void MainWindow::on_nosto_clicked()
{
    ui->stackedWidget->setCurrentIndex(nostoPage);
    ui->summatWidget->setVisible(true);
    ui->otsikkoLabel->setText("Nosto");
    state = nosto;
}


void MainWindow::on_talletus_clicked()
{
    ui->stackedWidget->setCurrentIndex(talletusPage);
    ui->summatWidget->setVisible(true);
    ui->otsikkoLabel->setText("Talletus");
    state = talletus;
}


void MainWindow::on_tilisiirto_clicked()
{
    ui->stackedWidget->setCurrentIndex(tilisiirtoPage);
    ui->summatWidget->setVisible(true);
    ui->otsikkoLabel->setText("Tilisiirto");
    state = tilisiirto;
}

void MainWindow::on_naytaTiedot_clicked()
{
    ui->stackedWidget->setCurrentIndex(naytatiedotPage);
    ui->summatWidget->setVisible(false);
    ui->otsikkoLabel->setText("Tietosi");
}

void MainWindow::getRfid(QString id)
{
    ui->stackedWidget->setCurrentIndex(kirjauduPage);
    id.remove(0,3).chop(3);
    ui->idKortti->setText(id);
    ui->kirjautumisLabel->clear();
}

void MainWindow::numpadHandler(QString paramNum)
{
    if(state == kirjaudu){
        num = paramNum;
        objNumPad->close();
    } else {
        objNumPad->close();
        emit muuSumma(paramNum, state);
    }
}

void MainWindow::on_kirjauduUlos_clicked()
{
    ui->stackedWidget->setCurrentIndex(tervetuloaPage);
    ui->summatWidget->setVisible(false);
    ui->valikkoWidget->setVisible(false);
    ui->otsikkoLabel->clear();
    num.clear();
    webToken.clear();
}

void MainWindow::loginHandler()
{
    if(webToken == "false") {
        ui->kirjautumisLabel->setText("PIN VÄÄRIN");
    } else {
        ui->stackedWidget->setCurrentIndex(mainPage);
        ui->valikkoWidget->setVisible(true);
        ui->otsikkoLabel->setText("Terve " + nimi);
        ui->saldoLCD->display(saldo);
    }
}

void MainWindow::on_summa10_clicked()
{
    summaHandler("10", state);
}


void MainWindow::on_summa20_clicked()
{
    summaHandler("20", state);
}


void MainWindow::on_summa50_clicked()
{
    summaHandler("50", state);
}


void MainWindow::on_summa100_clicked()
{
    summaHandler("100", state);
}


void MainWindow::on_summa500_clicked()
{
    summaHandler("500", state);
}

void MainWindow::on_summaMuu_clicked()
{
    objNumPad->stringSizeLimiter(false, 0);
    objNumPad->censorInput(false);
    objNumPad->show();
}

void MainWindow::summaHandler(QString summa, states s)
{
    num = summa;
    QString resource;
    QJsonObject jsonObj;

    QString receiverTilinro = "Tili_2";

    if(s == nosto){
        resource = "proseduuri/nosto";
        jsonObj.insert("kortinnro", kortinnro);
        jsonObj.insert("tilinro", tilinro);
        jsonObj.insert("rahasumma", num);
    } else if(s == talletus){
        resource = "proseduuri/talletus";
        jsonObj.insert("kortinnro", kortinnro);
        jsonObj.insert("tilinro", tilinro);
        jsonObj.insert("rahasumma", num);
    } else if(s == tilisiirto){
        resource = "proseduuri/tilisiirto";
        jsonObj.insert("kortinnro", kortinnro);
        jsonObj.insert("sendertilinro", tilinro);
        jsonObj.insert("receivertilinro", receiverTilinro);
        jsonObj.insert("rahasumma", num);
    }
    qDebug()<<jsonObj;
    emit requestPost(resource, webToken, jsonObj);
}

void MainWindow::on_showNumpad_clicked()
{
    objNumPad->show();
}

