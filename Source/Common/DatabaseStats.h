/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// DatabaseStats functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef DatabaseStatsH
#define DatabaseStatsH
//---------------------------------------------------------------------------

#include <map>
#include <vector>
#include "MediaConchLib.h"
#include "Database.h"
#include "StatsFrame.h"
//---------------------------------------------------------------------------

namespace MediaConch {

//***************************************************************************
// Class Database
//***************************************************************************

class DatabaseStats : public Database
{
public:
    //Constructor/Destructor
    DatabaseStats();
    virtual ~DatabaseStats();

    void        set_database_directory(const std::string& dirname);
    void        set_database_filename(const std::string& name);

    // Init
    virtual int create_stats_table() = 0;
    virtual int update_stats_table() = 0;
    virtual int init_stats() = 0;

    virtual int register_stats(long user_id, long file_id,
                               MediaConchLib::StatsFrameMin& stats, std::string& error) = 0;
    virtual int get_stats(long user_id, long file_id, std::vector<MediaConchLib::StatsFrameMin*>& stats,
                          std::string& error) = 0;

protected:
    // Database dependant
    void        get_sql_query_for_create_stats_table(std::string& q);

private:
    DatabaseStats (const DatabaseStats&);
    DatabaseStats& operator=(const DatabaseStats&);
};

}

#endif
