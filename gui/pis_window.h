#ifndef PIS_WINDOW_H
#define PIS_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <QAtomicInt>

#include "pis_config.h"
#include "pis_schedule.h"

Q_DECLARE_METATYPE(pis::ScheduleEvent)

class PISWorker : public QThread {
    Q_OBJECT
public:
    PISWorker(const pis::Route& route, QObject* parent = nullptr);
    void stop();
signals:
    void eventOccurred(const pis::ScheduleEvent& event);
    void finished();
    void tick(int elapsed_sec, int station_idx);
protected:
    void run() override;
private:
    pis::Route route_;
    std::atomic<bool> running_{true};
};

class PISWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit PISWindow(QWidget* parent = nullptr);
    ~PISWindow() override;
public slots:
    void onEvent(const pis::ScheduleEvent& event);
    void onTick(int elapsed_sec, int station_idx);
    void onFinished();
private slots:
    void onStartClicked();
    void onStopClicked();
private:
    void setupUI();
    void setupStyles();
    void populateStationList(const pis::Route& route);
    QLabel *title_label_, *route_label_, *current_station_label_, *next_station_label_;
    QLabel *event_type_label_, *announcement_label_, *time_label_;
    QProgressBar* route_progress_;
    QListWidget* station_list_;
    QPushButton *start_button_, *stop_button_;
    pis::ConfigManager config_mgr_;
    PISWorker* worker_{nullptr};
    pis::Route current_route_;
    int total_stations_{0};
};

#endif // PIS_WINDOW_H
