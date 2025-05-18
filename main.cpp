#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMainWindow>
#include <QComboBox>
#include <QSlider>
#include <QGraphicsItem>
#include <QWheelEvent>
#include <QTransform>
#include <QDir>
#include <QGraphicsSceneWheelEvent>
#include <QListWidget>
#include <QDirIterator>

class FeatureItem : public QGraphicsPixmapItem {
public:
    FeatureItem(const QPixmap& pixmap) : QGraphicsPixmapItem(pixmap) {
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges);
    }

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event) override {
        double scaleFactor = (event->delta() > 0) ? 1.1 : 0.9;
        setScale(scale() * scaleFactor);
    }
};

class MainWindow : public QMainWindow {
    QGraphicsScene *scene;
    QGraphicsView *view;
    QWidget *central;
    QComboBox *featureSelector;
    QString basePath = QApplication::applicationDirPath() + "/assets";
    FeatureItem *currentItem = nullptr;
    QSlider *rotationSlider;
    QSlider *scaleSlider;
    QListWidget *featureList;

public:
    MainWindow() {
        scene = new QGraphicsScene(this);
        view = new QGraphicsView(scene);
        view->setRenderHint(QPainter::Antialiasing);

        featureSelector = new QComboBox();
        featureSelector->addItem("eyes");
        featureSelector->addItem("noses");
        featureSelector->addItem("mouths");
        featureSelector->addItem("ears");
        featureSelector->addItem("glasses");
        featureSelector->addItem("shoes");

        featureList = new QListWidget();
        featureList->setViewMode(QListView::IconMode);
        featureList->setIconSize(QSize(64, 64));
        featureList->setResizeMode(QListView::Adjust);
        featureList->setSpacing(10);
        featureList->setMovement(QListView::Static);
        featureList->setFixedHeight(120);  // show as a preview strip

        connect(featureList, &QListWidget::itemClicked, this, [=](QListWidgetItem *item) {
            QString path = item->data(Qt::UserRole).toString();
            QPixmap pixmap(path);
            auto *feature = new FeatureItem(pixmap);
            feature->setPos(100, 100);
            scene->addItem(feature);
        });


        QPushButton *addButton = new QPushButton("Add Feature");
        QPushButton *loadBody = new QPushButton("Load Potato");

        connect(loadBody, &QPushButton::clicked, this, &MainWindow::loadBodyImage);
        connect(addButton, &QPushButton::clicked, this, &MainWindow::addFeature);

        auto layout = new QHBoxLayout();
        layout->addWidget(featureSelector);
        layout->addWidget(addButton);
        layout->addWidget(loadBody);

        auto mainLayout = new QVBoxLayout();
        mainLayout->addLayout(layout);
        mainLayout->addWidget(view);

        rotationSlider = new QSlider(Qt::Horizontal);
        rotationSlider->setRange(-180, 180);
        rotationSlider->setTickInterval(10);
        rotationSlider->setTickPosition(QSlider::TicksBelow);

        scaleSlider = new QSlider(Qt::Horizontal);
        scaleSlider->setRange(10, 300);  // Represents 0.1 to 3.0
        scaleSlider->setTickInterval(10);
        scaleSlider->setTickPosition(QSlider::TicksBelow);

        connect(rotationSlider, &QSlider::valueChanged, this, [this](int value) {
            if (currentItem) currentItem->setRotation(value);
        });

        connect(scaleSlider, &QSlider::valueChanged, this, [this](int value) {
            if (currentItem) currentItem->setScale(value / 100.0);
        });

        connect(scene, &QGraphicsScene::selectionChanged, this, [this]() {
            if (scene->selectedItems().size() > 0)
                updateSlidersForItem(scene->selectedItems().first());
        });

        connect(featureSelector, &QComboBox::currentTextChanged, this, [=](const QString &text) {
            loadFeatureThumbnails(text);
        });

        loadBodyImage2();
        mainLayout->addWidget(featureList);

        mainLayout->addWidget(rotationSlider);
        mainLayout->addWidget(scaleSlider);
        central = new QWidget();
        central->setLayout(mainLayout);
        setCentralWidget(central);
        resize(800, 600);
        setWindowTitle("Mr. Potato Head Maker");

        featureSelector->setCurrentIndex(1);
        featureSelector->setCurrentIndex(0);
    }

    void updateSlidersForItem(QGraphicsItem *item) {
        currentItem = dynamic_cast<FeatureItem *>(item);
        if (currentItem) {
            rotationSlider->setValue(int(currentItem->rotation()));
            scaleSlider->setValue(int(currentItem->scale() * 100));
        }
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Delete) {
            for (QGraphicsItem *item : scene->selectedItems()) {
                scene->removeItem(item);
                delete item;
            }
        }
    }


    void loadFeatureThumbnails(const QString &folderName) {
        featureList->clear();
        QString dirPath = basePath + "/" + folderName;
        QDirIterator it(dirPath, {"*.png", "*.jpg", "*.jpeg"}, QDir::Files);
        while (it.hasNext()) {
            QString path = it.next();
            QPixmap pix(path);
            if (!pix.isNull()) {
                QListWidgetItem *item = new QListWidgetItem(QIcon(pix), "");
                item->setData(Qt::UserRole, path);
                item->setSizeHint(QSize(72, 72));
                featureList->addItem(item);
            }
        }
    }

    void loadBodyImage2() {
        QString path = QApplication::applicationDirPath() + "/assets/body.png";
        if (!path.isEmpty()) {
            scene->clear(); // clear previous
            QPixmap pixmap(path);
            auto body = scene->addPixmap(pixmap);
            body->setZValue(-1);  // background
        }
    }

    void loadBodyImage() {
        QString path = QFileDialog::getOpenFileName(this, "Select Potato Image", basePath, "Images (*.png *.jpg)");
        if (!path.isEmpty()) {
            scene->clear(); // clear previous
            QPixmap pixmap(path);
            auto body = scene->addPixmap(pixmap);
            body->setZValue(-1);  // background
        }
    }

    void addFeature() {
        QString featureType = featureSelector->currentText();
        QString dirPath = basePath + "/" + featureType;
        QString path = QFileDialog::getOpenFileName(this, "Select Feature", dirPath, "Images (*.png *.jpg)");

        if (!path.isEmpty()) {
            QPixmap pixmap(path);
            auto *item = new FeatureItem(pixmap);
            item->setPos(100, 100);  // arbitrary default
            scene->addItem(item);
            }
        }
    };

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
