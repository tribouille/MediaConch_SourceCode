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
#include <ZenLib/Ztring.h>
#include <ZenLib/Dir.h>
#include "PluginStats.h"

//---------------------------------------------------------------------------
namespace MediaConch {

    //***************************************************************************
    // Constructor/Destructor PluginStats
    //***************************************************************************

    //---------------------------------------------------------------------------
    PluginStats::PluginStats()
        : Plugin(), frame(NULL), stream_idx(0)
    {
        type = MediaConchLib::PLUGIN_STAT;
    }

    //---------------------------------------------------------------------------
    PluginStats::~PluginStats()
    {
    }

    //---------------------------------------------------------------------------
    PluginStats::PluginStats(const PluginStats& p) : Plugin(p)
    {
        frame = p.frame;
        stats_dir = p.stats_dir;
    }

    //---------------------------------------------------------------------------
    int PluginStats::load_plugin(const std::map<std::string, Container::Value>& obj, std::string&)
    {
        if (obj.find("statsDir") != obj.end() && obj.at("statsDir").type == Container::Value::CONTAINER_TYPE_STRING)
        {
            stats_dir = obj.at("statsDir").s;

            ZenLib::Ztring z_path = ZenLib::Ztring().From_UTF8(stats_dir);
            if (!ZenLib::Dir::Exists(z_path))
                ZenLib::Dir::Create(z_path);

            if (!ZenLib::Dir::Exists(z_path))
                stats_dir = "";
        }

        if (!stats_dir.size())
        {
            if (create_report_dir("MediaConchStats", "StatsDir", stats_dir) < 0)
                stats_dir = ".";
        }

        return 0;
    }

}
