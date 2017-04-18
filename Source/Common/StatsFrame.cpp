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
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include "StatsFrame.h"

//---------------------------------------------------------------------------
namespace MediaConch {

    //***************************************************************************
    // Constructor/Destructor StatsFrame
    //***************************************************************************

    //---------------------------------------------------------------------------
    StatsFrame::StatsFrame()
        : key_frame(false), stream_idx(0),
          pkt_pos(-1), pkt_pts(-1), pkt_size(0),
          pix_fmt(0), pict_type_char(-1), duration(0.0)
    {
        fill_dico_stats_names();
    }

    //---------------------------------------------------------------------------
    StatsFrame::~StatsFrame()
    {
    }

    //---------------------------------------------------------------------------
    StatsFrame::StatsFrame(const StatsFrame& s)
        : key_frame(s.key_frame), stream_idx(s.stream_idx),
          pkt_pos(s.pkt_pos), pkt_pts(s.pkt_pts), pkt_size(s.pkt_size),
          pix_fmt(s.pix_fmt), pict_type_char(s.pict_type_char), duration(s.duration)
    {
        std::map<std::string, double>::const_iterator it = s.stats_dico_name.begin();

        for (; it != s.stats_dico_name.end(); ++it)
            stats_dico_name[it->first] = it->second;
    }

    //---------------------------------------------------------------------------
    void StatsFrame::fill_dico_stats_names()
    {
        stats_dico_name["lavfi.signalstats.YMIN"] = 0.0;
        stats_dico_name["lavfi.signalstats.YLOW"] = 0.0;
        stats_dico_name["lavfi.signalstats.YAVG"] = 0.0;
        stats_dico_name["lavfi.signalstats.YHIGH"] = 0.0;
        stats_dico_name["lavfi.signalstats.YMAX"] = 0.0;

        stats_dico_name["lavfi.signalstats.UMIN"] = 0.0;
        stats_dico_name["lavfi.signalstats.ULOW"] = 0.0;
        stats_dico_name["lavfi.signalstats.UAVG"] = 0.0;
        stats_dico_name["lavfi.signalstats.UHIGH"] = 0.0;
        stats_dico_name["lavfi.signalstats.UMAX"] = 0.0;

        stats_dico_name["lavfi.signalstats.VMIN"] = 0.0;
        stats_dico_name["lavfi.signalstats.VLOW"] = 0.0;
        stats_dico_name["lavfi.signalstats.VAVG"] = 0.0;
        stats_dico_name["lavfi.signalstats.VHIGH"] = 0.0;
        stats_dico_name["lavfi.signalstats.VMAX"] = 0.0;

        stats_dico_name["lavfi.signalstats.VDIF"] = 0.0;
        stats_dico_name["lavfi.signalstats.UDIF"] = 0.0;
        stats_dico_name["lavfi.signalstats.YDIF"] = 0.0;

        stats_dico_name["lavfi.signalstats.SATMIN"] = 0.0;
        stats_dico_name["lavfi.signalstats.SATLOW"] = 0.0;
        stats_dico_name["lavfi.signalstats.SATAVG"] = 0.0;
        stats_dico_name["lavfi.signalstats.SATHIGH"] = 0.0;
        stats_dico_name["lavfi.signalstats.SATMAX"] = 0.0;

        stats_dico_name["lavfi.signalstats.HUEMED"] = 0.0;
        stats_dico_name["lavfi.signalstats.HUEAVG"] = 0.0;

        stats_dico_name["lavfi.signalstats.TOUT"] = 0.0;
        stats_dico_name["lavfi.signalstats.VREP"] = 0.0;
        stats_dico_name["lavfi.signalstats.BRNG"] = 0.0;

        stats_dico_name["lavfi.cropdetect.x1"] = 0.0;
        stats_dico_name["lavfi.cropdetect.x2"] = 0.0;
        stats_dico_name["lavfi.cropdetect.y1"] = 0.0;
        stats_dico_name["lavfi.cropdetect.y2"] = 0.0;
        stats_dico_name["lavfi.cropdetect.w"] = 0.0;
        stats_dico_name["lavfi.cropdetect.h"] = 0.0;

        stats_dico_name["lavfi.psnr.mse.v"] = 0.0;
        stats_dico_name["lavfi.psnr.mse.u"] = 0.0;
        stats_dico_name["lavfi.psnr.mse.y"] = 0.0;

        stats_dico_name["lavfi.psnr.psnr.v"] = 0.0;
        stats_dico_name["lavfi.psnr.psnr.u"] = 0.0;
        stats_dico_name["lavfi.psnr.psnr.y"] = 0.0;

        stats_dico_name["lavfi.ssim.All"] = 0.0;
        stats_dico_name["lavfi.ssim.V"] = 0.0;
        stats_dico_name["lavfi.ssim.U"] = 0.0;
        stats_dico_name["lavfi.ssim.Y"] = 0.0;

        stats_dico_name["lavfi.idet.single.bff"] = 0.0;
        stats_dico_name["lavfi.idet.single.tff"] = 0.0;
        stats_dico_name["lavfi.idet.single.progressive"] = 0.0;
        stats_dico_name["lavfi.idet.single.undetermined"] = 0.0;

        stats_dico_name["lavfi.idet.multiple.bff"] = 0.0;
        stats_dico_name["lavfi.idet.multiple.tff"] = 0.0;
        stats_dico_name["lavfi.idet.multiple.progressive"] = 0.0;
        stats_dico_name["lavfi.idet.multiple.undetermined"] = 0.0;

        stats_dico_name["lavfi.idet.repeated.bottom"] = 0.0;
        stats_dico_name["lavfi.idet.repeated.top"] = 0.0;
        stats_dico_name["lavfi.idet.repeated.neither"] = 0.0;

        stats_dico_name["pkt_duration_time"] = 0.0;
        stats_dico_name["pkt_size"] = 0.0;
    }

    //---------------------------------------------------------------------------
    void StatsFrame::data_to_vector(std::map<std::string, std::vector<double> >& res)
    {
        res["key_frame"].push_back((double)key_frame);
        res["stream_idx"].push_back((double)stream_idx);
        res["pkt_pos"].push_back((double)pkt_pos);
        res["pkt_pts"].push_back((double)pkt_pts);
        res["pkt_size"].push_back((double)pkt_size);
        res["pix_fmt"].push_back((double)pix_fmt);
        res["pict_type_char"].push_back((double)pict_type_char);
        res["duration"].push_back(duration);

        std::map<std::string, double>::iterator it = stats_dico_name.begin();
        for (; it != stats_dico_name.end(); ++it)
            res[it->first].push_back(it->second);
    }

    //---------------------------------------------------------------------------
    void StatsFrame::vector_to_data(const std::string& name, double val)
    {
        if (name == "key_frame")
            key_frame = (bool)val;
        else if (name == "stream_idx")
            stream_idx = (size_t)val;
        else if (name == "pkt_pos")
            pkt_pos = (int64_t)val;
        else if (name == "pkt_pts")
            pkt_pts = (int64_t)val;
        else if (name == "pkt_size")
            pkt_size = (int)val;
        else if (name == "pix_fmt")
            pix_fmt = (int)val;
        else if (name == "pict_type_char")
            pict_type_char = (char)val;
        else if (name == "duration")
            duration = val;
        else
            stats_dico_name[name] = val;
    }

    //---------------------------------------------------------------------------
    void StatsFrame::to_xml(std::string& out, size_t width, size_t height)
    {
        std::stringstream data;

        //     // std::stringstream pkt_pts_time; pkt_pts_time << std::fixed << std::setprecision(7) << (x[1][x_Pos]+FirstTimeStamp);
        //     std::stringstream pkt_duration_time; pkt_duration_time << std::fixed << std::setprecision(7) << stats[pos]->duration;
        //     std::stringstream key_frame;
        //     key_frame << (stats[pos]->key_frame ? '1' : '0');

        data << "        <frame media_type=\"video\"";
        data << " stream_index=\"" << stream_idx << "\"";

        //     Data<<" key_frame=\"" << key_frame.str() << "\"";
        //     Data << " pkt_pts=\"" << stats[pos]->pkt_pts << "\"";
        //     // Data<<" pkt_pts_time=\"" << pkt_pts_time.str() << "\"";
        //     if (pkt_duration_time)
        //         Data<<" pkt_duration_time=\"" << pkt_duration_time.str() << "\"";
        data << " pkt_pos=\"" << pkt_pos << "\"";
        data << " pkt_size=\"" << pkt_size << "\"";
        data << " width=\"" << width << "\" height=\"" << height << "\"";
        // data << " pix_fmt=\"" << av_get_pix_fmt_name((AVPixelFormat)pix_fmt) << "\"";
        data << " pict_type=\"" << pict_type_char << "\"";

        data << ">\n";

            std::map<std::string, double>::iterator it = stats_dico_name.begin();
            for (; it != stats_dico_name.end(); ++it)
            {
                std::string key = it->first;
                if(key == "pkt_duration_time" || key == "pkt_size")
                    continue;

                std::stringstream value;
            //     switch (Plot_Pos)
            //     {
            //     case Item_Crop_x2 :
            //     case Item_Crop_w :
            //         // Special case, values are from width
            //         value<<Width-y[Plot_Pos][x_Pos];
            //         break;
            //     case Item_Crop_y2 :
            //     case Item_Crop_h :
            //         // Special case, values are from height
            //         value<<Height-y[Plot_Pos][x_Pos];
            //         break;
            //     default:
                value << it->second;
            //     }

                data << "            <tag key=\"" + key + "\" value=\"" + value.str() + "\"/>\n";
            }

        data << "        </frame>\n";

        out += data.str();
    }
}
