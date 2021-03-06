/**********************************************************************
** This program is part of 'MOOSE', the
** Messaging Object Oriented Simulation Environment.
**           Copyright (C) 2003-2010 Upinder S. Bhalla. and NCBS
** It is made available under the terms of the
** GNU Lesser General Public License version 2.1
** See the file COPYING.LIB for the full notice.
**********************************************************************/

#include "../basecode/header.h"
#include "../basecode/global.h"
#include <fstream>
#include <array>

#include "TableBase.h"
#include "Table.h"
#include "../scheduling/Clock.h"
#include "StreamerBase.h"

static SrcFinfo1< vector< double >* > *requestOut()
{
    static SrcFinfo1< vector< double >* > requestOut(
        "requestOut",
        "Sends request for a field to target object"
    );
    return &requestOut;
}

static DestFinfo *handleInput()
{
    static DestFinfo input(
        "input",
        "Fills data into table. Also handles data sent back following request",
        new OpFunc1< Table, double >( &Table::input )
    );
    return &input;
}

const Cinfo* Table::initCinfo()
{
    //////////////////////////////////////////////////////////////
    // Field Definitions
    //////////////////////////////////////////////////////////////
    static ValueFinfo< Table, double > threshold(
        "threshold"
        , "threshold used when Table acts as a buffer for spikes"
        , &Table::setThreshold
        , &Table::getThreshold
    );

    static ValueFinfo< Table, bool > useStreamer(
        "useStreamer"
        , "When set to true, write to a file instead writing in memory."
        " If `outfile` is not set, streamer writes to default path."
        , &Table::setUseStreamer
        , &Table::getUseStreamer
    );

    static ValueFinfo< Table, bool > useSpikeMode(
        "useSpikeMode"
        , "When set to true, look for spikes in a time-series."
        " Normally used for monitoring Vm for action potentials."
        " Could be used for any thresholded event. Default is False."
        , &Table::setUseSpikeMode
        , &Table::getUseSpikeMode
    );

    static ValueFinfo< Table, string > datafile(
        "datafile"
        , "Set the name of file to which data is written to. If set, "
        " streaming support is automatically enabled."
        , &Table::setDatafile
        , &Table::getDatafile
    );

    static ValueFinfo< Table, string > outfile(
        "outfile"
        , "Use datafile (deprecated)"
        , &Table::setDatafile
        , &Table::getDatafile
    );


    static ValueFinfo< Table, string > format(
        "format"
        , "Data format for table: default csv"
        , &Table::setFormat
        , &Table::getFormat
    );

    // relevant for Streamer class.  When data is written to a datafile, this is
    // used to create column name.
    static ValueFinfo< Table, string > columnName(
        "columnName"
        , "Name of the table written in header of data file."
        , &Table::setColumnName
        , &Table::getColumnName
    );

    //////////////////////////////////////////////////////////////
    // MsgDest Definitions
    //////////////////////////////////////////////////////////////

    static DestFinfo spike(
        "spike",
        "Fills spike timings into the Table. Signal has to exceed thresh",
        new OpFunc1< Table, double >( &Table::spike )
    );

    static DestFinfo process(
        "process",
        "Handles process call, updates internal time stamp.",
        new ProcOpFunc< Table >( &Table::process )
    );

    static DestFinfo reinit(
        "reinit",
        "Handles reinit call.",
        new ProcOpFunc< Table >( &Table::reinit )
    );

    //////////////////////////////////////////////////////////////
    // SharedMsg Definitions
    //////////////////////////////////////////////////////////////
    static Finfo* procShared[] =
    {
        &process, &reinit
    };

    static SharedFinfo proc(
        "proc"
        , "Shared message for process and reinit"
        , procShared, sizeof( procShared ) / sizeof( const Finfo* )
    );

    //////////////////////////////////////////////////////////////
    // Field Element for the vector data
    // Use a limit of 2^20 entries for the tables, about 1 million.
    //////////////////////////////////////////////////////////////

    static Finfo* tableFinfos[] =
    {
        &threshold,		// Value
        &format,                // Value
        &columnName,            // Value
        &datafile,              // Value
        &outfile,               // Value
        &useStreamer,           // Value
        &useSpikeMode,          // Value
        handleInput(),		// DestFinfo
        &spike,			// DestFinfo
        requestOut(),		// SrcFinfo
        &proc,			// SharedFinfo
    };

    static string doc[] =
    {
        "Name", "Table",
        "Author", "Upi Bhalla",
        "Description",
        "Table for accumulating data values, or spike timings. "
        "Can either receive incoming doubles, or can explicitly "
        "request values from fields provided they are doubles. "
        "The latter mode of use is preferable if you wish to have "
        "independent control of how often you sample from the output "
        "variable. \n"
        "Typically used for storing simulation output into memory, or to file"
        " when stream is set to True \n"
        "There are two functionally identical variants of the Table "
        "class: Table and Table2. Their only difference is that the "
        "default scheduling of the Table (Clock Tick 8, dt = 0.1 ms ) "
        "makes it suitable for "
        "tracking electrical compartmental models of neurons and "
        "networks. \n"
        "Table2 (Clock Tick 18, dt = 1.0 s) is good for tracking "
        "biochemical signaling pathway outputs. \n"
        "These are just the default values and Tables can be assigned"
        " to any Clock Tick and timestep in the usual manner.",
    };

    static Dinfo< Table > dinfo;

    static Cinfo tableCinfo (
        "Table",
        TableBase::initCinfo(),
        tableFinfos,
        sizeof( tableFinfos ) / sizeof ( Finfo* ),
        &dinfo,
        doc,
        sizeof( doc ) / sizeof( string )
    );

    static string doc2[] =
    {
        doc[0], "Table2", doc[2], doc[3], doc[4], doc[5]
    };

    doc2[1] = "Table2";

    static Cinfo table2Cinfo (
        "Table2",
        TableBase::initCinfo(),
        tableFinfos,
        sizeof( tableFinfos ) / sizeof ( Finfo* ),
        &dinfo,
        doc2,
        sizeof( doc2 ) / sizeof( string )
    );

    return &tableCinfo;
}

//////////////////////////////////////////////////////////////
// Basic class Definitions
//////////////////////////////////////////////////////////////

static const Cinfo* tableCinfo = Table::initCinfo();

Table::Table() :
    threshold_( 0.0 ),
    lastTime_( 0.0 ),
    input_( 0.0 ),
    fired_(false),
    useSpikeMode_(false),
    dt_( 0.0 ),
    lastN_(0),
    useFileStreamer_(false),
    datafile_(""),
    format_("csv")
{
}

Table::~Table( )
{
    // Make sure to write to rest of the entries to file before closing down.
    if( useFileStreamer_ )
    {
        mergeWithTime( data_ );
        assert( ! datafile_.empty() );
        StreamerBase::writeToOutFile( datafile_, format_, APPEND, data_, columns_);
        clearAllVecs();
    }
}

Table& Table::operator=( const Table& tab )
{
    return *this;
}


//////////////////////////////////////////////////////////////
// MsgDest Definitions
//////////////////////////////////////////////////////////////

void Table::process( const Eref& e, ProcPtr p )
{
    lastTime_ = p->currTime;
    tvec_.push_back(lastTime_);

    // Copy incoming data to ret and insert into vector.
    vector< double > ret;
    requestOut()->send( e, &ret );

    if (useSpikeMode_)
    {
        for ( auto i = ret.begin(); i != ret.end(); ++i )
            spike( *i );
    }
    else
        vec().insert( vec().end(), ret.begin(), ret.end() );

    /*  If we are streaming to a file, let's write to a file. And clean the
     *  vector.
     *  Write at every 5 seconds or whenever size of vector is more than 10k.
     */
    if( useFileStreamer_ )
    {
        if( fmod(lastTime_, 5.0) == 0.0 || getVecSize() >= 10000 )
        {
            mergeWithTime( data_ );
            StreamerBase::writeToOutFile( datafile_, format_, APPEND, data_, columns_ );
            clearAllVecs();
        }
        }
    }

void Table::clearAllVecs()
{
    clearVec();
    tvec_.clear();
    data_.clear();
}

/**
 * @brief Reinitialize
 *
 * @param e
 * @param p
 */
void Table::reinit( const Eref& e, ProcPtr p )
{
    tablePath_ = e.id().path();
    unsigned int numTick = e.element()->getTick();
    Clock* clk = reinterpret_cast<Clock*>(Id(1).eref().data());


    dt_ = clk->getTickDt( numTick );
    fired_ = false;

    // Set column name for this table. It is used in Streamer to generate
    // column names because path can be pretty verbose and name may not be
    // unique. This is the default column name.
    if( tableColumnName_.empty() )
        tableColumnName_ = moose::moosePathToColumnName(tablePath_);

    columns_ = {"time", tableColumnName_};

    /** Create the default filepath for this table.  */
    if( useFileStreamer_ )
    {
        // If user has not set the filepath, then use the table path prefixed
        // with rootdit as path.
        if( datafile_.empty() )
            setDatafile(moose::moosePathToUserPath(tablePath_) + '.' + format_);
    }

    input_ = 0.0;
    vec().resize( 0 );
    lastTime_ = 0;
    vector< double > ret;
    requestOut()->send( e, &ret );

    if (useSpikeMode_)
    {
        for ( auto i = ret.begin(); i != ret.end(); ++i )
            spike( *i );
    }
    else
        vec().insert( vec().end(), ret.begin(), ret.end() );

    tvec_.push_back(lastTime_);

    if( useFileStreamer_ )
    {
        mergeWithTime( data_ );
        StreamerBase::writeToOutFile(datafile_, format_, WRITE, data_, columns_);
        clearAllVecs();
    }
}

//////////////////////////////////////////////////////////////
// Used to handle direct messages into the table, or
// returned plot data from queried objects.
//////////////////////////////////////////////////////////////
void Table::input( double v )
{
    vec().push_back( v );
}

void Table::spike( double v )
{
    if ( fired_ )
    { // Wait for it to go below threshold
        if ( v < threshold_ )
            fired_ = false;
    }
    else
    {
        if ( v > threshold_ )
        {
            // wait for it to go above threshold.
            fired_ = true;
            vec().push_back( lastTime_ );
        }
    }
}

//////////////////////////////////////////////////////////////
// Field Definitions
void Table::setThreshold( double v )
{
    threshold_ = v;
}

double Table::getThreshold() const
{
    return threshold_;
}

// Set the format of table to which its data should be written.
void Table::setFormat( string format )
{
    if( format == "csv" )
        format_ = format;
    else
        LOG( moose::warning
             , "Unsupported format " << format
             << " only csv is supported for single table."
           );
}

// Get the format of table to which it has to be written.
string Table::getFormat( void ) const
{
    return format_;
}

/*  User defined column name for streamer  */
string Table::getColumnName( void ) const
{
    return tableColumnName_;
}

void Table::setColumnName( const string colname )
{
    tableColumnName_ = colname ;
}

/* Enable/disable streamer support. */
void Table::setUseStreamer( bool useStreamer )
{
    useFileStreamer_ = useStreamer;
}

bool Table::getUseStreamer( void ) const
{
    return useFileStreamer_;
}

/* Enable/disable spike mode. */
void Table::setUseSpikeMode( bool useSpikeMode )
{
    useSpikeMode_ = useSpikeMode;
}

bool Table::getUseSpikeMode( void ) const
{
    return useSpikeMode_;
}


/*  set/get datafile_ */
void Table::setDatafile( string filepath )
{
    datafile_ = moose::createMOOSEPath( filepath );
    if( ! moose::createParentDirs( datafile_ ) )
        datafile_ = moose::toFilename( datafile_ );

    setUseStreamer( true );
    // If possible get the format of file as well.
    format_ = moose::getExtension( datafile_, true );
    if( format_.size() == 0 )
        format_ = "csv";
}

string Table::getDatafile( void ) const
{
    return datafile_;
}

// Get the dt_ of this table
double Table::getDt( void ) const
{
    return dt_;
}

/**
 * @brief Take the vector from table and timestamp it. It must only be called
 * when packing the data for writing.
 */
void Table::mergeWithTime( vector<double>& data )
{
    auto v = vec();
    for (unsigned int i = 0; i < v.size(); i++)
    {
        data.push_back(tvec_[i]);
        data.push_back(v[i]);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis.  Convert table data to JOSN.
 *
 * @Returns string.
 */
/* ----------------------------------------------------------------------------*/
string Table::toJSON(bool withTime, bool clear)
{
    stringstream ss;
    auto v = vec();
    if( clear )
        lastN_ = 0;

    for (unsigned int i = lastN_; i < v.size(); i++)
    {
        if(withTime)
            ss << '[' << tvec_[i] << ',' << v[i] << "],";
        else
            ss << v[i] << ',';
    }

    string res = ss.str();
    if(',' == res.back())
        res.pop_back();

    if( clear )
        clearAllVecs();
    else
        lastN_ += v.size();

    return res;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Collect data in given vector. Its similar to toJSON function.
 *
 * @Param data
 * @Param withTime
 * @Param clear
 */
/* ----------------------------------------------------------------------------*/
void Table::collectData(vector<double>& data, bool withTime, bool clear)
{
    auto v = vec();
    if( clear )
        lastN_ = 0;

    for (unsigned int i = lastN_; i < v.size(); i++)
    {
        if(withTime)
            data.push_back(tvec_[i]);
        data.push_back(v[i]);
    }

    if( clear )
        clearAllVecs();
    else
        lastN_ = v.size();
}
