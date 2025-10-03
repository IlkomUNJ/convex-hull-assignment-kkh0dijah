#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QPointF>

struct ConvexHullResult {
    QVector<QPointF> hull;
    long long iterations;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onFindHullClicked();
    void onClearClicked();

private:
    QWidget *centralWidget;
    QGraphicsView *view;
    QGraphicsScene *scene;
    QPushButton *btnFindHull;
    QPushButton *btnClear;
    QLabel *labelPoints;
    QLabel *labelHullInfo;

    QVector<QPointF> points;
    QVector<QGraphicsEllipseItem*> pointItems;
    QVector<QGraphicsTextItem*> pointLabels;
    QVector<QGraphicsLineItem*> hullLines;

    void addPointAt(const QPointF &p);
    void clearHullLines();
    void drawHullFromPoints();

    ConvexHullResult computeFastConvexHull(const QVector<QPointF> &pts);
    ConvexHullResult computeSlowConvexHull(const QVector<QPointF> &pts);

    static qreal crossProduct(const QPointF &O, const QPointF &A, const QPointF &B);
};

#endif
