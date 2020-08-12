/*!
 * \file
 * \brief Заголовочный файл класса MainWindow.
 * \author Кирилл Пушкарёв
 * \date 2017
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory> // unique_ptr

#include <QItemSelection>
#include <QMainWindow>

#include "notebook.hpp"

// Объявляем класс Ui::MainWindow, чтобы ниже можно было упоминать указатели на него,
// не включая определение класса. Этот класс создаётся автоматически из UI-файла.
// Данное объявление также было создано автоматически, когда Qt Creator
// добавлял в проект UI-файл и класс для него.
namespace Ui {
class MainWindow;
}

/*!
 * \brief Класс главного окна программы.
 *
 * Отвечает за реализацию функций, специфичных для данной конкретной
 * программы. За реализацию функций, общих для всех программ с графическим
 * интерфейсом, отвечает класс QMainWindow из библиотеки Qt.
 *
 * MainWindow является производным классом от QMainWindow и может добавлять свои
 * атрибуты и методы к тем, которые уже есть в QMainWindow.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //! Конструктор с необязательным указанием родительского объекта \a parent.
    explicit MainWindow(QWidget *parent = 0);
    //! Деструктор MainWindow.
    ~MainWindow();
    //! Событие закрытия приложения
    void closeEvent(QCloseEvent *event);
    /*
     * В этом разделе перечисляются слоты — методы, которые могут получать и
     * обрабатывать сигналы, например от активации пункта меню
     */
private slots:
    //! Отображает окно с информацией о программе.
    void displayAbout();
    //! Создаёт новую записную книжку.
    void newNotebook();
    //! Сохраняет текущую записную книжку. Возвращает \c true в случае успеха.
    bool saveNotebook();
    //! Сохраняет текущую записную книжку в указанном пользователем файле. Возвращает \c true в случае успеха.
    bool saveNotebookAs();
    //! Открывает записную книжку вместо текущей. Возвращает \c true в случае успеха.
    bool openNotebook();
    //! Закрывает текущую записную книжку. Возвращает \c true в случае успеха.
    bool closeNotebook();
    //! Создаёт новую заметку в текущей записной книжке. Возвращает \c true в случае успеха.
    bool newNote();
    //! Удаляет выбранные заметки из текущей записной книжки.
    void deleteNotes();
    //! Обновляет интерфейс окна.
    void updateUI();
    //! Обновляет заголовок окна.
    void refreshWindowTitle();
    //! Закрывает приложение.
    void on_actionExit_triggered();
    //! Открывает еКурсы.
    void on_actionVisit_eCourses_triggered();
    //! Экспортирует заметки в текстовом формате.
    bool on_actionSave_As_Text_triggered();
    //! Запускает диалог лотереи.
    void on_actionLottery_triggered();
    //! Запускает диалог редактирования заметки
    void on_notesView_activated(const QModelIndex &index);
    //! Запускает поиск текста заметки в интернете
    void on_actionWeb_search_triggered();

    // В этом разделе перечисляются сигналы, которые выдаёт данный класс
signals:
    /*!
     * \brief Сигнал об изменении имени открытого файла.
     * \param name Новое имя файла записной книжки.
     *
     * Сигнализирует, что имени открытого файла записной книжки присвоено
     * значение \a name.
     */
    void notebookFileNameChanged(QString name);
    //! Сигнализирует, что записная книжка готова к работе с пользователем (после создания или открытия).
    void notebookReady();
    //! Сигнализирует, что записная книжка успешно создана.
    void notebookCreated();
    /*!
     * \brief Сигнализирует, что записная книжка успешно открыта.
     * \param fileName Имя файла открытой записной книжки.
     */
    void notebookOpened(QString fileName);
    //! Сигнализирует, что записная книжка успешно сохранена.
    void notebookSaved();
    //! Сигнализирует, что записная книжка успешно закрыта.
    void notebookClosed();
    /*
     * В этом разделе перечисляются закрытые члены класса, которые обеспечивают
     * его работу и недоступны извне.
     */
private:
    /*!
     * \brief Сохраняет текущую записную книжку в файл.
     * \param fileName Имя файла.
     */
    void saveNotebookToFile(QString fileName);
    //! Возвращает \c true, если в настоящий момент имеется открытая записная книжка.
    bool isNotebookOpen() const;
    //! Устанавливает имя файла текущей записной книжки равным \a name.
    void setNotebookFileName(QString name = QString());
    //! Возвращает имя файла текущей записной книжки.
    QString notebookName() const;
    //! Создаёт новую записную книжку.
    void createNotebook();
    //! Устанавливает указатель на текущую записную книжку равным \a notebook.
    void setNotebook(Notebook *notebook);
    //! Уничтожает объект текущей записной книжки.
    void destroyNotebook();

    /*!
     * \brief Указатель на сгенерированный интерфейс.
     *
     * Указатель на объект UI-класса, сгенерированного на основе UI-файла
     * mainwindow.ui.
     *
     * Через этот указатель можно обратиться к элементам главного окна,
     * созданного в Qt Designer.
     * \sa \ref faq_qt_designer_whatis
     */
    Ui::MainWindow *mUi;
    /*!
     * \brief Указатель на текущую записную книжку.
     * \note std::unique_ptr, в целом, ведёт себя как обычный указатель, но
     * автоматически удаляет указуемый объект при присваивании другого значения
     * или уничтожении самого unique_ptr.
     */
    std::unique_ptr<Notebook> mNotebook;
    //! Имя файла текущей записной книжки.
    QString mNotebookFileName;
};

#endif // MAINWINDOW_H
