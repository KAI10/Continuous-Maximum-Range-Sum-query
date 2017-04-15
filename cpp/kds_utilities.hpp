void addToKDS(Event event)
{
    kds.insert(event);
}

long long addLineEventsToKDS(long long total_events, double current_time, double event_time){
    //cout << "inside addLineEventsToKDS\n";
    if(event_time > current_time){
        Event e(total_events, NEW_SAMPLE, -1, -1, event_time);
        addToKDS(e);
        //cout << "total_events: " << total_events << endl;
        total_events++;
        //cout << "total_events: " << total_events << "\n\n";
    }
    return total_events;
}

long long addINIEventsToKDS(int oid1, int oid2, long long total_events, double event_time, int event_type){
    Event e(total_events, event_type, oid1, oid2, event_time);
    addToKDS(e);
    //cout << "total_events: " << total_events << endl;
    total_events++;
    //cout << "total_events: " << total_events << "\n\n";
    return total_events;
}