/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#ifndef CHECKERWINDOW_H
#define CHECKERWINDOW_H

#include "mainwindow.h"

#include <QFileInfo>
#include <QString>

class QFrame;
class QFile;
class QWebView;

namespace MediaConch {

class MainWindow;
class WebView;
class ProgressBar;

class CheckerWindow : public QObject
{
    Q_OBJECT

public:
    explicit CheckerWindow(MainWindow *parent = 0);
    ~CheckerWindow();

    // Functions
    void                        create_web_view();
    void                        update_web_view(std::string file, int policy);
    void                        update_web_view(QList<QFileInfo>& files, int policy);
    void                        set_display_xslt(const QString& d) { display_xslt = d; }
    void                        reset_display_xslt() { display_xslt.clear(); }
    void                        change_local_files(QStringList& files);

    // Helpers
    void                        checker_add_file(const QString& file, int policy);
    void                        checker_add_files(QList<QFileInfo>& file, int policy);
    void                        checker_add_policy_file(const QString& file, QString& policy);
    void                        checker_add_policy_files(QList<QFileInfo>& file, QString& policy);
    bool                        is_analyzes_done();

private:
    // Visual elements
    MainWindow*                 mainwindow;
    WebView*                    MainView;
    ProgressBar*                progressBar;
    QString                     display_xslt;
    bool                        analyse;
    unsigned int                result_index;

    void                        clearVisualElements();
    void                        set_web_view_content(QString& html);
    void                        set_error_http(MediaConchLib::errorHttp error);

//***************************************************************************
// HELPER
//***************************************************************************

    void remove_template_tags(QString& data);
    void load_include_in_template(QString& html);
    void remove_element_in_template(QString& html);
    void load_form_in_template(QString& html);
    QString create_html();
    QString create_html_base(QString& body);
    QString create_html_body();
    QString create_form_upload();
    QString create_form_online();
    QString create_form_repository();
    void add_script_js_tree(std::string& file);
    void remove_form_online(int pos, QString& html);
    void change_collapse_form(QString& html);
    void change_body_in_template(QString& body, QString& html);
    void add_policy_to_form_selection(QString& policies, QString& form, const char *selector);
    void create_policy_options(QString& policies);
    void add_display_to_form_selection(QString& displays, QString& form, const char *selector);
    void create_displays_options(QString& displays);
#if (QT_VERSION >= 0x050600)
    void add_file_detail_to_html(std::string& file, int policy);
#else
    void add_file_detail_to_html(std::string& file, int policy, QString& html);
#endif
    void create_html_file_detail(std::string& file, int policy, QString& html);
    void change_html_file_detail(QString& html, std::string& file);
    void change_html_file_detail_inform_xml(QString& html, std::string& file);
    void change_html_file_detail_conformance(QString& html, std::string& file);
    void change_html_file_detail_policy_report(QString& html, std::string& file, int policy);
    void change_html_file_detail_trace(QString& html, std::string& file);
    void remove_html_file_detail_policy_report(QString& html);
    bool report_is_html(QString &report);
    bool report_is_xml(QString &report);
    bool is_policy_html_valid(QString &report);
    void change_report_policy_save_name(std::string& file, QString& ext, QString& html);
    QString file_remove_ext(std::string& file);
    void get_displays_use(std::string& display_name, std::string& display_content);

public Q_SLOTS:
    void actionCloseAllTriggered();
private Q_SLOTS:
    void createWebViewFinished(bool ok);
};

}

#endif // MAINWINDOW_H
