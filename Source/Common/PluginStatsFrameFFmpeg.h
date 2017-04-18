/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Stats frame FFMPEG functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef PLUGINSTATSFRAMEFFMPEGH
#define PLUGINSTATSFRAMEFFMPEGH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "Plugin.h"
#include "PluginStats.h"
#include "StatsFrame.h"

#ifdef HAVE_FFMPEG

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfiltergraph.h>

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif
}

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// Class PluginStatsFrameFFmpeg
//***************************************************************************

class PluginStatsFrameFFmpeg : public PluginStats
{
public:
    PluginStatsFrameFFmpeg();
    virtual ~PluginStatsFrameFFmpeg();
    PluginStatsFrameFFmpeg(const PluginStatsFrameFFmpeg&);

    virtual int  load_plugin(const std::map<std::string, Container::Value>& obj, std::string& error);
    virtual int  run(std::string& error);

    void         set_pix_fmt(int px) { pix_fmt = (AVPixelFormat)px; }

private:
    PluginStatsFrameFFmpeg&    operator=(const PluginStatsFrameFFmpeg&);

    int          init(std::string& error);
    int          init_filters(const std::string& filters_descr, std::string& error);
    int          add_filters(std::string& error);
    int          register_frame_stats(struct AVFrame* Frame);
    void         register_stats_in_db();

    std::string                filter_descr;
    std::string                stats_dir;

    AVPixelFormat              pix_fmt;
    AVFrame                   *av_frame;
    AVFilterGraph             *filter_graph;
    AVFilter                  *filter_src;
    AVFilter                  *filter_sink;
    AVFilterContext           *filter_src_ctx;
    AVFilterContext           *filter_sink_ctx;
};

}

#else

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// Class PluginStatsFrameFFmpeg
//***************************************************************************

class PluginStatsFrameFFmpeg : public PluginStats
{
public:
    PluginStatsFrameFFmpeg() {}
    virtual ~PluginStatsFrameFFmpeg() {}
    PluginStatsFrameFFmpeg(const PluginStatsFrameFFmpeg&) {}

    virtual int  load_plugin(const std::map<std::string, Container::Value>&, std::string& error)
    {
        error = "MediaConch is not compiled with FFmpeg support.";
        return -1;
    }

    virtual int  run(std::string& error)
    {
        error = "MediaConch is not compiled with FFmpeg support.";
        return -1;
    }

    void         set_pix_fmt(int) { }

private:
    PluginStatsFrameFFmpeg&    operator=(const PluginStatsFrameFFmpeg&);
};

}

#endif // !HAVE_LIBEVENT


#endif // !PLUGINSTATSFRAMEFFMPEGH
