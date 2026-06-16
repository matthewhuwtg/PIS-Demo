#include "pis_window.h"
#include <QFont>
#include <QCoreApplication>
#include <iostream>

PISWorker::PISWorker(const pis::Route& route, QObject* parent) : QThread(parent), route_(route) {}
void PISWorker::stop() { running_ = false; }
void PISWorker::run() {
    qRegisterMetaType<pis::ScheduleEvent>("pis::ScheduleEvent");
    pis::ScheduleEngine schedule;
    schedule.initialize(route_);
    schedule.onEvent([this](const pis::ScheduleEvent& ev) { emit eventOccurred(ev); });
    schedule.start();
    while (running_ && schedule.isRunning()) {
        emit tick(schedule.elapsedSeconds(), schedule.currentStationIndex());
        msleep(200);
    }
    if (!running_) schedule.stop();
    emit finished();
}

PISWindow::PISWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI(); setupStyles();
    qRegisterMetaType<pis::ScheduleEvent>("pis::ScheduleEvent");
}
PISWindow::~PISWindow() { if (worker_ && worker_->isRunning()) { worker_->stop(); worker_->wait(3000); } }

void PISWindow::setupUI() {
    setWindowTitle("PIS - Passenger Information System");
    setMinimumSize(850, 700);
    resize(950, 720);
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(20, 15, 20, 15);

    title_label_ = new QLabel("== Passenger Information System ==");
    QFont tf; tf.setPointSize(22); tf.setBold(true); title_label_->setFont(tf);
    title_label_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title_label_);

    route_label_ = new QLabel("Route: --");
    QFont rf; rf.setPointSize(13); route_label_->setFont(rf);
    route_label_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(route_label_);

    event_type_label_ = new QLabel("Status: [IDLE]");
    QFont ef; ef.setPointSize(15); ef.setBold(true); event_type_label_->setFont(ef);
    event_type_label_->setAlignment(Qt::AlignCenter);
    event_type_label_->setFixedHeight(35);
    mainLayout->addWidget(event_type_label_);

    current_station_label_ = new QLabel("Current Station: --");
    QFont sf; sf.setPointSize(20); sf.setBold(true); current_station_label_->setFont(sf);
    current_station_label_->setAlignment(Qt::AlignCenter);
    current_station_label_->setFixedHeight(45);
    mainLayout->addWidget(current_station_label_);

    next_station_label_ = new QLabel("Next Stop: --");
    QFont nf; nf.setPointSize(13); next_station_label_->setFont(nf);
    next_station_label_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(next_station_label_);

    route_progress_ = new QProgressBar();
    route_progress_->setMinimum(0); route_progress_->setMaximum(100);
    route_progress_->setValue(0); route_progress_->setFixedHeight(28);
    mainLayout->addWidget(route_progress_);

    time_label_ = new QLabel("Elapsed: 0m 00s");
    QFont tif; tif.setPointSize(12); time_label_->setFont(tif);
    time_label_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(time_label_);

    announcement_label_ = new QLabel("Announcement: --");
    QFont af; af.setPointSize(11); af.setItalic(true); announcement_label_->setFont(af);
    announcement_label_->setAlignment(Qt::AlignCenter);
    announcement_label_->setWordWrap(true);
    announcement_label_->setFixedHeight(40);
    mainLayout->addWidget(announcement_label_);

    station_list_ = new QListWidget();
    station_list_->setMinimumHeight(180);
    QFont lf; lf.setPointSize(12); station_list_->setFont(lf);
    mainLayout->addWidget(station_list_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    start_button_ = new QPushButton("Start Simulation");
    start_button_->setFixedSize(200, 45);
    stop_button_ = new QPushButton("Stop");
    stop_button_->setFixedSize(140, 45);
    stop_button_->setEnabled(false);
    btnLayout->addStretch();
    btnLayout->addWidget(start_button_);
    btnLayout->addSpacing(15);
    btnLayout->addWidget(stop_button_);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(start_button_, &QPushButton::clicked, this, &PISWindow::onStartClicked);
    connect(stop_button_, &QPushButton::clicked, this, &PISWindow::onStopClicked);
}

void PISWindow::setupStyles() {
    setStyleSheet(R"(
        QMainWindow { background-color: #1a1a2e; }
        QLabel { color: #e0e0e0; background: transparent; }
        QProgressBar {
            border: 2px solid #16213e; border-radius: 10px;
            background-color: #0f3460; text-align: center;
            color: white; font-weight: bold; font-size: 13px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #e94560, stop:0.5 #e94560, stop:1 #0f3460);
            border-radius: 8px;
        }
        QListWidget {
            background-color: #16213e; color: #8899aa;
            border: 1px solid #0f3460; border-radius: 8px; padding: 5px;
            font-size: 13px; outline: none;
        }
        QListWidget::item {
            padding: 8px 12px; border-bottom: 1px solid #0f3460; border-radius: 4px;
        }
        QListWidget::item:selected { background-color: #e94560; color: white; }
        QPushButton {
            background-color: #0f3460; color: white; border: none;
            padding: 10px 25px; border-radius: 8px;
            font-size: 15px; font-weight: bold;
        }
        QPushButton:hover { background-color: #e94560; }
        QPushButton:disabled { background-color: #333; color: #666; }
    )");
}

void PISWindow::populateStationList(const pis::Route& route) {
    station_list_->clear();
    for (size_t i = 0; i < route.stations.size(); ++i) {
        station_list_->addItem(QString("[%1] %2").arg(route.stations[i].id.c_str()).arg(route.stations[i].name_en.c_str()));
    }
}

void PISWindow::onStartClicked() {
    std::string config_path;
    QString appDir = QCoreApplication::applicationDirPath();
    if (appDir.endsWith("gui/build")) config_path = appDir.toStdString() + "/../../config/sample_config.json";
    else config_path = appDir.toStdString() + "/config/sample_config.json";
    if (!config_mgr_.load(config_path)) { announcement_label_->setText("[ERROR] Failed to load config!"); return; }

    const auto& cfg = config_mgr_.get();
    current_route_ = cfg.route;
    total_stations_ = static_cast<int>(current_route_.stations.size());
    route_label_->setText(QString("Route: %1 (%2, %3 stations)").arg(cfg.route.name.c_str()).arg(cfg.route.direction.c_str()).arg(total_stations_));
    setWindowTitle(QString("PIS - %1").arg(cfg.route.name.c_str()));
    populateStationList(current_route_);
    start_button_->setEnabled(false);
    stop_button_->setEnabled(true);
    if (total_stations_ > 0) {
        station_list_->setCurrentRow(0);
        current_station_label_->setText(current_route_.stations[0].name_en.c_str());
        next_station_label_->setText(total_stations_ > 1 ? QString("Next: %1 ->").arg(current_route_.stations[1].name_en.c_str()) : "Next: --");
    }
    event_type_label_->setText("Status: [DEPARTING]");
    announcement_label_->setText("Train departing...");

    worker_ = new PISWorker(current_route_, this);
    connect(worker_, &PISWorker::eventOccurred, this, &PISWindow::onEvent);
    connect(worker_, &PISWorker::tick, this, &PISWindow::onTick);
    connect(worker_, &PISWorker::finished, this, &PISWindow::onFinished);
    worker_->start();
}

void PISWindow::onStopClicked() {
    if (worker_ && worker_->isRunning()) { worker_->stop(); worker_->wait(2000); }
    onFinished();
}

void PISWindow::onEvent(const pis::ScheduleEvent& event) {
    QString stationEn = event.station_name_en.c_str();
    current_station_label_->setText(stationEn);
    QString status;
    switch (event.type) {
        case pis::ScheduleEventType::DEPARTURE:   status = "Status: [DEPARTURE]";   announcement_label_->setText("Now departing from " + stationEn); break;
        case pis::ScheduleEventType::APPROACHING: status = "Status: [APPROACHING]"; announcement_label_->setText("Approaching " + stationEn); break;
        case pis::ScheduleEventType::NEXT_STOP:   status = "Status: [NEXT STOP]";   announcement_label_->setText("Next stop: " + stationEn); break;
        case pis::ScheduleEventType::ARRIVAL:     status = "Status: [ARRIVED]";     announcement_label_->setText("Arrived at " + stationEn); break;
        case pis::ScheduleEventType::TERMINUS:    status = "Status: [TERMINUS]";    announcement_label_->setText("Terminus: " + stationEn + " - Thank you"); break;
        default: status = "Status: " + QString(event.typeString().c_str()); announcement_label_->setText(event.message.c_str()); break;
    }
    event_type_label_->setText(status);
    int ni = event.station_index + 1;
    if (ni < total_stations_) next_station_label_->setText(QString("Next: %1 ->").arg(current_route_.stations[ni].name_en.c_str()));
    else next_station_label_->setText("Next: -- (Terminus)");
    if (total_stations_ > 1) route_progress_->setValue((event.station_index * 100) / (total_stations_ - 1));
    int m = event.elapsed_sec / 60, s = event.elapsed_sec % 60;
    time_label_->setText(QString("Elapsed: %1m %2s").arg(m).arg(s, 2, 10, QChar('0')));
    if (event.station_index >= 0 && event.station_index < station_list_->count()) station_list_->setCurrentRow(event.station_index);
}

void PISWindow::onTick(int elapsed_sec, int station_idx) {
    int m = elapsed_sec / 60, s = elapsed_sec % 60;
    time_label_->setText(QString("Elapsed: %1m %2s").arg(m).arg(s, 2, 10, QChar('0')));
    if (total_stations_ > 1) route_progress_->setValue((station_idx * 100) / (total_stations_ - 1));
}

void PISWindow::onFinished() {
    start_button_->setEnabled(true);
    stop_button_->setEnabled(false);
    if (worker_) { worker_->deleteLater(); worker_ = nullptr; }
    event_type_label_->setText("Status: [STOPPED]");
}
