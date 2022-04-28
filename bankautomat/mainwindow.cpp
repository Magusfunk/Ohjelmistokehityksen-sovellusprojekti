#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loginAttempts = 0;

    objRestApi = new Rest_api;
    objNumPad = new numpad_ui;
    oRfid = new Rfid_dll;
    ajastin = new QTimer;

    QFont f( "Comic Sans MS", 25, QFont::Bold);
    QFont f2( "Comic Sans MS", 18, QFont::Bold);
    ui->paaOtsikkoLabel->setFont(f);
    ui->paaOtsikkoLabel->setAlignment(Qt::AlignCenter);

    //  ui->otsikkoLabel->setFont(f); KOMMENTTINA KOSKA otsikkoLabel poistui vahingossa, se täytyy vielä lisätä UI:hin ja uncommentata tämä koodi.
    ui->paaOtsikkoLabel->setText(" Tervetuloa! <br>"
                                 " Syötä kortti.");

    /*
    ui->paaOtsikkoLabel_2->setFont(f);
    ui->paaOtsikkoLabel_2->setText(" Syötä Kortti");
    */

    ui->saldoLabel->setStyleSheet("font: 18pt;");
    ui->saldoLCD_2->setStyleSheet("font: 18pt;");
    ui->saldoLabel->setFont(f2);
    ui->saldoLCD_2->setFont(f2);
    ui->saldoLCD_2->setAlignment(Qt::AlignCenter);

    ui->saldoLabel->setStyleSheet("QLabel {background-color : black; color : white; }");

    ui->label_5->setStyleSheet("QLabel {color : black; }"); // nimi label

    ui->label_6->setStyleSheet("QLabel {color : black; }"); // osoite label

    ui->label_7->setStyleSheet("QLabel {color : black; }"); // puhelinnumero label

    ui->label_8->setStyleSheet("QLabel {color : black; }"); // tilinumero label

    ui->stackedWidget->setCurrentIndex(tervetuloaPage);

    //Tilitapahtumien viewin asetukset
    QHeaderView *hView;
    hView = ui->tilitapahtumaView->horizontalHeader();
    hView->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->summatWidget->setVisible(false);
    ui->valikkoWidget->setVisible(false);
    ui->nostoPageStackedWidget->setVisible(false);
    ui->talletusPageStackedWidget->setVisible(false);
    ui->siirtoPageStackedWidget->setVisible(false);

    connect(this, &MainWindow::requestLogin,
            objRestApi, &Rest_api::sendPost);

    connect(this, &MainWindow::requestGet,
            objRestApi, &Rest_api::sendGet);

    connect(this, &MainWindow::requestPost,
            objRestApi, &Rest_api::sendPost);

    connect(this, &MainWindow::requestPut,
            objRestApi, &Rest_api::sendPut);

    connect(objRestApi, &Rest_api::returnData,
            this, &MainWindow::processData);

    connect(objNumPad, &numpad_ui::numpadEnterClicked,
            this, &MainWindow::numpadEnter_clicked);

    connect(oRfid, &Rfid_dll::readEvent,
            this, &MainWindow::on_syotaPin_clicked);

    connect(ajastin, &QTimer::timeout,
            this, &MainWindow::on_kirjauduUlos_clicked);

    connect(ui->kyllaNostaButton, &QAbstractButton::clicked,
            this, &MainWindow::kylla_clicked);

    connect(ui->kyllaTalletaButton, &QAbstractButton::clicked,
            this, &MainWindow::kylla_clicked);

    connect(ui->kyllaSiirraButton, &QAbstractButton::clicked,
            this, &MainWindow::kylla_clicked);

    connect(ui->eiNostaButton, &QAbstractButton::clicked,
            this, &MainWindow::ei_clicked);

    connect(ui->eiTalletaButton, &QAbstractButton::clicked,
            this, &MainWindow::ei_clicked);

    connect(ui->eiSiirraButton, &QAbstractButton::clicked,
            this, &MainWindow::ei_clicked);

    connect(ui->nostaButton, &QAbstractButton::clicked,
            this, &MainWindow::NostaTalletaSiirra_clicked);

    connect(ui->talletaButton, &QAbstractButton::clicked,
            this, &MainWindow::NostaTalletaSiirra_clicked);

    connect(ui->siirraButton, &QAbstractButton::clicked,
            this, &MainWindow::NostaTalletaSiirra_clicked);


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
    delete ajastin;
    ajastin = nullptr;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    objNumPad->close();
    event->accept();
}

void MainWindow::kirjautumisHandler(events e)
{
    if (e == korttiSyotetty && kirjautunutState == false){
        kortinnro = oRfid->returnId();
        kortinnro.remove(0,3).chop(3);

        kortinnro = ui->idKortti->text(); //kovakoodaus testaamista varten syöttäkää kortin id line edittiin
        if(this->lukitutKortitCheck() == true){
            kirjautumisHandler(korttiLukittu);
        } else{
            event = korttiSyotetty;
            ui->stackedWidget->setCurrentIndex(kirjauduPage);

            ui->kirjautumisLabel->clear();
            //qDebug()<<kortinnro;

            objNumPad->stringSizeLimiter(true, 4);
            objNumPad->censorInput(true);


            ui->kirjautumisLabel->clear();
            qDebug()<<kortinnro;

        }
    } else if (e == pinSyotetty){
        //kortinnro = ui->idKortti->text();
        //kortinnro = "0A005968A0"; //kovakoodaus testaamista varten
        QString pin = objNumPad->returnNum();
        QJsonObject jsonObj;
        jsonObj.insert("idKortti", kortinnro);
        jsonObj.insert("pin", pin);
        QString resource = "login";
        emit requestLogin(resource, webToken, jsonObj);

    } else if (e == pinVaarin){
        ui->varoitusLabel->setStyleSheet("QLabel {background-color : black; color : white; }");
        ui->varoitusLabel->setText("PIN VÄÄRIN");

      // alempi ehto myöskin kommentiksi jos ei käytä testailussa kortinnro = (idKortti lineEditin arvo)
      if (ui->idKortti->text().isEmpty()) {
            ui->varoitusLabel->setText("Kortin ID puuttuu");
      }

        loginAttempts++;
        qDebug()<<loginAttempts;
        if(loginAttempts == 3){
            kirjautumisHandler(korttiLukittu);
        }
    } else if (e == pinOikein){
        objNumPad->close();
        QString resource = "kortti/asiakasjatili/" + kortinnro;
        loginAttempts = 0;
        kirjautunutState = true;
        emit requestGet(resource, webToken);

    } else if (e == korttiLukittu){
        ajastin->start(2000);
        objNumPad->close();
        if(this->lukitutKortitCheck()==false){
            qDebug()<<kortinnro + " lisätty lukittujen listaan";
            lukitutKortit.append(kortinnro);
        }
        pageHandler(pinLukittuPage,false,false,"");

    } else if (e == kirjauduUlos){
        pageHandler(tervetuloaPage, false, false, " Tervetuloa!");

        webToken.clear();
        ajastin->stop();
        kirjautunutState = false;
        loginAttempts = 0;
    }

}

void MainWindow::loginHandler()
{
    if(webToken == "false") {
        kirjautumisHandler(pinVaarin);
    } else {
        kirjautumisHandler(pinOikein);
    }
}

void MainWindow::loggedInHandler(events e)
{
    if (e == haeSaldoEtusivulle){
        QString resource = "tili/saldo/" + tilinro;
        emit requestGet(resource, webToken);
    } else if (e == naytaEtusivu){

        ui->saldoLCD_2->setText(saldo);

        pageHandler(mainPage, true, false, "Terve, " + nimi);

    } else if (e == nosto){
        toimenpide = nosta;
        ui->nostoPageStackedWidget->setVisible(false);
        ui->nostoLabel->clear();
        summa.clear();
        pageHandler(nostoPage, true, true, "Nosto");

    } else if (e == talletus){
        toimenpide = talleta;
        ui->talletusPageStackedWidget->setVisible(false);
        ui->talletusLabel->clear();
        summa.clear();
        pageHandler(talletusPage, true, true, "Talletus");

    } else if (e == tilisiirto){
        toimenpide = siirra;
        ui->siirtoPageStackedWidget->setVisible(false);
        ui->siirtoLabel->clear();
        summa.clear();
        rcvTilinro.clear();
        pageHandler(tilisiirtoPage, true, true, "Tilisiirto");

    } else if (e == haeTilitapahtumat){
        QString resource = "tilitapahtuma/tilinumero/" + tilinro;
        emit requestGet(resource, webToken);

    } else if (e == naytaTilitapahtumat){
        pageHandler(tilitapahtumaPage, true, false, "Tilitapahtumat");

    } else if (e == kayttajatiedot){
        pageHandler(tiedotPage, true, false, "Tietosi");

        ui->nimiLabel->setText(nimi);

        ui->osoiteLabel->setText(osoite);

        ui->puhnroLabel->setText(puhnro);

        ui->tilinroLabel->setText(tilinro);

    }
}

void MainWindow::pageHandler(pages sivu, bool valikko, bool summat, QString teksti)
{
    ui->stackedWidget->setCurrentIndex(sivu);
    ui->valikkoWidget->setVisible(valikko);
    ui->summatWidget->setVisible(summat);
    // ui->otsikkoLabel->setText(teksti); KOMMENTTINA KOSKA otsikkoLabel poistui vahingossa, se täytyy vielä lisätä UI:hin ja uncommentata tämä koodi.
}

void MainWindow::on_syotaPin_clicked()
{
    kirjautumisHandler(korttiSyotetty);
}

void MainWindow::numpadEnter_clicked()
{
    if (event == korttiSyotetty){
        kirjautumisHandler(pinSyotetty);
    } else if (event == muuSumma){
        summaButtonsHandler();
    } else if (event == tilinumero){
        tilinumeroHandler();
    }
}

void MainWindow::on_etusivu_clicked()
{
    loggedInHandler(haeSaldoEtusivulle);
}

void MainWindow::on_nosto_clicked()
{
    loggedInHandler(nosto);
}

void MainWindow::on_talletus_clicked()
{
    loggedInHandler(talletus);
}

void MainWindow::on_tilisiirto_clicked()
{
    loggedInHandler(tilisiirto);
}

void MainWindow::on_tilitapahtumat_clicked()
{
    loggedInHandler(haeTilitapahtumat);
}

void MainWindow::on_naytaTiedot_clicked()
{
    loggedInHandler(kayttajatiedot);
}

void MainWindow::on_kirjauduUlos_clicked()
{
    kirjautumisHandler(kirjauduUlos);
}

void MainWindow::on_summa10_clicked()
{
    event = summaClicked;
    summa = "10";
    summaButtonsHandler();
}

void MainWindow::on_summa20_clicked()
{
    event = summaClicked;
    summa = "20";
    summaButtonsHandler();
}

void MainWindow::on_summa50_clicked()
{
    event = summaClicked;
    summa = "50";
    summaButtonsHandler();
}

void MainWindow::on_summa100_clicked()
{
    event = summaClicked;
    summa = "100";
    summaButtonsHandler();
}

void MainWindow::on_summa500_clicked()
{
    event = summaClicked;
    summa = "500";
    summaButtonsHandler();
}

void MainWindow::on_summaMuu_clicked()
{
    event = muuSumma;
    objNumPad->stringSizeLimiter(false, 0);
    objNumPad->censorInput(false);
    objNumPad->show();
}

void MainWindow::on_syotaTilinumero_clicked()
{
    event = tilinumero;
    objNumPad->stringSizeLimiter(false, 0);
    objNumPad->censorInput(false);
    objNumPad->show();
}

void MainWindow::tilinumeroHandler()
{
    rcvTilinro = objNumPad->returnNum();
    objNumPad->close();
    ui->siirtoLabel->setText("Tilinumero syötetty");
    ui->siirtoPageStackedWidget->setCurrentIndex(0);
}

bool MainWindow::lukitutKortitCheck()
{
    bool cardLocked;
    for(int i = 0;i<lukitutKortit.size();i++){
        qDebug()<<"Lukittujen korttien lista: " + lukitutKortit.value(i);
        if(kortinnro == lukitutKortit.value(i)){
            cardLocked = true;
            return cardLocked;
        }
    }
    cardLocked = false;
    return cardLocked;

}

void MainWindow::summaButtonsHandler()
{
    if(event == muuSumma){
        summa = objNumPad->returnNum();
        qDebug()<<summa;
    }
    objNumPad->close();

    if(toimenpide == nosta){
        ui->nostoLabel->setText("Summa <b>" + summa + "</b> syötetty");
        ui->nostoPageStackedWidget->setVisible(true);
        ui->nostoPageStackedWidget->setCurrentIndex(0);
    } else if(toimenpide == talleta){
        ui->talletusLabel->setText("Summa <b>" + summa + "</b> syötetty");
        ui->talletusPageStackedWidget->setVisible(true);
        ui->talletusPageStackedWidget->setCurrentIndex(0);
    } else if(toimenpide == siirra){
        ui->siirtoLabel->setText("Summa <b>" + summa + "</b> syötetty <br> Syötä tilinumero");
        ui->siirtoPageStackedWidget->setVisible(true);
        ui->siirtoPageStackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::NostaTalletaSiirra_clicked()
{
    if(toimenpide == nosta){
        ui->nostoPageStackedWidget->setCurrentIndex(1);
        ui->nostoLabel->setText("Haluatko varmasti nostaa <b>" + summa + "</b> rahaa?");
    } else if(toimenpide == talleta){
        ui->talletusPageStackedWidget->setCurrentIndex(1);
        ui->talletusLabel->setText("Haluatko varmasti tallettaa <b>" + summa + "</b> rahaa?");
    } else if(toimenpide == siirra){
        ui->siirtoPageStackedWidget->setCurrentIndex(1);
        ui->siirtoLabel->setText("Haluatko varmasti siirtää <b>" + summa + "</b> rahaa tilille <b>" + rcvTilinro + "</b> ?");
    }
}

void MainWindow::kylla_clicked()
{
    if(toimenpide == nosta){
        ui->nostoLabel->setText("Nosto onnistui!");
        ui->nostoPageStackedWidget->setVisible(false);
    } else if(toimenpide == talleta){
        ui->talletusLabel->setText("Talletus onnistui!");
        ui->talletusPageStackedWidget->setVisible(false);
    } else if(toimenpide == siirra){
        ui->siirtoLabel->setText("Tilisiirto onnistui!");
        ui->siirtoPageStackedWidget->setVisible(false);
    }

    rahaliikenneHandler();
}

void MainWindow::ei_clicked()
{
    if(toimenpide == nosta){
        loggedInHandler(nosto);
    } else if(toimenpide == talleta){
        loggedInHandler(talletus);
    } else if(toimenpide == siirra){
        loggedInHandler(tilisiirto);
    }
}

void MainWindow::rahaliikenneHandler()
{
    QString resource;
    QJsonObject jsonObj;

    if(toimenpide == nosta){
        resource = "proseduuri/nosto";
        jsonObj.insert("kortinnro", kortinnro);
        jsonObj.insert("tilinro", tilinro);
        jsonObj.insert("rahasumma", summa);
    } else if(toimenpide == talleta){
        resource = "proseduuri/talletus";
        jsonObj.insert("kortinnro", kortinnro);
        jsonObj.insert("tilinro", tilinro);
        jsonObj.insert("rahasumma", summa);
    } else if(toimenpide == siirra){
        resource = "proseduuri/tilisiirto";
        jsonObj.insert("kortinnro", kortinnro);
        jsonObj.insert("sendertilinro", tilinro);
        jsonObj.insert("receivertilinro", rcvTilinro);
        jsonObj.insert("rahasumma", summa);
    }
    qDebug()<<jsonObj;
    rcvTilinro.clear();
    summa.clear();
    emit requestPost(resource, webToken, jsonObj);
}

void MainWindow::on_showNumpad_clicked()
{
    objNumPad->show();
}

void MainWindow::processData(QString resource, QByteArray data)
{
    if (resource == "login"){
        webToken = data;
        emit login();
        loginHandler();
    } else if (resource == "kortti/asiakasjatili/" + kortinnro){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        QJsonObject jsonObj = jsonDoc.object();

        //QString idAsiakas = QString::number(jsonObj["idAsiakas"].toInt());
        nimi = jsonObj["nimi"].toString();
        osoite = jsonObj["osoite"].toString();
        puhnro = jsonObj["puhelinnumero"].toString();
        tilinro = jsonObj["idTilinumero"].toString();
        saldo = QString::number(jsonObj["saldo"].toDouble(), 'f', 0); //ei näy oikein

        loggedInHandler(naytaEtusivu);

    } else if (resource == "tili/saldo/" + tilinro){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        QJsonObject jsonObj = jsonDoc.object();
        saldo = QString::number(jsonObj["saldo"].toDouble(), 'f', 0);

        loggedInHandler(naytaEtusivu);

    } else if (resource == "tilitapahtuma/tilinumero/" + tilinro){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        QJsonArray json_array = jsonDoc.array();

        QStandardItemModel *tilitapahtumaModel = new QStandardItemModel(10,3);
        tilitapahtumaModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Aikaleima"));
        tilitapahtumaModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Summa"));
        tilitapahtumaModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Tyyppi"));

        for(int row = 0;row<json_array.size();row++){
            QJsonValue value = json_array.at(row);
            QJsonObject jsonObj = value.toObject();

            QString date = jsonObj["dateTime"].toString();
            date.replace("-","/").replace("T"," ").chop(5);

            QStandardItem *Aikaleima = new QStandardItem(date);
            tilitapahtumaModel->setItem(row, 0, Aikaleima);
            QStandardItem *Summa = new QStandardItem(QString::number(jsonObj["summa"].toDouble()));
            tilitapahtumaModel->setItem(row, 1, Summa);
            QStandardItem *Tyyppi = new QStandardItem(jsonObj["tilitapahtuma"].toString());
            tilitapahtumaModel->setItem(row, 2, Tyyppi);
        }
        ui->tilitapahtumaView->setModel(tilitapahtumaModel);
        loggedInHandler(naytaTilitapahtumat);
    }
}





