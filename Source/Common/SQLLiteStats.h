/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// SQLLite functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef HAVE_SQLITE

//---------------------------------------------------------------------------
#ifndef SQLLiteStatsH
#define SQLLiteStatsH
//---------------------------------------------------------------------------

#include <MediaInfo/Setup.h>
#include "DatabaseStats.h"
#include "SQLLite.h"
#include <sqlite3.h>
//---------------------------------------------------------------------------

namespace MediaConch {

//***************************************************************************
// Class SQLLiteStats
//***************************************************************************

class SQLLiteStats : public DatabaseStats, public SQLLite
{
public:
    //Constructor/Destructor
    SQLLiteStats();
    virtual ~SQLLiteStats();

    //Init
    virtual int create_stats_table();
    virtual int update_stats_table();

    // Stats
    virtual int register_stats(long user_id, long file_id,
                               MediaConchLib::StatsFrameMin& stats, std::string& error);
    virtual int get_stats(long user_id, long file_id, std::vector<MediaConchLib::StatsFrameMin*>& stats,
                          std::string& error);

protected:
    virtual int init();
    virtual int init_stats();

private:
    int           stats_version;
    static int    current_stats_version;

    int           serialize_stats(const std::vector<double>& stats, ZenLib::int8u*& data, size_t& len,
                                  size_t& orig_len, MediaConchLib::compression& compress, std::string& error);
    int           parse_stats(const ZenLib::int8u* data, size_t len, size_t orig_len, MediaConchLib::compression& compress,
                              std::vector<double>& stats, std::string& error);

    SQLLiteStats (const SQLLiteStats&);
    SQLLiteStats& operator=(const SQLLiteStats&);
};

}

#endif /* !SQLLiteStatsH */

#endif /* !HAVE_SQLITE */
