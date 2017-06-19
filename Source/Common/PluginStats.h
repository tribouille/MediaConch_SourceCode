/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Scheduler functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef PLUGINSTATSH
#define PLUGINSTATSH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "Plugin.h"
#include "StatsFrame.h"

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// Class PluginStats
//***************************************************************************

class PluginStats : public Plugin
{
public:
    PluginStats();
    virtual ~PluginStats();
    PluginStats(const PluginStats& p);

    virtual int                            load_plugin(const std::map<std::string, Container::Value>& obj, std::string& error);

    void                                   set_frame(MediaInfo_Event_Global_FrameContent_0 *f) { frame = f; }
    void                                   set_stream_idx(size_t s) { stream_idx = s; }

    std::string                            get_stats_dir() const { return stats_dir; }
    const                                  std::vector<StatsFrame*>& get_stats() { return stats; }

protected:
    // Configuration
    std::string                            stats_dir;

    // video components
    std::vector<StatsFrame*>               stats;

    // Data
    MediaInfo_Event_Global_FrameContent_0 *frame;
    size_t                                 stream_idx;

private:
    PluginStats &operator=(const PluginStats&);
};

}

#endif // !PLUGINSTATSH
