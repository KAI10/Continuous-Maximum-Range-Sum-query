/*
 * query.cpp
 * command: ./query.out x1 y1 t1 x2 y2 t2 speed capacity n
 * Here 'n' means: run query on n'th TPR tree. This argument is optional
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-09-10
 */

///./query.out x1 y1 t1 x2 y2 t2 speed n

void query(vector<int> &res, double x1, double y1, double t1, double x2, double y2, double t2, double speed)
{
    //const clock_t begin_time = clock();
    /*
    if(argc < 9)
    {
        puts("command: ./query.out x1 y1 t1 x2 y2 t2 speed capacity n");
        return 1;
    }
    */

    //cout << diskFileName << endl;
    //unsigned long capacity = (unsigned long)numberOfObjects;

    // retrieve Indexpointer from disk
    //Index* idx = getIndexFromDisk(diskFileName.c_str(), capacity);


    ///build the two sample points from arguments
    sample first(x1, y1, t1, speed), second(x2, y2, t2, speed);

    /*
    first.lat = x1;
    first.lon = y1;
    first.time = t1;

    second.lat = x2;
    second.lon = y2;
    second.time = t2;

    first.speed = speed;
    second.speed = first.speed;
    */


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
    //res.clear();
    for (int64_t i = 0; i < nResultCount; ++i)
    {
        SpatialIndex::IData* data=0;
        data = results[i];
        //data = (*results)[i];
        //if(i>0) cout << ' ';
        res.push_back(data->getIdentifier());
        //printf("%ld\n", data->getIdentifier());
        //cout << data->getIdentifier() << endl;
        //cout << endl;
    }

    //cout << "elapsed time: " << double( clock () - begin_time ) /  CLOCKS_PER_SEC << " seconds\n";

    //idx->flush();
    //return 0;
}


