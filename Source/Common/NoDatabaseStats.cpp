/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "NoDatabaseStats.h"
#include <sstream>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// NoDatabaseStats
//***************************************************************************

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
NoDatabaseStats::NoDatabaseStats() : DatabaseStats()
{
}

//---------------------------------------------------------------------------
NoDatabaseStats::~NoDatabaseStats()
{
}

//---------------------------------------------------------------------------
int NoDatabaseStats::init()
{
    return 0;
}

//---------------------------------------------------------------------------
int NoDatabaseStats::init_stats()
{
    return init();
}

//---------------------------------------------------------------------------
int NoDatabaseStats::execute()
{
    return 0;
}

//---------------------------------------------------------------------------
int NoDatabaseStats::create_stats_table()
{
    return 0;
}

//---------------------------------------------------------------------------
int NoDatabaseStats::update_stats_table()
{
    return 0;
}

//---------------------------------------------------------------------------
int NoDatabaseStats::register_stats(long user_id, long file_id,
                                    MediaConchLib::StatsFrameMin& s, std::string&)
{
    stats[user_id][file_id].push_back(new MediaConchLib::StatsFrameMin(s));
    return 0;
}

//---------------------------------------------------------------------------
int NoDatabaseStats::get_stats(long user_id, long file_id, std::vector<MediaConchLib::StatsFrameMin*>& s,
                               std::string& error)
{
    if (stats.find(user_id) == stats.end())
    {
        error = "Cannot find user for these stats.";
        return -1;
    }

    if (stats[user_id].find(file_id) == stats[user_id].end())
    {
        error = "Cannot find this file for these stats.";
        return -1;
    }

    s = stats[user_id][file_id];
    return 0;
}

}
