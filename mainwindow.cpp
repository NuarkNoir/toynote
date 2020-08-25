/*!
 * \file
 * \brief Файл реализации класса MainWindow.
 * \author Кирилл Пушкарёв
 * \date 2017
 */
#include "mainwindow.hpp"
// Заголовочный файл UI-класса, сгенерированного на основе mainwindow.ui
#include "ui_mainwindow.h"

#include <set>
#include <stdexcept>

#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSaveFile>
#include <QUrlQuery>
#include <QtGlobal> // qVersion()
#include <QDateTime>

#include "config.hpp"
#include "editnotedialog.hpp"
#include "loteryprocessor.h"

/*!
 * Конструирует объект класса с родительским объектом \a parent.
 * Параметр \p parent имеет значение по умолчанию 0. Указывать родительский
 * объект нужно, например чтобы дочерний объект был автоматически удалён
 * при удалении родительского. В случае главного окна родителя можно не указывать.
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), // Передаём parent конструктору базового класса
    mUi(new Ui::MainWindow) // Создаём объект Ui::MainWindow
{
    // Присоединяем сигналы, соответствующие изменению статуса записной книжки,
    // к слоту, обеспечивающему обновление интерфейса окна
    connect(this, &MainWindow::notebookReady, this, &MainWindow::updateUI);
    connect(this, &MainWindow::notebookClosed, this, &MainWindow::updateUI);
    // Присоединяем сигнал создания записной книжки к лямбда-выражению,
    // устанавливающему в главном окне признак изменения (имеет ли текущий
    // документ несохранённые изменения). В заголовке окна при наличии
    // несохранённых изменений будет отображаться звёздочка или другое
    // обозначение, в зависимости от системы. "[this]" в лямбда-выражении
    // означает, что оно обращается к методам данного класса (в данном случае к
    // методу MainWindow::setWindowModified()).
    //
    // Лямбда-выражение — это выражение, результатом которого является
    // функциональный объект (объект, действующий как функция). В фигурных
    // скобках записывается тело этой функции. Таким образом, в данном случае
    // сигнал MainWindow::notebookCreated будет вызывать код, записанный в
    // фигурных скобках, то есть метод MainWindow::setWindowModified() с параметром true.
    connect(this, &MainWindow::notebookCreated, this, [this] { setWindowModified(true); });
    connect(this, &MainWindow::notebookSaved, this, [this] { setWindowModified(false); });

    // Отображаем GUI, сгенерированный из файла mainwindow.ui, в данном окне
    mUi->setupUi(this);
    // Настраиваем таблицу заметок, чтобы её последняя колонка занимала всё доступное место
    mUi->notesView->horizontalHeader()->setStretchLastSection(true);
    // Обновляем заголовок окна
    refreshWindowTitle();
    // Создаём новую записную книжку
    newNotebook();
}

/*!
* Отвечает за уничтожение объектов MainWindow. Сюда можно поместить
* функции, которые надо выполнить перед уничтожением (например, закрыть
* какие-либо файлы или освободить память).
*/
MainWindow::~MainWindow()
{
    // Удаляем объект Ui::MainWindow
    delete mUi;
}

void MainWindow::closeEvent(QCloseEvent *_) {
    on_actionExit_triggered();
}

void MainWindow::displayAbout()
{
    /*
     * Создаём простой диалог типа QMessageBox, который является дочерним
     * по отношению к главному окну. Указатель this указывает на тот объект
     * класса (MainWindow), для которого был вызван данный метод (displayAbout()).
     *
     * Для создания стандартных вариантов этих окон (окно информации, сообщение
     * об ошибке и т. д.) есть статические методы, создающие готовое окно со
     * всеми необходимыми настройками: QMessageBox::information(),
     * QMessageBox::critical() и т. д.
     */
    QMessageBox aboutDlg(this);
    // Включаем расширенное форматирование текста (разновидность HTML, позволяет
    // выделять текст, ставить ссылки и т. д.) в окне aboutDlg
    aboutDlg.setTextFormat(Qt::RichText);
    /*
     * Устанавливаем заголовок окна aboutDlg. Функция tr() отвечает за перевод строки на другой
     * язык, если он предусмотрен в данной программе. Метод arg() заменяет в строке
     * %1, %2 и т. д. на переданную ему строку. В данном случае, с его помощью
     * в строку заголовка подставляется название программы, которое хранится
     * в пространстве имён Config в файле config.hpp
     */
    aboutDlg.setWindowTitle(tr("About %1").arg(Config::applicationName));
    // Устанавливаем иконку информационного сообщения
    aboutDlg.setIcon(QMessageBox::Information);
    // Устанавливаем основной текст в окне aboutDlg
    aboutDlg.setText(tr("%1 %2<br>"
        "Author: <a href=\"mailto:kpushkarev@sfu-kras.ru\">Kirill Pushkaryov</a>, 2019.<br>"
        "Author: Gorbatsevich Andrei Anatolyevich, KI19-07B, 031941597.<br>"
        "This application is dynamically linked against the "
        "<a href=\"https://www.qt.io/developers/\">Qt Library</a> "
        "v. %3, which is under the LGPLv3 license.<br>"
        "Icons by <a href=\"http://tango.freedesktop.org/"
        "Tango_Desktop_Project\">The Tango! Desktop Project</a>.")
        .arg(Config::applicationName).arg(Config::applicationVersion)
        .arg(qVersion()));
    // Отображаем окно как модальное (блокирующее все остальные окна)
    aboutDlg.exec();
}

void MainWindow::newNotebook()
{
    // Прежде чем открыть новую, закрываем текущую записную книжку
    if (!closeNotebook())
    {
        // Если записная книжка не была закрыта, прерываем операцию
        return;
    }
    // Создаём новую записную книжку
    createNotebook();
    // Сбрасываем имя файла текущей записной книжки
    setNotebookFileName();
    // Сигнализируем о готовности
    emit notebookReady();
    // Сигнализируем о создании записной книжки
    emit notebookCreated();
}

bool MainWindow::saveNotebook()
{
    // Если записная книжка не открыта, выдаём сообщение об этом
    if (!isNotebookOpen())
    {
        QMessageBox::warning(this, Config::applicationName, tr("No open notebooks"));
        return false;
    }
    // Если для текущей записной книжки не установлено имя файла...
    if (mNotebookFileName.isEmpty())
    {
        // Задействуем функцию "сохранить как" и выходим
        return saveNotebookAs();
    }
    else
    {
        // Cохраняем в текущий файл
        saveNotebookToFile(mNotebookFileName);
    }
    // Сигнализируем о готовности
    emit notebookReady();
    // Сигнализируем о сохранении записной книжки
    emit notebookSaved();
    return true;
}

bool MainWindow::saveNotebookAs()
{
    // Если записная книжка не открыта, выдаём сообщение об этом
    if (!isNotebookOpen())
    {
        QMessageBox::warning(this, Config::applicationName, tr("No open notebooks"));
        return false;
    }
    // Выводим диалог выбора файла для сохранения
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Notebook As"), QString(), Config::notebookFileNameFilter);
    // Если пользователь не выбрал файл, возвращаем false
    if (fileName.isEmpty())
    {
        return false;
    }
    // Сохраняем записную книжку в выбранный файл
    saveNotebookToFile(fileName);
    // Устанавливаем выбранное имя файла в качестве текущего
    setNotebookFileName(fileName);
    // Сигнализируем о готовности
    emit notebookReady();
    // Сигнализируем о сохранении записной книжки
    emit notebookSaved();
    return true;
}

bool MainWindow::openNotebook()
{
    // Если записная книжка открыта, сначала закрываем её
    if (isNotebookOpen() && !closeNotebook())
    {
        // Если записная книжка не была закрыта, возвращаем false
        return false;
    }
    // Выводим диалог выбора файла для открытия
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Notebook"), QString(), Config::notebookFileNameFilter);
    // Если пользователь не выбрал файл, возвращаем false
    if (fileName.isEmpty())
    {
        return false;
    }
    // Блок обработки исключительных ситуаций
    try
    {
        // Создаём объект inf, связанный с файлом fileName
        QFile inf(fileName);
        // Открываем файл только для чтения
        if (!inf.open(QIODevice::ReadOnly))
        {
            throw std::runtime_error((tr("open(): ") + inf.errorString()).toStdString());
        }
        // Привязываем к файлу поток, позволяющий считывать объекты Qt
        QDataStream ist(&inf);
        // Создаём новый объект записной книжки
        std::unique_ptr<Notebook> nb(new Notebook);
        // Загружаем записную книжку из файла
        ist >> *nb;
        // Устанавливаем новую записную книжку в качестве текущей.
        // Метод release() забирает указатель у объекта nb
        setNotebook(nb.release());
    }
    catch (const std::exception &e)
    {
        // Если при открытии файла возникла исключительная ситуация, сообщить пользователю
        QMessageBox::critical(this, Config::applicationName, tr("Unable to open the file %1: %2").arg(fileName).arg(e.what()));
        return false;
    }
    // Устанавливаем текущее имя файла
    setNotebookFileName(fileName);
    // Сигнализируем о готовности
    emit notebookReady();
    // Сигнализируем об открытии записной книжки
    emit notebookOpened(mNotebookFileName);
    return true;
}

bool MainWindow::closeNotebook()
{
    // Если записная книжка не открыта, возвращаем true
    if (!isNotebookOpen())
    {
        return true;
    }
    // Создаём окно с вопросом о сохранении файла
    QMessageBox saveQuery(this);
    // Устанавливаем иконку вопроса
    saveQuery.setIcon(QMessageBox::Question);
    // Ставим название программы в заголовок
    saveQuery.setWindowTitle(Config::applicationName);
    // Устанавливаем текст вопроса. Вместо %1 метод arg() подставит в строку
    // результат notebookName() (название документа)
    saveQuery.setText(tr("Would you like to save %1?").arg(notebookName()));
    // Добавляем в окно стандартные кнопки: сохранить, не сохранять и отменить
    saveQuery.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    // Выбираем кнопку по умолчанию — сохранить
    saveQuery.setDefaultButton(QMessageBox::Save);
    // Выводим модальный диалог saveQuery и смотрим результат
    switch (saveQuery.exec())
    {
    case QMessageBox::Save: // Сохранить
        // Сохранить записную книжку
        if (!saveNotebook())
        {
            return false;
        }
        // Здесь не должно быть break
    case QMessageBox::Discard: // Закрыть без сохранения
        // Уничтожаем объект записной книжки
        destroyNotebook();
        // Сбрасываем текущее имя файла
        setNotebookFileName();
        // Сигнализируем о закрытии записной книжки
        emit notebookClosed();
        break;
    case QMessageBox::Cancel: // Отмена
        return false;
    }
    return true;
}

bool MainWindow::newNote()
{
    // Если записная книжка не открыта, выдаём сообщение об этом
    if (!isNotebookOpen())
    {
        QMessageBox::warning(this, Config::applicationName, tr("No open notebooks"));
        return false;
    }
    // Создаём диалог редактирования заметки
    EditNoteDialog noteDlg(this);
    // Устанавливаем заголовок noteDlg
    noteDlg.setWindowTitle(tr("New Note"));
    // Создаём заметку и передаём указатель на неё noteDlg
    Note note;
    noteDlg.setNote(&note);
    // Если пользователь не подтвердил изменения, возвращаем false
    if (noteDlg.exec() != EditNoteDialog::Accepted)
    {
        return false;
    }
    // Вставляем заметку в записную книжку
    mNotebook->insert(note);
    return true;
}

void MainWindow::deleteNotes()
{
    // Создаём окно с вопросом об удалении заметок
    QMessageBox delCong(this);
    delCong.setIcon(QMessageBox::Question);
    delCong.setWindowTitle(tr("Deletion confirmation"));
    delCong.setText(tr("Are you sure you want to delete these note(s)?"));
    delCong.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delCong.setDefaultButton(QMessageBox::No);
    // Выводим модальный диалог saveQuery и смотрим результат
    if (delCong.exec() == QMessageBox::No) {
        return;
    }

    // Для хранения номеров строк создаём STL-контейнер "множество", элементы
    // которого автоматически упорядочиваются по возрастанию
    std::set<int> rows;
    {
        // Получаем от таблицы заметок список индексов выбранных в настоящий момент
        // элементов
        QModelIndexList idc = mUi->notesView->selectionModel()->selectedRows();
        // Вставляем номера выбранных строк в rows
        for (const auto &i : idc)
        {
            rows.insert(i.row());
        }
    }
    // Обходим множество номеров выбранных строк *по убыванию*, чтобы удаление предыдущих
    // не сбивало нумерацию следующих
    for (auto it = rows.rbegin(); it != rows.rend(); ++it)
    {
        // Удаляем строку
        mNotebook->erase(*it);
    }
}

void MainWindow::updateUI() {
    refreshWindowTitle();

    // сохраним состояние наличия открытой записной книжки в переменную, для производительности
    bool ino = isNotebookOpen();
    this->mUi->actionSave           ->setEnabled(ino);  // File|Save
    this->mUi->actionSave_As        ->setEnabled(ino);  // File|Save as
    this->mUi->actionSave_As_Text   ->setEnabled(ino);  // File|Save as text
    this->mUi->actionCloseNotebook  ->setEnabled(ino);  // File|Close
    this->mUi->actionNew_Note       ->setEnabled(ino);  // Add
    this->mUi->notesView            ->setEnabled(ino);  // Notes grid

    // хэндлер выделения заметок;
    // мы сбрасываем модель при закрытии нотбука, поэтому нужно устанавливать каждый раз новый
    if (ino) {
        connect(
            mUi->notesView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this] (const QItemSelection &selected) {
                bool selected_any = selected.size() > 0;
                this->mUi->actionDelete_Notes->setEnabled(selected_any);  // Delete

                bool selected_only = mUi->notesView->selectionModel()->selectedIndexes().size() == 1;
                this->mUi->actionWeb_search->setEnabled(selected_only);  // Search
            }
        );
    }
}

void MainWindow::refreshWindowTitle()
{
    QString nbname = notebookName();
    // Если имя записной книжки не пусто...
    if (!nbname.isEmpty())
    {
        // Ставим в заголовок окна название программы (Config::applicationName)
        // и имя текущей записной книжки (nbname)
        // Метка "[*]" обозначает место, куда будет подставлена звёздочка, если
        // установлен флаг изменения окна (см. QWidget::setWindowModified())
        setWindowTitle(tr("%1 - %2[*]").arg(Config::applicationName).arg(nbname));
    }
    else
    {
        // Ставим в заголовок окна только название программы (Config::applicationName)
        setWindowTitle(Config::applicationName);
    }
}

/*!
 * Сохраняет текущую записную книжку в файл \a fileName. Данный метод
 * отвечает непосредственно за сохранение и не предусматривает диалога с
 * пользователем.
 */
void MainWindow::saveNotebookToFile(QString fileName)
{
    // Если записная книжка не открыта, прерываем операцию
    if (!isNotebookOpen())
    {
        return;
    }
    // Блок обработки исключительных ситуаций
    try
    {
        /*
         * Создаём объект outf, связанный с файлом fileName.
         * QSaveFile обеспечивает безопасное сохранение (через промежуточный
         * временный файл), чтобы избежать потери данных в случае нештатного
         * завершения операции сохранения. Само сохранение происходит при вызове
         * метода commit().
         */
        QSaveFile outf(fileName);
        // Открываем файл только для записи
        outf.open(QIODevice::WriteOnly);
        // Привязываем к файлу поток, позволяющий выводить объекты Qt
        QDataStream ost(&outf);
        // Выводим записную книжку в файл
        ost << *mNotebook;
        // Запускаем сохранение и смотрим результат.
        // В случае неудачи запускаем исключительную ситуацию (блок прерывается,
        // управление передаётся в блок catch)
        if (!outf.commit())
        {
            throw std::runtime_error(tr("Unable to commit the save").toStdString());
        }
        // Устанавливаем текущее имя файла
        setNotebookFileName(fileName);
    }
    catch (const std::exception &e)
    {
        // Если при сохранении файла возникла исключительная ситуация, сообщить пользователю
        QMessageBox::critical(this, Config::applicationName, tr("Unable to write to the file %1: %2").arg(fileName).arg(e.what()));
    }
}

bool MainWindow::isNotebookOpen() const
{
    // Преобразуем указатель mNotebook к типу bool. Нулевой указатель при этом
    // даст false, ненулевой — true
    return static_cast<bool>(mNotebook);
}

void MainWindow::setNotebookFileName(QString name)
{
    // Устанавливаем имя файла
    mNotebookFileName = name;
    // Сигнализируем о смене имени файла
    emit notebookFileNameChanged(name);
}

QString MainWindow::notebookName() const
{
    // Если записная книжка не открыта, возвращаем пустую строку
    if (!isNotebookOpen())
    {
        return QString();
    }
    // Иначе, если имя текущего файла пустое, возвращаем строку "Untitled"
    else if (mNotebookFileName.isEmpty())
    {
        return tr("Untitled");
    }
    // Возвращаем короткое имя текущего файла (без пути)
    return QFileInfo(mNotebookFileName).fileName();
}

void MainWindow::createNotebook()
{
    setNotebook(new Notebook);
}

void MainWindow::setNotebook(Notebook *notebook)
{
    /*
     * Заменяем имеющийся указатель на объект записной книжки новым.
     * Если в mNotebook хранился какой-то ненулевой указатель на объект,
     * то метод reset() удалит его автоматически
     */
    mNotebook.reset(notebook);
    // Связываем новый объект записной книжки с таблицей заметок в главном окне
    mUi->notesView->setModel(mNotebook.get());
}

/*!
 * Прекращает отображение текущей записной книжки в графическом интерфейсе
 * и удаляет объект Notebook по указателю mNotebook.
 */
void MainWindow::destroyNotebook()
{
    // Отключаем объект записной книжки от таблицы заметок в главном окне
    mUi->notesView->setModel(0);
    // Удаляем объект записной книжки
    mNotebook.reset();
}

void MainWindow::on_actionExit_triggered()
{
    if (isNotebookOpen())
    {
        QMessageBox exitDlg(this);
        exitDlg.setIcon(QMessageBox::Question);
        exitDlg.setWindowTitle(Config::applicationName);
        exitDlg.setText(tr("Would you like to save %1 before exit?").arg(notebookName()));
        exitDlg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        exitDlg.setDefaultButton(QMessageBox::Save);
        if (exitDlg.exec() == QMessageBox::Save)
        {
            saveNotebook();
        }
    }

    QCoreApplication::exit(0);
}

void MainWindow::on_actionVisit_eCourses_triggered()
{
    QDesktopServices::openUrl(QUrl("https://e.sfu-kras.ru"));
}

void MainWindow::on_actionSave_As_Text_triggered()
{
    // Выводим диалог выбора файла для сохранения
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Notebook As Text"), QString(), Config::textNotebookFileNameFilter);
    // Если пользователь не выбрал файл, возвращаем false
    if (fileName.isEmpty())
    {
        return;
    }

    // Сохраняем записную книжку в выбранный файл в текстовом формате
    try
    {
        QSaveFile outf(fileName);
        outf.open(QIODevice::WriteOnly);
        QTextStream ost(&outf);
        auto ns = mNotebook->size();
        for (int i = 0; i < ns; i++) {
            ost << QString("+++ %1/%2 +++\n").arg(i+1).arg(ns);
            ost << tr("Title: ") << (*mNotebook)[i].title() << "\n";
            ost << tr("Content: ") << (*mNotebook)[i].text() << "\n";
            ost << QString("--- %1/%2 ---\n").arg(i+1).arg(ns);
        }

        if (!outf.commit())
        {
            throw std::runtime_error(tr("Unable to commit the save").toStdString());
        }
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, Config::applicationName, tr("Unable to write to the file %1: %2").arg(fileName).arg(e.what()));
    }
}

void MainWindow::on_actionLottery_triggered()
{
    QMessageBox aboutDlg(this);
    aboutDlg.setTextFormat(Qt::RichText);
    aboutDlg.setWindowTitle(tr("Lottery"));
    aboutDlg.setIcon(QMessageBox::NoIcon);
    auto lot_result = nuarkd::LoteryProcessor().obtainPrize();
    auto dialog_text = tr("Today is %1<br>").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy"));

    if (lot_result.first) {
        dialog_text += tr("You won our lottery and your prize is... <br><br><b>%1</b><br><br>Congrats!")
                        .arg(lot_result.second);
    }
    else {
        dialog_text += tr("You lose our lottery, but don't get upset! You can try again!");
    }

    aboutDlg.setText(dialog_text);
    aboutDlg.exec();
}

void MainWindow::on_notesView_activated(const QModelIndex &index)
{
    if (mUi->notesView->selectionModel()->selectedRows().size() != 1) {
        return;
    }

    int pos = index.row();
    // Создаём диалог редактирования заметки
    EditNoteDialog noteDlg(this);
    noteDlg.setWindowTitle(tr("Edit Note"));

    Note note = (*mNotebook)[pos];
    noteDlg.setNote(&note);

    if (noteDlg.exec() == EditNoteDialog::Accepted)
    {
        mNotebook->updateNoteAt(note, pos);
    }
}

void MainWindow::on_actionWeb_search_triggered()
{
    QUrlQuery query("https://yandex.ru/search/?");
    auto note = (*mNotebook)[mUi->notesView->selectionModel()->currentIndex().row()];
    query.addQueryItem("text", note.text());
    QString url = query.toString();
    QDesktopServices::openUrl(url);
}
