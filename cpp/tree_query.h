/*
 * tree_query.h
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-09-10
 */


void query(vector<int> &res, double x1, double y1, double t1, double x2, double y2, double t2, double speed)
{

    ///build the two sample points from arguments
    sample first(x1, y1, t1, speed), second(x2, y2, t2, speed);


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
        res.push_back(data->getIdentifier());
        //cout << data->getIdentifier() << endl;
        //cout << endl;
    }
}


