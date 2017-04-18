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
#include "DatabaseStats.h"
#include <sstream>
#include <stdlib.h>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// DatabaseStats
//***************************************************************************

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
DatabaseStats::DatabaseStats() : Database()
{
}

//---------------------------------------------------------------------------
DatabaseStats::~DatabaseStats()
{
}

//---------------------------------------------------------------------------
void DatabaseStats::set_database_directory(const std::string& dirname)
{
    Database::set_database_directory(dirname);
}

//---------------------------------------------------------------------------
void DatabaseStats::set_database_filename(const std::string& filename)
{
    Database::set_database_filename(filename);
}

//---------------------------------------------------------------------------
void DatabaseStats::get_sql_query_for_create_stats_table(std::string& q)
{
    std::stringstream create;
    create << "CREATE TABLE IF NOT EXISTS MEDIACONCH_STATS "; // Table name
    create << "(USER                  INT   NOT NULL, ";
    create << "FILE_ID                INT   NOT NULL, ";
    create << "STREAM                 INT   NOT NULL, ";
    create << "PLUGIN                 TEXT  NOT NULL, ";
    create << "DATA_NAME              TEXT  NOT NULL, ";
    create << "DATA                   TEXT  NOT NULL, ";
    create << "DATA_COMPRESSED        INT   NOT NULL, ";
    create << "DATA_SIZE              INT   NOT NULL);";
    q = create.str();
}

}
