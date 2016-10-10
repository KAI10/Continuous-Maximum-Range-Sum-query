/*
 * tree_utilities.h
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-09-10
 */

#include <iostream>
#include <spatialindex/capi/sidx_api.h>
#include <spatialindex/capi/sidx_impl.h>
#include <spatialindex/capi/sidx_config.h>

using namespace std;
using namespace SpatialIndex;

typedef long long ll;
const double HORIZON = 1e10;

#define mem(list, val) memset(list, (val), sizeof(list))
#define pb push_back
#define tol 1e-12
#define d1 .01
#define d2 .01

/// Necessary Structures

struct PT
{
    double lat, lon;
    PT() {}
    PT(double lt, double ln)
    {
        lat=lt;
        lon=ln;
    }
};

struct sample
{
    double lat, lon, time, speed;
    sample() {}
    sample(double lt, double ln, double tm, double sp)
    {
        lat = lt;
        lon = ln;
        time = tm;
        speed = sp;
    }
    bool operator<(const sample &p)const
    {
        return time < p.time;
    }
};

struct obj
{
    ll id;
    double time_offset;
    vector<sample> inst;
};

///Necessary functions

PT getVelocity(sample &one, sample &two)
{
    PT v;
    v.lat = (two.lat - one.lat)/(two.time - one.time);
    v.lon = (two.lon - one.lon)/(two.time - one.time);
    return v;
}

inline PT getBottomLeftPoint(PT p)
{
    return PT(p.lat - d1/2, p.lon - d2/2);
}
inline PT getUpperRightPoint(PT p)
{
    return PT(p.lat + d1/2, p.lon + d2/2);
}

///retrieve tprtree index from disk
Index* getIndexFromDisk(const char* diskFileName, unsigned long capacity)
{
    // create a property set with default values.
    // see utility.cc for all defaults  http://libspatialindex.github.io/doxygen/Utility_8cc_source.html#l00031
    Tools::PropertySet* ps = GetDefaults();
    Tools::Variant var;

    // Set index to store in disk (default is disk)
    //index identifier is set internally (randomly)

    /// set index type to TPR-Tree
    var.m_varType = Tools::VT_ULONG;
    var.m_val.ulVal = RT_TPRTree;
    ps->setProperty("IndexType", var);

    ///don't overwrite the existing index
    var.m_varType = Tools::VT_BOOL;
    var.m_val.bVal = false;
    ps->setProperty("Overwrite", var);

    ///give disk file name
    var.m_varType = Tools::VT_PCHAR;
    var.m_val.pcVal = const_cast<char*>(diskFileName);
    ps->setProperty("FileName", var);

    ///give identifier to find index in disk, normally = 1
    var.m_varType = Tools::VT_LONGLONG;
    var.m_val.llVal = 1;
    ps->setProperty("IndexIdentifier", var);

    /// set horizon >= time difference between consecutive time samples
    var.m_varType = Tools::VT_DOUBLE;
    var.m_val.dblVal = HORIZON;
    ps->setProperty("Horizon", var);

    var.m_varType = Tools::VT_ULONG;
    var.m_val.ulVal = capacity;
    ps->setProperty("IndexCapacity", var);

    //set leaf capacity
    var.m_varType = Tools::VT_ULONG;
    var.m_val.ulVal = capacity;
    ps->setProperty("LeafCapacity", var);

    //cout << "property setting complete\n";
    // initalise index
    Index* idx = new Index(*ps);
    delete ps;

    //cout << "Here\n";
    // check index is ok
    if (!idx->index().isIndexValid())
    {
        throw "Failed to get valid index";
    }
    else
    {
        //cout << "Got index" << endl;
    }

    return idx;
}


///inserts a moving region in tpr tree
bool insertMovingRegion(Index *idx, sample &first, sample &second, int64_t id)
{
    PT low, high, velo; ///velo -> velocity vector of dula rectangle

    low = getBottomLeftPoint(PT(first.lat, first.lon));
    high = getUpperRightPoint(PT(first.lat, first.lon));
    velo = getVelocity(first, second);

    double start = first.time, finish = second.time;
    double low_coords[] = {low.lat, low.lon}, high_coords[] = {high.lat, high.lon}, low_v[] = {velo.lat, velo.lon};

    ///don't need to use these, necessary for func call
    uint32_t nDataLength = 0;
    uint8_t* pData = 0;

    // create shape
    SpatialIndex::IShape* shape = 0;
    shape = new SpatialIndex::MovingRegion(low_coords, high_coords, low_v, low_v, start, finish, 2);

    //cout << "Moving Region Created\n";
    // insert into index along with the an object and an ID
    try
    {
        idx->index().insertData(nDataLength, pData, *shape, id);
    }
    catch(...)
    {
        cout << "Exception caught while inserting object " << id << endl;
        return false;
    }

    //cout << "MovingRegion " << id << " inserted into index." << endl;
    delete shape;
    return true;
}

bool time_compare(obj lt, obj rt)
{
    return (lt.inst[0].time <= rt.inst[0].time);
}

