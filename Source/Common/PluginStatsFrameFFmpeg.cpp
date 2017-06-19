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

#ifdef HAVE_FFMPEG
//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include "PluginStatsFrameFFmpeg.h"

extern "C"
{
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/pixdesc.h>
}

//---------------------------------------------------------------------------
namespace MediaConch {

    //***************************************************************************
    // Constructor/Destructor PluginStatsFrameFFmpeg
    //***************************************************************************

    //---------------------------------------------------------------------------
    PluginStatsFrameFFmpeg::PluginStatsFrameFFmpeg()
        : PluginStats(), av_frame(NULL), filter_graph(NULL),
          filter_src(NULL), filter_sink(NULL),
          filter_src_ctx(NULL), filter_sink_ctx(NULL)
    {
        type = MediaConchLib::PLUGIN_STAT;

        av_register_all();
        avfilter_register_all();
        avcodec_register_all();
        av_log_set_level(AV_LOG_QUIET);
    }

    //---------------------------------------------------------------------------
    PluginStatsFrameFFmpeg::~PluginStatsFrameFFmpeg()
    {
    }

    //---------------------------------------------------------------------------
    PluginStatsFrameFFmpeg::PluginStatsFrameFFmpeg(const PluginStatsFrameFFmpeg& v): PluginStats(v)
    {
        filter_descr = v.filter_descr;
    }

    //---------------------------------------------------------------------------
    int PluginStatsFrameFFmpeg::load_plugin(const std::map<std::string, Container::Value>& obj, std::string&)
    {
        PluginStats::load_plugin(obj, error);

        if (obj.find("filter") != obj.end() && obj.at("filter").type == Container::Value::CONTAINER_TYPE_STRING)
            filter_descr = obj.at("filter").s;
        else
            filter_descr = "signalstats=stat=tout+vrep+brng,split[a][b];[a]field=top[a1];[b]field=bottom[b1];[a1][b1]psnr";

        return 0;
    }

    //---------------------------------------------------------------------------
    int PluginStatsFrameFFmpeg::init(std::string& error)
    {
        if (!frame || !frame->Planes_Size)
        {
            error = "The frame is not valid";
            return -1;
        }

        av_frame = av_frame_alloc();
        if (!av_frame)
        {
            error = "Cannot allocate av frame";
            return -1;
        }

        return 0;
    }

    //---------------------------------------------------------------------------
    int PluginStatsFrameFFmpeg::run(std::string& error)
    {
        if (init(error) < 0)
            return -1;

        if (add_filters(error) < 0)
            return -1;

        return 0;
    }

    int PluginStatsFrameFFmpeg::init_filters(const std::string& filters_descr, std::string& error)
    {
        if (!frame->Planes_Size)
        {
            error = "No plane in this frame";
            return -1;
        }

        // Init AV stuff
        AVFilterInOut *outputs = avfilter_inout_alloc();
        AVFilterInOut *inputs  = avfilter_inout_alloc();

        filter_graph = avfilter_graph_alloc();
        filter_src  = avfilter_get_by_name("buffer");
        filter_sink = avfilter_get_by_name("buffersink");

        std::stringstream ss;
        ss << "video_size=" << frame->Planes[0]->Width << "x" << frame->Planes[0]->Height;

        size_t pix_fmt = 298; //AV_PIX_FMT_BGR0
        ss << ":pix_fmt=" << pix_fmt;
        ss << ":time_base=" << 1 << "/" << 1;
        ss << ":pixel_aspect=" << 0 << "/" << 1;

        int ret;
        if ((ret = avfilter_graph_create_filter(&filter_src_ctx, filter_src, "in",
                                                ss.str().c_str(), NULL, filter_graph)) < 0)
        {
            error = "Cannot create filter source buffer.";
            return -1;
        }

        if (avfilter_graph_create_filter(&filter_sink_ctx, filter_sink, "out",
                                         NULL, NULL, filter_graph) < 0)
        {
            error = "Cannot create filter sink buffer.";
            return -1;
        }

        /* Endpoints for the filter graph. */
        outputs->name       = av_strdup("in");
        outputs->filter_ctx = filter_src_ctx;
        outputs->pad_idx    = 0;
        outputs->next       = NULL;
        inputs->name       = av_strdup("out");
        inputs->filter_ctx = filter_sink_ctx;
        inputs->pad_idx    = 0;
        inputs->next       = NULL;

        if (avfilter_graph_parse_ptr(filter_graph, filters_descr.c_str(),
                                     &inputs, &outputs, NULL) < 0)
        {
            error = "error using avfilter_graph_parse_ptr.";
            return -1;
        }

        if (avfilter_graph_config(filter_graph, NULL) < 0)
        {
            error = "error using avfilter_graph_config.";
            return -1;
        }

        return 0;
    }

    //---------------------------------------------------------------------------
    int PluginStatsFrameFFmpeg::add_filters(std::string& error)
    {
        if (init_filters(filter_descr, error) < 0)
            return -1;

        av_frame->pts = frame->PTS;
        av_frame->pkt_dts = frame->DTS;
        av_frame->data[0] = frame->Planes[0]->Buffer;
        av_frame->linesize[0] = frame->Planes[0]->AllBytesPerLine();
        av_frame->width = frame->Planes[0]->Width;
        av_frame->height = frame->Planes[0]->Height;
        av_frame->format = 298; //AV_PIX_FMT_BGR0

        /* push the decoded frame into the filtergraph */
        if (av_buffersrc_add_frame_flags(filter_src_ctx, av_frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
        {
            error = "Error while feeding the filtergraph.";
            return -1;
        }

        int ret = 0;
        AVFrame *filt_frame = av_frame_alloc();
        while (1)
        {
            ret = av_buffersink_get_frame(filter_sink_ctx, filt_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;

            if (ret < 0)
            {
                error = "av_buffersink_get_frame error.";
                return -1;
            }

            register_frame_stats(filt_frame);
            av_frame_unref(filt_frame);
        }

        return 0;
    }

    //---------------------------------------------------------------------------
    int PluginStatsFrameFFmpeg::register_frame_stats(struct AVFrame* fram)
    {
        AVDictionary *dico = av_frame_get_metadata(fram);

        StatsFrame *stat = new StatsFrame;
        stat->stream_idx = stream_idx;
        // stat->duration = duration;

        AVDictionaryEntry *entry = NULL;
        for (;;)
        {
            entry = av_dict_get(dico, "", entry, AV_DICT_IGNORE_SUFFIX);
            if (!entry)
                break;

            if (!entry->key)
                continue;

            if (stat->stats_dico_name.find(entry->key) == stat->stats_dico_name.end())
                continue;

            char* end = NULL;
            double value = strtod(entry->value, &end);
            // printf("key=%s\n", entry->key);
            // printf("value=%f\n", value);

            //     // Special cases: crop: x2, y2
            //     if (string(e->key)=="lavfi.cropdetect.x2")
            //         y[j][x_Current]=Width-value;
            //     else if (string(e->key)=="lavfi.cropdetect.y2")
            //         y[j][x_Current]=Height-value;
            //     else if (string(e->key)=="lavfi.cropdetect.w")
            //         y[j][x_Current]=Width-value;
            //     else if (string(e->key)=="lavfi.cropdetect.h")
            //         y[j][x_Current]=Height-value;
            //     else
            //         y[j][x_Current]=value;

            stat->stats_dico_name[entry->key] = value;

            //     if (PerItem[j].Group1!=Group_VideoMax && y_Max[PerItem[j].Group1]<y[j][x_Current])
            //         y_Max[PerItem[j].Group1]=y[j][x_Current];
            //     if (PerItem[j].Group2!=Group_VideoMax && y_Max[PerItem[j].Group2]<y[j][x_Current])
            //         y_Max[PerItem[j].Group2]=y[j][x_Current];
            //     if (PerItem[j].Group1!=Group_VideoMax && y_Min[PerItem[j].Group1]>y[j][x_Current])
            //         y_Min[PerItem[j].Group1]=y[j][x_Current];
            //     if (PerItem[j].Group2!=Group_VideoMax && y_Min[PerItem[j].Group2]>y[j][x_Current])
            //         y_Min[PerItem[j].Group2]=y[j][x_Current];

            //     //Stats
            //     Stats_Totals[j]+=y[j][x_Current];
            //     if (PerItem[j].DefaultLimit!=DBL_MAX)
            //     {
            //         if (y[j][x_Current]>PerItem[j].DefaultLimit)
            //             Stats_Counts[j]++;
            //         if (PerItem[j].DefaultLimit2!=DBL_MAX && y[j][x_Current]>PerItem[j].DefaultLimit2)
            //             Stats_Counts2[j]++;
            //     }
            // }
        }

        // y[Item_pkt_duration_time][x_Current] = stat->durations;

        // {
        //     double& group1Max = y_Max[PerItem[Item_pkt_duration_time].Group1];
        //     double& group1Min = y_Min[PerItem[Item_pkt_duration_time].Group1];
        //     double& current = durations[x_Current];

        //     if(group1Max < current)
        //         group1Max = current;
        //     if(group1Min > current)
        //         group1Min = current;
        // }

        stat->key_frame = fram->key_frame ? true : false;

        stat->pkt_pos = fram->pkt_pos;
        stat->pkt_size = fram->pkt_size;
        stat->pkt_pts = fram->pkt_pts;

        // y[Item_pkt_size][x_Current] = pkt_size[x_Current];

        // {
        //     double& group1Max = y_Max[PerItem[Item_pkt_size].Group1];
        //     double& group1Min = y_Min[PerItem[Item_pkt_size].Group1];
        //     int& current = pkt_size[x_Current];

        //     if(group1Max < current)
        //         group1Max = current;
        //     if(group1Min > current)
        //         group1Min = current;
        // }

        stat->pix_fmt = fram->format;
        stat->pict_type_char = av_get_picture_type_char(fram->pict_type);

        // if (x_Max[0]<=x[0][x_Current])
        // {
        //     x_Max[0]=x[0][x_Current];
        //     x_Max[1]=x[1][x_Current];
        //     x_Max[2]=x[2][x_Current];
        //     x_Max[3]=x[3][x_Current];
        // }
        // x_Current++;
        // if (x_Current_Max<=x_Current)
        //     x_Current_Max=x_Current;

        stats.push_back(stat);
        return 0;
    }

}

#endif //!HAVE_FFMPEG
