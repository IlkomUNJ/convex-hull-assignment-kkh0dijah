#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QStatusBar>
#include <algorithm>
#include <cmath>
#include <QtMath>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    centralWidget(new QWidget(this)),
    view(new QGraphicsView(this)),
    scene(new QGraphicsScene(this)),
    btnFindHull(new QPushButton("Find Convex Hull", this)),
    btnClear(new QPushButton("Clear Canvas", this)),
    labelPoints(new QLabel("Points: 0", this)),
    labelHullInfo(new QLabel("Algorithms Iterations: N/A", this))
{

    auto *topLayout = new QHBoxLayout();
    topLayout->addWidget(btnFindHull);
    topLayout->addWidget(btnClear);
    topLayout->addSpacing(20);
    topLayout->addWidget(labelPoints);
    topLayout->addStretch();
    topLayout->addWidget(labelHullInfo);


    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QBrush(QColor(50,50,50)));
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);


    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(view);
    mainLayout->setContentsMargins(6,6,6,6);

    setCentralWidget(centralWidget);


    connect(btnFindHull, &QPushButton::clicked, this, &MainWindow::onFindHullClicked);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClearClicked);


    view->viewport()->installEventFilter(this);


    scene->setSceneRect(0, 0, 900, 600);
}

MainWindow::~MainWindow()
{

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == view->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                QPointF scenePos = view->mapToScene(me->pos());
                addPointAt(scenePos);


                if (!hullLines.isEmpty()) {
                    clearHullLines();
                    drawHullFromPoints();
                }

                if (points.size() < 3) {
                    labelHullInfo->setText("Algorithms Iterations:");
                }

                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::addPointAt(const QPointF &p)
{
    points.append(p);

    qreal r = 4.0;
    QGraphicsEllipseItem *ei = scene->addEllipse(p.x()-r, p.y()-r, 2*r, 2*r,
                                                 QPen(Qt::white), QBrush(Qt::blue));
    ei->setZValue(1);
    pointItems.append(ei);

    QGraphicsTextItem *txt = scene->addText(QString::number(points.size()));
    txt->setDefaultTextColor(Qt::white);
    txt->setFont(QFont("Arial", 10));
    txt->setPos(p.x()+6, p.y()-10);
    txt->setZValue(2);
    pointLabels.append(txt);

    labelPoints->setText(QString("Points: %1").arg(points.size()));
}

void MainWindow::clearHullLines()
{
    for (auto *ln : hullLines) {
        scene->removeItem(ln);
        delete ln;
    }
    hullLines.clear();
}

void MainWindow::drawHullFromPoints()
{
    if (points.size() < 3) {
        statusBar()->showMessage("Minimal 3 titik untuk menggambar convex hull.", 3000);
        return;
    }

    ConvexHullResult fastHull = computeFastConvexHull(points);
    ConvexHullResult slowHull = computeSlowConvexHull(points);
    QVector<QPointF> hull = fastHull.hull;

    labelHullInfo->setText(QString("Fast Algorithm Iterations: %L1 | Slow Algorithm Iterations: %L2")
                               .arg(fastHull.iterations)
                               .arg(slowHull.iterations));

    if (hull.isEmpty()) return;

    QPen pen(Qt::red);
    pen.setWidth(2);

    for (int i = 0; i < hull.size(); ++i) {
        QPointF a = hull[i];
        QPointF b = hull[(i+1) % hull.size()];
        QGraphicsLineItem *li = scene->addLine(QLineF(a,b), pen);
        li->setZValue(0);
        hullLines.append(li);
    }
}

void MainWindow::onFindHullClicked()
{
    clearHullLines();
    drawHullFromPoints();
}

void MainWindow::onClearClicked()
{
    for (auto *it : pointItems) { scene->removeItem(it); delete it; }
    pointItems.clear();

    for (auto *t : pointLabels) { scene->removeItem(t); delete t; }
    pointLabels.clear();

    clearHullLines();
    points.clear();

    labelPoints->setText("Points: 0");
    labelHullInfo->setText("Algorithms Iterations:");
    statusBar()->showMessage("Canvas dibersihkan.", 2000);
}

qreal MainWindow::crossProduct(const QPointF &O, const QPointF &A, const QPointF &B)
{
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

ConvexHullResult MainWindow::computeFastConvexHull(const QVector<QPointF> &pts)
{
    ConvexHullResult result;
    int n = pts.size();
    if (n < 3) return result;

    QVector<QPointF> a = pts;
    std::sort(a.begin(), a.end(), [](const QPointF &p1, const QPointF &p2){
        if (p1.x() != p2.x()) return p1.x() < p2.x();
        return p1.y() < p2.y();
    });


    result.iterations = (long long)n * std::max(1, (int)std::round(std::log2(n)));


    QVector<QPointF> lower, upper;

    for (const QPointF &p : a) {

        while (lower.size() >= 2 && crossProduct(lower[lower.size()-2], lower[lower.size()-1], p) <= 0)
            lower.removeLast();
        lower.append(p);
    }

    for (int i = a.size()-1; i >= 0; --i) {
        const QPointF &p = a[i];

        while (upper.size() >= 2 && crossProduct(upper[upper.size()-2], upper[upper.size()-1], p) <= 0)
            upper.removeLast();
        upper.append(p);
    }


    for (int i = 0; i < lower.size(); ++i) result.hull.append(lower[i]);
    for (int i = 1; i < upper.size() - 1; ++i) result.hull.append(upper[i]);

    return result;
}


ConvexHullResult MainWindow::computeSlowConvexHull(const QVector<QPointF> &pts)
{
    ConvexHullResult result;
    int n = pts.size();
    if (n < 3) return result;

    result.iterations = 0;


    int p0Index = 0;
    for (int i = 1; i < n; ++i) {
        if (pts[i].x() < pts[p0Index].x() ||
            (pts[i].x() == pts[p0Index].x() && pts[i].y() < pts[p0Index].y()))
        {
            p0Index = i;
        }
    }


    int p = p0Index;
    int next = -1;

    do {
        result.hull.append(pts[p]);
        next = (p + 1) % n;


        for (int i = 0; i < n; ++i) {
            result.iterations++;


            qreal cp = crossProduct(pts[p], pts[next], pts[i]);
            if (cp > 0) {
                next = i;
            }
        }

        p = next;

    } while (p != p0Index && result.hull.size() <= n);

    return result;
}
