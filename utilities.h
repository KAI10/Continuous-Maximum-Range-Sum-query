/*
 * utilities.h
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-09-10
 */

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

///inserts a moving region in tpr tree
bool insertMovingRegion(Index *idx, sample &first, sample &second, int64_t id)
{
    PT low, high, velo; ///velo -> velocity vector of dula rectangle

    low = getBottomLeftPoint(PT(first.lat, second.lon));
    high = getUpperRightPoint(PT(first.lat, second.lon));
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

