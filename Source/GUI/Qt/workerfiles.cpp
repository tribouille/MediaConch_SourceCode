/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#include "workerfiles.h"
#include "mainwindow.h"
#include "Common/FileRegistered.h"

#include "DatabaseUi.h"

#include <QString>
#include <QDir>
#include <QTimer>
#include <pthread.h>

namespace MediaConch {

//***************************************************************************
// Constructor / Desructor WorkerFilesValidate
//***************************************************************************

//---------------------------------------------------------------------------
WorkerFilesValidate::WorkerFilesValidate(MainWindow* m, WorkerFiles* w, const std::string& f, FileRegistered* r)
                    : QThread(), is_running(false), mainwindow(m), worker(w), file(f), fr(r)
{
    flag_start = true;
    flag_kill = false;
    connect(this, SIGNAL(terminated()), this, SLOT(plop()), Qt::DirectConnection);
}

//---------------------------------------------------------------------------
WorkerFilesValidate::~WorkerFilesValidate()
{
}

//---------------------------------------------------------------------------
void WorkerFilesValidate::plop()
{
    printf("is terminated\n");
}

//---------------------------------------------------------------------------
bool WorkerFilesValidate::stop()
{
    mut.lock();
    if (flag_start)
        flag_kill = true;
    mut.unlock();

    printf("lock:%d\n", flag_kill);
    if (flag_kill)
        terminate();

    bool w = wait(1000);

    printf("avant wait:%d\n", isFinished());

    return !flag_kill;
}

//---------------------------------------------------------------------------
void WorkerFilesValidate::run()
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    is_running = true;

    printf("run lock 1\n");
    mut.lock();
    if (flag_kill)
    {
        printf("run unlock 1\n");
        mut.unlock();
        return;
    }

    flag_start = true;
    printf("run unlock 1\n");
    mut.unlock();

    std::string err;
    validate(err);

    printf("run lock 2\n");
    mut.lock();
    if (flag_kill)
    {
        printf("run unlock 2\n");
        mut.unlock();
        return;
    }

    flag_start = false;
        printf("run unlock 2\n");
    mut.unlock();

    fr->analyzed = true;
    worker->update_validate_of_registered_file(file, fr, err);
    worker->emit_signal_validate_end(file);
}

void WorkerFilesValidate::validate(std::string& err)
{
    std::vector<size_t> policies_ids;
    std::vector<std::string> policies_contents;
    std::map<std::string, std::string> options;
    std::vector<MediaConchLib::Checker_ValidateRes*> res;

    if (mainwindow->validate((MediaConchLib::report)fr->report_kind, fr->file_id,
                             policies_ids, policies_contents, options, res, err) < 0)
        mainwindow->set_str_msg_to_status_bar(err);
    else if (!res.size())
        err = "Internal error: Validate result is not correct.";
    else
        fr->implementation_valid = res[0]->valid;

    for (size_t j = 0; j < res.size(); ++j)
        delete res[j];
    res.clear();

    if (fr->policy >= 0)
    {
        policies_ids.push_back(fr->policy);

        if (mainwindow->validate(MediaConchLib::report_Max, fr->file_id,
                                 policies_ids, policies_contents, options, res, err) < 0)
            mainwindow->set_str_msg_to_status_bar(err);
        else if (!res.size())
            err = "Internal error: Validate result is not correct.";
        else
            fr->policy_valid = res[0]->valid;
        for (size_t j = 0; j < res.size(); ++j)
            delete res[j];
        res.clear();
    }
}

//***************************************************************************
// Constructor / Desructor WorkerFiles
//***************************************************************************

//---------------------------------------------------------------------------
WorkerFiles::WorkerFiles(MainWindow* m) : QThread(), mainwindow(m), db(NULL), timer(NULL),
                                          file_index(0), max_threads(1), current(0)
{
    qRegisterMetaType<std::string>("std::string");
    connect(this, SIGNAL(validate_finished(std::string)), this, SLOT(remove_validate_thread(std::string)));
}

//---------------------------------------------------------------------------
WorkerFiles::~WorkerFiles()
{
    if (timer)
    {
        timer->stop();
        delete timer;
        timer = NULL;
    }

    to_validate_files_mutex.lock();
    std::map<std::string, WorkerFilesValidate*>::iterator it = to_validate_files.begin();
    for (; it != to_validate_files.end(); ++it)
    {
        if (it->second)
        {
            it->second->stop();
            delete it->second;
        }
    }
    to_validate_files.clear();
    to_validate_files_mutex.unlock();
}

//---------------------------------------------------------------------------
void WorkerFiles::set_database(DatabaseUi* database)
{
    db = database;
}

//---------------------------------------------------------------------------
void WorkerFiles::run()
{
    timer = new QTimer(0);
    timer->moveToThread(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_files_registered()), Qt::DirectConnection);
    timer->start(1000); //TODO
    exec();
    if (timer)
    {
        timer->stop();
        delete timer;
        timer = NULL;
    }
}

//***************************************************************************
// Functions
//***************************************************************************

//---------------------------------------------------------------------------
FileRegistered* WorkerFiles::get_file_registered_from_file(const std::string& file)
{
    FileRegistered* fr = NULL;
    working_files_mutex.lock();
    if (working_files.find(file) != working_files.end() && working_files[file])
        fr = new FileRegistered(*working_files[file]);
    working_files_mutex.unlock();
    return fr;
}

//---------------------------------------------------------------------------
FileRegistered* WorkerFiles::get_file_registered_from_id(long id)
{
    FileRegistered* fr = NULL;
    working_files_mutex.lock();
    std::map<std::string, FileRegistered*>::iterator it = working_files.begin();
    for (; it != working_files.end(); ++it)
    {
        if (!it->second)
            continue;

        if (it->second->file_id == id)
        {
            fr = new FileRegistered(*it->second);
            break;
        }
    }
    working_files_mutex.unlock();
    return fr;
}

//---------------------------------------------------------------------------
long WorkerFiles::get_id_from_registered_file(const std::string& file)
{
    long id = -1;
    working_files_mutex.lock();
    if (working_files.find(file) != working_files.end() && working_files[file])
        id = working_files[file]->file_id;
    working_files_mutex.unlock();
    return id;
}

//---------------------------------------------------------------------------
std::string WorkerFiles::get_filename_from_registered_file_id(long file_id)
{
    std::string filename;
    working_files_mutex.lock();
    std::map<std::string, FileRegistered*>::iterator it = working_files.begin();
    for (; it != working_files.end(); ++it)
    {
        if (!it->second)
            continue;

        if (it->second->file_id == file_id)
        {
            filename = it->second->filename;
            break;
        }
    }
    working_files_mutex.unlock();
    return filename;
}

//---------------------------------------------------------------------------
void WorkerFiles::get_registered_files(std::map<std::string, FileRegistered>& files)
{
    working_files_mutex.lock();
    std::map<std::string, FileRegistered*>::iterator it = working_files.begin();
    for (; it != working_files.end(); ++it)
    {
        if (!it->second)
            continue;

        files[it->first] = FileRegistered(*it->second);
    }
    working_files_mutex.unlock();
}

//---------------------------------------------------------------------------
int WorkerFiles::add_file_to_list(const std::string& file, const std::string& path,
                                  int policy, int display, int verbosity, bool fixer,
                                  bool create_policy, std::string& err)
{
    std::string full_file(path);
#ifdef WINDOWS
    if (full_file.size() && full_file[full_file.size() - 1] != '/' && full_file[full_file.size() - 1] != '\\')
        full_file += "/";
#else
    if (full_file.length())
        full_file += "/";
#endif
    full_file += file;

    bool exists = false;
    FileRegistered *fr = NULL;
    working_files_mutex.lock();
    if (working_files.find(full_file) != working_files.end() && working_files[full_file])
    {
        exists = true;
        // nothing to do
        if (!fixer && !create_policy &&
            policy == working_files[full_file]->policy && display == working_files[full_file]->display
            && verbosity == working_files[full_file]->verbosity)
        {
            working_files_mutex.unlock();
            return 0;
        }
        else
            fr = new FileRegistered;
    }
    else
        fr = new FileRegistered;
    working_files_mutex.unlock();

    // Keep the old index for the same file
    if (exists)
    {
        fr->index = working_files[full_file]->index;
        fr->file_id = working_files[full_file]->file_id;
    }
    else
        fr->index = file_index++;

    fr->filename = file;
    fr->filepath = path;
    fr->policy = policy;
    fr->display = display;
    fr->verbosity = verbosity;
    fr->create_policy = create_policy;

    working_files_mutex.lock();

    if (exists)
        delete working_files[full_file];
    working_files[full_file] = fr;
    working_files_mutex.unlock();

    unfinished_files_mutex.lock();
    unfinished_files.push_back(full_file);
    unfinished_files_mutex.unlock();

    if (!fixer && exists)
    {
        to_update_files_mutex.lock();
        to_update_files[full_file] = new FileRegistered(*fr);
        to_update_files_mutex.unlock();
        return 0;
    }

    std::vector<std::string> vec;
    vec.push_back(full_file);

    int ret;
    std::vector<long> files_id;
    if ((ret = mainwindow->analyze(vec, fixer, files_id, err)) < 0)
    {
        mainwindow->set_str_msg_to_status_bar(err);
        return -1;
    }

    if (files_id.size() != 1)
    {
        err = "Internal error: analyze result is not correct, no id returned.";
        return -1;
    }

    fr->file_id = files_id[0];

    to_add_files_mutex.lock();
    to_add_files[full_file] = new FileRegistered(*fr);
    to_add_files_mutex.unlock();

    working_files_mutex.lock();
    working_files[full_file]->file_id = files_id[0];
    working_files_mutex.unlock();

    return 0;
}

//---------------------------------------------------------------------------
void WorkerFiles::update_policy_of_file_registered_from_file(long file_id, int policy)
{
    std::string file;
    working_files_mutex.lock();
    std::map<std::string, FileRegistered*>::iterator it = working_files.begin();
    for (; it != working_files.end(); ++it)
    {
        if (!it->second || it->second->file_id != file_id)
            continue;

        file = it->first;
        break;
    }

    if (!file.size())
    {
        // file is not existing
        working_files_mutex.unlock();
        return;
    }

    working_files[file]->policy = policy;

    bool policy_valid = false;
    if (working_files[file]->analyzed && policy >= 0)
    {
        working_files_mutex.unlock();

        std::string err;
        std::vector<size_t> policies_ids;
        std::vector<std::string> policies_contents;
        std::vector<MediaConchLib::Checker_ValidateRes*> res;
        std::map<std::string, std::string> options;
        policies_ids.push_back(policy);

        if (mainwindow->validate(MediaConchLib::report_Max, file,
                                 policies_ids, policies_contents, options, res, err) == 0 && res.size() == 1)
        {
            policy_valid = res[0]->valid;
            for (size_t j = 0; j < res.size() ; ++j)
                delete res[j];
            res.clear();
        }

        working_files_mutex.lock();
    }

    working_files[file]->policy_valid = policy_valid;
    FileRegistered fr = *working_files[file];
    working_files_mutex.unlock();

    to_update_files_mutex.lock();
    to_update_files[file] = new FileRegistered(fr);
    to_update_files_mutex.unlock();
}

//---------------------------------------------------------------------------
void WorkerFiles::update_validate_of_registered_file(const std::string& file, FileRegistered* fr, const std::string& err)
{
    if (err.size())
        mainwindow->set_str_msg_to_status_bar(err);

    to_update_files_mutex.lock();
    if (to_update_files.find(file) != to_update_files.end())
    {
        delete to_update_files[file];
        to_update_files[file] = new FileRegistered(*fr);
    }
    else
        to_update_files[file] = new FileRegistered(*fr);
    to_update_files_mutex.unlock();

    working_files_mutex.lock();
    if (working_files.find(file) != working_files.end())
    {
        delete working_files[file];
        working_files[file] = fr;
    }
    else
        delete fr;
    working_files_mutex.unlock();
}

//---------------------------------------------------------------------------
void WorkerFiles::emit_signal_validate_end(const std::string& file)
{
    printf("emit signal:%s\n", file.c_str());
    Q_EMIT validate_finished(file);
}

//---------------------------------------------------------------------------
void WorkerFiles::remove_validate_thread(const std::string& file)
{
    printf("a\n");
    to_validate_files_mutex.lock();
    printf("b\n");
    std::map<std::string, WorkerFilesValidate*>::iterator it = to_validate_files.find(file);
    if (it != to_validate_files.end())
    {
        if (it->second)
        {
            if (!it->second->stop())
                current_decrease();
            delete it->second;
        }
        to_validate_files.erase(it);
    }
    to_validate_files_mutex.unlock();
    current_decrease();

    update_validate_files_registered();
}

//---------------------------------------------------------------------------
void WorkerFiles::current_decrease()
{
    current_mutex.lock();
    --current;
    current_mutex.unlock();
}

//---------------------------------------------------------------------------
void WorkerFiles::clear_files()
{
    working_files_mutex.lock();
    std::map<std::string, FileRegistered*>::iterator it = working_files.begin();

    for (; it != working_files.end(); ++it)
    {
        if (!it->second)
            continue;

        delete it->second;
    }
    working_files.clear();
    working_files_mutex.unlock();
    remove_all_registered_file_from_db();
}

//---------------------------------------------------------------------------
void WorkerFiles::update_files_registered()
{
    if (timer)
    {
        delete timer;
        timer = NULL;
    }

    update_unfinished_files();
    update_add_files_registered();
    update_delete_files_registered();
    update_update_files_registered();
    update_validate_files_registered();

    timer = new QTimer(0);
    timer->moveToThread(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_files_registered()), Qt::DirectConnection);
    timer->start(1000); //TODO
}

//---------------------------------------------------------------------------
void WorkerFiles::update_add_files_registered()
{
    std::vector<FileRegistered*> vec;
    to_add_files_mutex.lock();
    if (to_add_files.size())
    {
        std::map<std::string, FileRegistered*>::iterator it = to_add_files.begin();
        for (; it != to_add_files.end(); ++it)
            vec.push_back(it->second);
        to_add_files.clear();
    }
    to_add_files_mutex.unlock();

    if (!vec.size())
        return;

    add_registered_files_to_db(vec);
    for (size_t i = 0; i < vec.size(); ++i)
        delete vec[i];
    vec.clear();
}

//---------------------------------------------------------------------------
void WorkerFiles::update_update_files_registered()
{
    std::vector<FileRegistered*> vec;
    to_update_files_mutex.lock();
    if (to_update_files.size())
    {
        std::map<std::string, FileRegistered*>::iterator it = to_update_files.begin();
        for (; it != to_update_files.end(); ++it)
        {
            if (!it->second)
                continue;

            vec.push_back(it->second);
            if (it->second->create_policy)
                mainwindow->create_policy_from_file(it->second);
        }
        to_update_files.clear();
    }
    to_update_files_mutex.unlock();

    if (!vec.size())
        return;

    update_registered_files_in_db(vec);
    for (size_t i = 0; i < vec.size(); ++i)
        delete vec[i];
    vec.clear();
}

//---------------------------------------------------------------------------
void WorkerFiles::update_validate_files_registered()
{
    to_validate_files_mutex.lock();
    if (!to_validate_files.size())
    {
        to_validate_files_mutex.unlock();
        return;
    }

    current_mutex.lock();
    printf("max=%u, current:%u\n", max_threads, current);
    if (current >= max_threads)
    {
        current_mutex.unlock();
        to_validate_files_mutex.unlock();
        return;
    }

    std::map<std::string, WorkerFilesValidate*>::iterator it = to_validate_files.begin();
    for (; it != to_validate_files.end(); ++it)
    {
        if (it->second && !it->second->is_running)
        {
            it->second->start();
            printf("size=%lu, current:%u\n", to_validate_files.size(), current);
            ++current;
            break;
        }
    }
    current_mutex.unlock();
    to_validate_files_mutex.unlock();
}

//---------------------------------------------------------------------------
void WorkerFiles::update_delete_files_registered()
{
    std::vector<FileRegistered*> vec;

    to_delete_files_mutex.lock();
    std::map<std::string, FileRegistered*>::iterator it = to_delete_files.begin();
    for (; it != to_delete_files.end(); ++it)
        vec.push_back(it->second);
    to_delete_files.clear();
    to_delete_files_mutex.unlock();

    if (!vec.size())
        return;

    remove_registered_files_from_db(vec);
    for (size_t i = 0; i < vec.size(); ++i)
        delete vec[i];
    vec.clear();
}

//---------------------------------------------------------------------------
void WorkerFiles::update_unfinished_files()
{
    std::string err;

    unfinished_files_mutex.lock();
    std::vector<std::string> files = unfinished_files;
    unfinished_files_mutex.unlock();

    std::vector<std::string> vec;
    for (size_t i = 0; i < files.size(); ++i)
    {
        FileRegistered* fr = get_file_registered_from_file(files[i]);
        if (!fr)
        {
            vec.push_back(files[i]);
            continue;
        }

        MediaConchLib::Checker_StatusRes st_res;
        int ret = mainwindow->is_analyze_finished(files[i], st_res, err);
        if (ret < 0)
        {
            mainwindow->set_str_msg_to_status_bar(err);
            continue;
        }

        if (st_res.finished)
        {
            fr->report_kind = MediaConchLib::report_MediaConch;
            if (st_res.tool)
                fr->report_kind = *st_res.tool;

            to_validate_files_mutex.lock();
            std::map<std::string, WorkerFilesValidate*>::iterator it = to_validate_files.find(files[i]);
            if (it != to_validate_files.end())
            {
                printf("Existe deja\n");
                if (it->second)
                {
                    if (!it->second->stop())
                        current_decrease();
                    delete it->second;
                }
                to_validate_files.erase(it);
            }

            WorkerFilesValidate* v = new WorkerFilesValidate(mainwindow, this, files[i], new FileRegistered(*fr));
            to_validate_files[files[i]] = v;
            to_validate_files_mutex.unlock();
        }
        else
        {
            fr->analyzed = false;
            fr->analyze_percent = 0.0;
            if (st_res.percent)
                fr->analyze_percent = *st_res.percent;
            vec.push_back(files[i]);
        }

        to_update_files_mutex.lock();
        if (to_update_files.find(files[i]) != to_update_files.end())
        {
            delete to_update_files[files[i]];
            to_update_files[files[i]] = new FileRegistered(*fr);
        }
        else
            to_update_files[files[i]] = new FileRegistered(*fr);
        to_update_files_mutex.unlock();

        working_files_mutex.lock();
        if (working_files.find(files[i]) != working_files.end())
        {
            delete working_files[files[i]];
            working_files[files[i]] = fr;
        }
        else
            delete fr;
        working_files_mutex.unlock();
    }

    unfinished_files_mutex.lock();
    if (files.size() != unfinished_files.size())
    {
        for (size_t i = 0; i < unfinished_files.size(); ++i)
        {
            size_t j = 0;
            for (; j < files.size(); ++j)
                if (files[j] == unfinished_files[i])
                    break;
            if (j == files.size())
                vec.push_back(unfinished_files[i]);
        }
    }
    unfinished_files = vec;
    unfinished_files_mutex.unlock();
}

//---------------------------------------------------------------------------
void WorkerFiles::remove_file_registered_from_file(const std::string& file)
{
    working_files_mutex.lock();
    std::map<std::string, FileRegistered*>::iterator it = working_files.find(file);
    if (it == working_files.end() || !working_files[file])
    {
        working_files_mutex.unlock();
        return;
    }

    FileRegistered* fr = working_files[file];

    working_files[file] = NULL;
    working_files.erase(it);
    working_files_mutex.unlock();

    //stop to try to validate
    to_validate_files_mutex.lock();
    std::map<std::string, WorkerFilesValidate*>::iterator it_val = to_validate_files.find(file);
    if (it_val != to_validate_files.end() && it_val->second)
    {
        if (!it_val->second->stop())
            current_decrease();
        printf("c\n");
        if (it_val->second->isFinished())
            delete it_val->second;
        printf("d\n");
        it_val->second = NULL;

        to_validate_files.erase(it_val);
    }
    to_validate_files_mutex.unlock();

    to_delete_files_mutex.lock();
    to_delete_files[file] = fr;
    to_delete_files_mutex.unlock();
}

//---------------------------------------------------------------------------
void WorkerFiles::add_registered_file_to_db(const FileRegistered* file)
{
    if (!db)
        return;

    db->ui_add_file(file);
}

//---------------------------------------------------------------------------
void WorkerFiles::add_registered_files_to_db(const std::vector<FileRegistered*>& files)
{
    if (!db)
        return;

    for (size_t i = 0; i < files.size();)
    {
        std::vector<FileRegistered*> tmp;
        size_t j = 0;
        for (; j < 50 && i + j < files.size(); ++j)
            tmp.push_back(files[i + j]);
        db->ui_add_files(tmp);
        i += j;
    }
}

//---------------------------------------------------------------------------
void WorkerFiles::update_registered_file_in_db(const FileRegistered* file)
{
    if (!db)
        return;

    db->ui_update_file(file);
}

//---------------------------------------------------------------------------
void WorkerFiles::update_registered_files_in_db(const std::vector<FileRegistered*>& files)
{
    if (!db)
        return;

    db->ui_update_files(files);
}

//---------------------------------------------------------------------------
void WorkerFiles::remove_registered_file_from_db(const FileRegistered* file)
{
    if (!db)
        return;

    db->ui_remove_file(file);
}

//---------------------------------------------------------------------------
void WorkerFiles::remove_registered_files_from_db(const std::vector<FileRegistered*>& files)
{
    if (!db)
        return;

    db->ui_remove_files(files);
}

//---------------------------------------------------------------------------
void WorkerFiles::remove_all_registered_file_from_db()
{
    if (!db)
        return;

    db->ui_remove_all_files();
}

//---------------------------------------------------------------------------
void WorkerFiles::get_registered_file_from_db(FileRegistered* file)
{
    if (!db || !file)
        return;

    db->ui_get_file(file);
}

//---------------------------------------------------------------------------
void WorkerFiles::fill_registered_files_from_db()
{
    if (!db)
        return;

    std::vector<FileRegistered*> vec;
    db->ui_get_elements(vec);

    std::vector<std::string> files;
    working_files_mutex.lock();
    for (size_t i = 0; i < vec.size(); ++i)
    {
        std::string err2;
        QString err;
        FileRegistered *fr = vec[i];

        std::string full_file(fr->filepath);
#ifdef WINDOWS
        if (full_file.size() && full_file[full_file.size() - 1] != '/' && full_file[full_file.size() - 1] != '\\')
            full_file += "/";
#else
        if (full_file.length())
            full_file += "/";
#endif
        full_file += fr->filename;

        //check if policy still exists
        MediaConchLib::Get_Policy p;
        if (fr->policy != -1 && mainwindow->policy_get(fr->policy, "JSON", p, err) < 0)
        {
            mainwindow->set_msg_to_status_bar(err);
            fr->policy = -1;
        }

        fr->index = file_index++;
        fr->analyzed = false;

        working_files[full_file] = fr;
        files.push_back(full_file);


        std::vector<std::string> tmp;
        tmp.push_back(full_file);

        int ret;
        std::vector<long> files_id;
        if ((ret = mainwindow->analyze(tmp, false, files_id, err2)) < 0)
            mainwindow->set_str_msg_to_status_bar(err2);
        else if (!files_id.size())
            mainwindow->set_str_msg_to_status_bar("Internal error: Analyze result is not correct.");
        else
            fr->file_id = files_id[0];
    }
    working_files_mutex.unlock();

    unfinished_files_mutex.lock();
    for (size_t i = 0; i < files.size(); ++i)
        unfinished_files.push_back(files[i]);
    unfinished_files_mutex.unlock();
}

}
