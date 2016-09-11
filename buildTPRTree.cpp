/*
 * buildTPRTree.cpp
 * command: ./buildTPRTree.out serial < input_data
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

map<ll, ll> real_id_to_object_id;
map<ll, ll>::iterator it;

vector<obj> objects;

void readDataset()
{
    char str[200];
    gets(str);

    ll p_id, nextID = 0;
    double lat, lon, time, speed;
    char ignore[20];

    while(scanf("%lld %lf %s %lf %lf", &p_id, &time, ignore, &lat, &lon)!=EOF)
    {
    	//cout << p_id << ' ' << time << endl;
        obj temp;
        time *=10;
        it = real_id_to_object_id.find(p_id); ///check if exists in map
        if(it == real_id_to_object_id.end())
        {
            temp.id = nextID;
            real_id_to_object_id[p_id] = nextID;
            nextID++;

            temp.time_offset = time;
            temp.inst.push_back(sample(lat, lon, time-temp.time_offset, 20));

            objects.push_back(temp);
        }
        else
        {
            int pos = (*it).second;
            double offset = objects[pos].time_offset;
            //int count = objects[pos].inst.size();
            objects[pos].inst.push_back(sample(lat, lon, time-offset, 20));
        }
    }
}

void showDataset(int ind)
{
    for(int i=0; i<objects.size(); i++)
    {
        cout << objects[i].id << ":\n";
        cout << objects[i].inst.size() << endl;

        if(objects[i].inst.size() > ind+1)
        {
            cout << objects[i].inst[ind].lat << ' ' << objects[i].inst[ind].lon << ' ' << objects[i].inst[ind].time << endl;
        }

        if(objects[i].inst.size() > ind+1)
        {
            cout << objects[i].inst[ind+1].lat << ' ' << objects[i].inst[ind+1].lon << ' ' << objects[i].inst[ind+1].time << endl;
        }

        cout << endl;
    }
}

Index* createIndex(char *diskFileName, unsigned long capacity)
{
    // create a property set with default values.
    // see utility.cc for all defaults  http://libspatialindex.github.io/doxygen/Utility_8cc_source.html#l00031
    Tools::PropertySet* ps = GetDefaults();
    Tools::Variant var;

    // Set index to store in disk (default is disk)
    //index identifier is set internally (randomly)

    var.m_varType = Tools::VT_PCHAR;
    var.m_val.pcVal = const_cast<char*>(diskFileName);
    ps->setProperty("FileName", var);

    // set index type to TPR-Tree
    var.m_varType = Tools::VT_ULONG;
    var.m_val.ulVal = RT_TPRTree;
    ps->setProperty("IndexType", var);

    // set horizon
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
        throw "Failed to create valid index";
    }
    else
    {
        //cout << "created index" << endl;
    }

    return idx;
}


int buildTree(Index* idx, int take)
{
    int inserted = 0, count = 0;
    bool ok;
    for(int i=0; i<objects.size(); i++)
    {
        if(objects[i].inst.size() <= take+1) continue; ///this object doesn't have 'take' NO. of line segments

        ok = insertMovingRegion(idx, objects[i].inst[take], objects[i].inst[take+1], objects[i].id);

        if(ok) inserted++;
        else
        {
            ///do something if need so
        }

        if(inserted % 1000 == 0)
        {
            //idx->buffer().flush();
            idx->flush();
        }
    }
    return inserted;
}

///one argument will be received, 'n', meaning, build tprtree with n'th line segment of every object

int main(int argc, char** argv)
{
    cout.precision(10);
    if(argc < 2)
    {
        puts("command: ./buildTPRTree.out serial < input_data");
        return 1;
    }

    readDataset();

    unsigned long capacity = objects.size();

    char diskFileName[] = "tprtree";
    string fileName = string(diskFileName);

    //initalise Indexpointer
    Index* idx = 0;
    try
    {
        idx = createIndex(diskFileName, capacity);
        idx->flush();
    }
    catch(...)
    {
        //cout << "exception during index creation"
        return 1;
    }

    /*
    Tools::PropertySet properties = idx->GetProperties();
    Tools::Variant vari = properties.getProperty("IndexIdentifier");
    cout << "ID: " << vari.m_val.llVal << endl;
    */

    //showDataset(atoi(argv[1]));

    int take = atoi(argv[1]), totalObjects;
    totalObjects = buildTree(idx, take);

    //cout << totalObjects << endl;

    //cout << "building complete\n";

    /*
    sample sam1(475245.596659, 4974049.111000, 0,20), sam2(475265.487272, 4974110.687660, 1000,20);
    obj o2;

    o2.id=243;

    o2.inst.pb(sam1);
    o2.inst.pb(sam2);

    PT low, high, velo;
    take=0;

    low = getBottomLeftPoint(PT(o2.inst[take].lat, o2.inst[take].lon));
    high = getUpperRightPoint(PT(o2.inst[take].lat, o2.inst[take].lon));
    velo = getVelocity(o2.inst[take], o2.inst[take+1]);

    double low_coords[2], high_coords[2], low_v[2];
    low_coords[0] = low.lat;
    low_coords[1] = low.lon;
    high_coords[0] = high.lat;
    high_coords[1] = high.lon;
    low_v[0] = velo.lat;
    low_v[1] = velo.lon;


    SpatialIndex::IShape* shape=0;
    ObjVisitor* visitor = new ObjVisitor;

    shape = new SpatialIndex::MovingRegion(low_coords, high_coords, low_v, low_v,  o2.inst[take].time, o2.inst[take+1].time, 2);
    //addMovingRegion(idx, disk, low_coords, high_coords, low_v, o1.inst[take].time, o1.inst[take+1].time, o1.id);
    //insertMovingRegion(idx, disk, o2.inst[take], o2.inst[take+1], o2.id);
    idx->index().intersectsWithQuery(*shape, *visitor);

    cout << "intersects with: " << visitor->GetResultCount() << " objects\n";

    int64_t nResultCount;
    nResultCount = visitor->GetResultCount();

    // get actual results
    std::vector<SpatialIndex::IData*>& results = visitor->GetResults();
    // an empty vector that wewill copt the results to
    vector<SpatialIndex::IData*>* resultsCopy = new vector<SpatialIndex::IData*>();

    // copy the Items into the newly allocated vector array
    // we need to make sure to clone the actual Item instead
    // of just the pointers, as the visitor will nuke them
    // upon destroy
    for (int64_t i = 0; i < nResultCount; ++i){
        resultsCopy->push_back(dynamic_cast<SpatialIndex::IData*>(results[i]->clone()));
    }

    //cout << "Inersects with: ";
    for (int64_t i = 0; i < nResultCount; ++i){
        SpatialIndex::IData* data=0;
        data = (*resultsCopy)[i];
        if(i>0) cout << ' ';
        cout << data->getIdentifier();
        cout << endl;
    }
    */


    idx->buffer().flush();
    idx->flush();

    return 0;
}


