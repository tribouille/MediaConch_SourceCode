/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifdef HAVE_SQLITE

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "SQLLiteStats.h"
#include "Core.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// SQLLiteStats
//***************************************************************************

int SQLLiteStats::current_stats_version = 1;

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
SQLLiteStats::SQLLiteStats() : DatabaseStats(), SQLLite()
{
}

//---------------------------------------------------------------------------
SQLLiteStats::~SQLLiteStats()
{
}

//---------------------------------------------------------------------------
int SQLLiteStats::init()
{
    return SQLLite::init(db_dirname, db_filename);
}

//---------------------------------------------------------------------------
int SQLLiteStats::init_stats()
{
    if (init() < 0)
        return -1;
    if (get_db_version(stats_version) < 0)
        return -1;
    return create_stats_table();
}

//---------------------------------------------------------------------------
int SQLLiteStats::create_stats_table()
{
    if (stats_version == 0)
    {
        get_sql_query_for_create_stats_table(query);

        const char* end = NULL;
        int ret = sqlite3_prepare_v2(db, query.c_str(), query.length() + 1, &stmt, &end);
        if (ret != SQLITE_OK || !stmt || (end && *end))
            return -1;
        ret = execute();
        if (ret < 0)
            return -1;
    }

    return update_stats_table();
}

//---------------------------------------------------------------------------
int SQLLiteStats::update_stats_table()
{
    const char* end = NULL;
    int ret = 0;

#define UPDATE_STATS_TABLE_FOR_VERSION(version)                         \
    do                                                                  \
    {                                                                   \
        end = NULL;                                                     \
        if (stats_version > version)                                    \
            continue;                                                   \
        get_sql_query_for_update_stats_table_v##version(query);         \
        if (!query.length())                                            \
            continue;                                                   \
                                                                        \
        const char* q = query.c_str();                                  \
        end = NULL;                                                     \
        while (1)                                                       \
        {                                                               \
            ret = sqlite3_prepare_v2(db, q, -1, &stmt, &end);           \
            if (version > 1 && (ret != SQLITE_OK || !stmt))             \
                return -1;                                              \
            ret = execute();                                            \
                                                                        \
            if (version > 1 && ret < 0)                                 \
                return ret;                                             \
            if (!end || !*end)                                          \
                break;                                                  \
                                                                        \
            q = end;                                                    \
        }                                                               \
                                                                        \
    } while(0);

    // UPDATE_STATS_TABLE_FOR_VERSION(0);

#undef UPDATE_STATS_TABLE_FOR_VERSION

    return set_db_version(current_stats_version);
}

//---------------------------------------------------------------------------
int SQLLiteStats::register_stats(long user_id, long file_id, MediaConchLib::StatsFrameMin& stats, std::string& err)
{
    std::stringstream create;

    reports.clear();
    create << "INSERT INTO MEDIACONCH_STATS";
    create << " (USER, FILE_ID, STREAM, PLUGIN, DATA_NAME, DATA, DATA_COMPRESSED, DATA_SIZE)";
    create << " VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    query = create.str();

    if (prepare_v2(query, err) < 0)
        return -1;

    int ret = sqlite3_bind_int(stmt, 1, user_id);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = sqlite3_bind_int(stmt, 2, file_id);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = sqlite3_bind_int(stmt, 3, stats.stream);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = sqlite3_bind_blob(stmt, 4, stats.plugin.c_str(), stats.plugin.length(), SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = sqlite3_bind_blob(stmt, 5, stats.name.c_str(), stats.name.size(), SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ZenLib::int8u* data = NULL;
    size_t orig_len = 0;
    size_t len = 0;
    MediaConchLib::compression compress = MediaConchLib::compression_ZLib;

    if (serialize_stats(stats.stats, data, len, orig_len, compress, err) < 0)
        return -1;

    ret = sqlite3_bind_blob(stmt, 6, data, len, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = sqlite3_bind_int(stmt, 7, (int)compress);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = sqlite3_bind_int(stmt, 8, orig_len);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = execute();
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    return 0;
}

//---------------------------------------------------------------------------
int SQLLiteStats::get_stats(long user_id, long file_id, std::vector<MediaConchLib::StatsFrameMin*>& stats,
                            std::string& err)
{
    std::stringstream create;
    reports.clear();
    create << "SELECT STREAM, PLUGIN, DATA_NAME, DATA, DATA_SIZE, DATA_COMPRESSED FROM MEDIACONCH_STATS";
    create << " WHERE USER = ? AND FILE_ID = ?;";
    query = create.str();

    if (prepare_v2(query, err) < 0)
        return -1;

    int ret = sqlite3_bind_int(stmt, 1, user_id);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    ret = sqlite3_bind_int(stmt, 2, file_id);
    if (ret != SQLITE_OK)
    {
        err = get_sqlite_error(ret);
        return -1;
    }

    if (execute())
    {
        err = error;
        return -1;
    }

    for (size_t i = 0; i < reports.size(); ++i)
    {
        MediaConchLib::StatsFrameMin* fr = new MediaConchLib::StatsFrameMin;
        size_t orig_len = 0;
        std::string data;
        MediaConchLib::compression compress = MediaConchLib::compression_None;

        if (reports[i].find("STREAM") != reports[i].end())
            fr->stream = std_string_to_uint(reports[i]["STREAM"]);

        if (reports[i].find("PLUGIN") != reports[i].end())
            fr->plugin = reports[i]["PLUGIN"];

        if (reports[i].find("DATA_NAME") != reports[i].end())
            fr->name = reports[i]["DATA_NAME"];

        if (reports[i].find("DATA") != reports[i].end())
            data = reports[i]["DATA"];

        if (reports[i].find("DATA_SIZE") != reports[i].end())
            orig_len = std_string_to_uint(reports[i]["DATA_SIZE"]);

        if (reports[i].find("DATA_COMPRESSED") != reports[i].end())
            compress = (MediaConchLib::compression)std_string_to_int(reports[i]["DATA_COMPRESSED"]);

        parse_stats((const ZenLib::int8u*)data.c_str(), data.size(), orig_len, compress, fr->stats, error);
        stats.push_back(fr);
    }
    return 0;
}

//---------------------------------------------------------------------------
int SQLLiteStats::serialize_stats(const std::vector<double>& stats, ZenLib::int8u*& data, size_t& len,
                                  size_t& orig_len, MediaConchLib::compression& compress, std::string&)
{
    len = stats.size() * sizeof(double);
    data = new ZenLib::int8u[len];
    for (size_t i = 0; i < stats.size(); ++i)
        std::memcpy(data + (i * sizeof(double)), (const void*)&stats[i], sizeof(double));

    ZenLib::int8u* tmp = NULL;
    orig_len = len;
    if (!Core::compress_buffer(data, orig_len, tmp, len, compress))
    {
        delete data;
        data = tmp;
    }
    return 0;
}

//---------------------------------------------------------------------------
int SQLLiteStats::parse_stats(const ZenLib::int8u* data, size_t len, size_t orig_len,
                              MediaConchLib::compression& compress,
                              std::vector<double>& stats, std::string& error)
{
    ZenLib::int8u* vals = NULL;
    size_t dst_len = 0;

    if (Core::uncompress_buffer(data, len, vals, dst_len, orig_len, compress) < 0)
    {
        error = "Error to uncompress the data buffer.";
        return -1;
    }

    if (compress != MediaConchLib::compression_ZLib)
    {
        vals = new ZenLib::int8u[len];
        std::memcpy(vals, data, len);
        dst_len = len;
    }

    size_t nb_val = dst_len / sizeof(double);

    for (size_t i = 0; i < nb_val; ++i)
    {
        double tmp = 0;
        std::memcpy(&tmp, vals + (i * sizeof(double)), sizeof(double));
        stats.push_back(tmp);
    }

    delete [] vals;

    return 0;
}

}

#endif
