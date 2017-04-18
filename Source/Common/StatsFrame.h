/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Stats frame functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef STATSFRAMEH
#define STATSFRAMEH
//---------------------------------------------------------------------------

#include <string>
#include <map>
#include <vector>

//---------------------------------------------------------------------------
namespace MediaConch {

struct StatsFrame
{
    // Constructor / Destructor
    StatsFrame();
    ~StatsFrame();

    StatsFrame(const StatsFrame&);

    // Data
    bool                           key_frame;                  // Frame is a Key frame?
    size_t                         stream_idx;                 // Stream Index of the frame
    int64_t                        pkt_pos;                    // Frame offsets
    int64_t                        pkt_pts;                    // Frame pkt_pts
    int                            pkt_size;                   // Frame size
    int                            pix_fmt;                    // Frame Format
    char                           pict_type_char;             //
    double                         duration;                   // Duration of the frame

    std::map<std::string, double>  stats_dico_name;

    void data_to_vector(std::map<std::string, std::vector<double> >& res);
    void vector_to_data(const std::string&, double val);

    void to_xml(std::string& out, size_t width, size_t height);

private:
    StatsFrame& operator=(const StatsFrame&);

    void         fill_dico_stats_names();
};

}

#endif // !STATSFRAMEH
