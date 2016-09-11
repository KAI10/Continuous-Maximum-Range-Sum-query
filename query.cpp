/*
 * query.cpp
 * command: ./query.out x1 y1 t1 x2 y2 t2 speed capacity n
 * Here 'n' means: run query on n'th TPR tree. This argument is optional
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
#define d1 1000
#define d2 1000

#include "utilities.h"

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

int main(int argc, char** argv)
{
    if(argc < 9)
    {
        puts("command: ./query.out x1 y1 t1 x2 y2 t2 speed capacity n");
        return 1;
    }

    string diskFileName;
    if(argc == 10) diskFileName = "tprtree" + to_string(atoi(argv[9]));
    else diskFileName = "tprtree";

    //cout << diskFileName << endl;
    unsigned long capacity = atol(argv[8]);

    /// retrieve Indexpointer from disk
    Index* idx = getIndexFromDisk(diskFileName.c_str(), capacity);


    ///build the two sample points from arguments
    sample first, second;

    first.lat = atof(argv[1]);
    first.lon = atof(argv[2]);
    first.time = atof(argv[3]);

    second.lat = atof(argv[4]);
    second.lon = atof(argv[5]);
    second.time = atof(argv[6]);

    first.speed = atof(argv[7]);
    second.speed = first.speed;


    ///create necessary data structures for range query
    PT low, high, velo;
    low = getBottomLeftPoint(PT(first.lat, first.lon));
    high = getUpperRightPoint(PT(first.lat, first.lon));
    velo = getVelocity(first, second);

    double low_coords[] = {low.lat, low.lon}, high_coords[] = {high.lat, high.lon}, low_v[] = {velo.lat, velo.lon};

    SpatialIndex::IShape* shape=0;

    ObjVisitor* visitor = new ObjVisitor;
    shape = new SpatialIndex::MovingRegion(low_coords, high_coords, low_v, low_v, first.time, second.time, 2);


    ///Do the range query
    try
    {
        idx->index().intersectsWithQuery(*shape, *visitor);
    }
    catch(...)
    {
        cout << "Error doing range query.\n";
    }


    int64_t nResultCount;
    nResultCount = visitor->GetResultCount();

    /// get actual results
    std::vector<SpatialIndex::IData*>& results = visitor->GetResults();

    // an empty vector that wewill copt the results to
    //vector<SpatialIndex::IData*>* resultsCopy = new vector<SpatialIndex::IData*>();

    // copy the Items into the newly allocated vector array
    // we need to make sure to clone the actual Item instead
    // of just the pointers, as the visitor will nuke them
    // upon destroy
    /*
    for (int64_t i = 0; i < nResultCount; ++i)
    {
        resultsCopy->push_back(dynamic_cast<SpatialIndex::IData*>(results[i]->clone()));
    }
    */

    //cout << "Inersects with: ";
    for (int64_t i = 0; i < nResultCount; ++i)
    {
        SpatialIndex::IData* data=0;
        data = results[i];
        //data = (*results)[i];
        //if(i>0) cout << ' ';
        printf("%ld\n", data->getIdentifier());
        //cout << data->getIdentifier() << endl;
        //cout << endl;
    }

    //idx->flush();
    return 0;
}


