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

    virtual int  load_plugin(const std::map<std::string, Container::Value>& obj, std::string& error);

    void         set_frame(const std::string& f) { frame = f; }
    void         set_width(size_t s) { width = s; }
    void         set_height(size_t s) { height = s; }
    void         set_stream_idx(size_t s) { stream_idx = s; }
    void         set_duration(double s) { duration = s; }
    void         set_time_base_num(size_t s) { time_base_num = s; }
    void         set_time_base_den(size_t s) { time_base_den = s; }
    void         set_sar_num(size_t s) { sar_num = s; }
    void         set_sar_den(size_t s) { sar_den = s; }

    std::string  get_stats_dir() { return stats_dir; }
    const std::vector<StatsFrame*>& get_stats() { return stats; }

protected:
    // Configuration
    std::string                stats_dir;

    // Data
    std::string  frame;

    // video components
    size_t                    width;
    size_t                    height;
    size_t                    stream_idx;
    double                    duration;
    size_t                    time_base_num;
    size_t                    time_base_den;
    size_t                    sar_num;
    size_t                    sar_den;

    std::vector<StatsFrame*>  stats;

private:
    PluginStats &operator=(const PluginStats&);
};

}

#endif // !PLUGINSTATSH
