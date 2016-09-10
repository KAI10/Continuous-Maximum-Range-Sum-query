/*
 * buildTPRTree.cpp
 * command: ./buildTPRTree.out number
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
    string str;
    cin >> str;

    ll p_id, nextID = 0;
    double lat, lon, time, speed;

    int count = 0;

    while(scanf("%lld,%lf,%lf,%lf,%lf", &p_id, &lat, &lon, &time, &speed)!=EOF)
    {
        obj temp;
        //time *=10;
        it = real_id_to_object_id.find(p_id); ///check if exists in map
        if(it == real_id_to_object_id.end())
        {
            temp.id = nextID;
            real_id_to_object_id[p_id] = nextID;
            nextID++;

            temp.time_offset = 0;
            temp.inst.push_back(sample(lat, lon, 0, 20));

            objects.push_back(temp);
        }
        else
        {
            int pos = (*it).second;
            count = objects[pos].inst.size();

            ///HERE 200 IS HARD CODED TIME DIFFERENCE BETWEEN TWO CONSECUTIVE SAMPLE TIMES
            objects[pos].inst.push_back(sample(lat, lon, count*200, 20));
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

Index* createIndex(const char *diskFileName)
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
    int inserted = 0;
    bool ok;
    for(int i=0; i<objects.size(); i++)
    {
        //cout << objects[i].inst.size() << endl;
        if(objects[i].inst.size() <= take+1) continue; ///this object doesn't have 'take' NO. of line segments

        ok = insertMovingRegion(idx, objects[i].inst[take], objects[i].inst[take+1], objects[i].id);

        if(ok) inserted++;
        else
        {
            ///do something if need so
        }

        if(inserted % 1000 == 0)
        {
            idx->buffer().flush();
            idx->flush();
        }
    }

    return inserted;
}


int main(int argc, char** argv)
{
    cout.precision(10);
    readDataset();

    //cout << objects[0].inst[30].lat << ' ' << objects[0].inst[30].lon << ' ' << objects[0].inst[30].time << endl;
    //cout << objects[0].inst[31].lat << ' ' << objects[0].inst[31].lon << ' ' << objects[0].inst[31].time << endl;
    //showDataset(69);

    for(int i=0;; i++)
    {
        string diskFileName = "tprtree" + to_string(i);

        //initalise Indexpointer
        Index* idx=0;
        try
        {
            idx = createIndex(diskFileName.c_str());
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
        cout << diskFileName << " ID: " << vari.m_val.llVal << endl;
        */

        int totalObjects;
        totalObjects = buildTree(idx, i);
        //cout << "objects = " << totalObjects << endl;

        //cout << "building complete\n";

        if(totalObjects == 0) break;

        idx->buffer().flush();
        idx->flush();
    }

    return 0;
}






