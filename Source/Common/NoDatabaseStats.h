/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// NoDatabase functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef NODATABASESTATSH
#define NODATABASESTATSH

//---------------------------------------------------------------------------
#include "DatabaseStats.h"

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// Class NoDatabaseStats
//***************************************************************************

class NoDatabaseStats : public DatabaseStats
{
public:
    //Constructor/Destructor
    NoDatabaseStats();
    virtual ~NoDatabaseStats();

    virtual int init();
    virtual int init_stats();

    //Init
    virtual int create_stats_table();
    virtual int update_stats_table();

    //Stats
    virtual int register_stats(long user_id, long file_id,
                               MediaConchLib::StatsFrameMin& stats, std::string& error);
    virtual int get_stats(long user_id, long file_id, std::vector<MediaConchLib::StatsFrameMin*>& stats,
                          std::string& error);

protected:
    virtual int  execute();

private:
    NoDatabaseStats (const NoDatabaseStats&);
    NoDatabaseStats& operator=(const NoDatabaseStats&);

    std::map<long, std::map<long, std::vector<MediaConchLib::StatsFrameMin*> > > stats;
};

}

#endif /* !NODATABASESTATS */
