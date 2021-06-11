#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"
#include "GPXParser.h"
#include "GPXHelpers.h"

/*  Assignment 2 CIS*2750
 *          Author: Karina Garmendez 1078442
 *          References: Denis Nikitenko
 *          Created: March 2021
 */

GPXdoc* createGPXdoc(char* fileName) {

    xmlNode *firstNode = NULL;
    xmlDoc *fileInfo = NULL;

    fileInfo = xmlReadFile(fileName, NULL, 0);

    if (fileInfo == NULL) {
        fprintf(stderr, "ERROR: File parsing of %s failed\n", fileName);
        return NULL;
    }
    GPXdoc *parsedData = (GPXdoc *)malloc(sizeof(GPXdoc));

    firstNode = xmlDocGetRootElement(fileInfo);
    parsedData->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    parsedData->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    parsedData->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    parsedData->creator = calloc(1000, sizeof(char));
    strcpy(parsedData->creator, "Karina");
    parsedData->version = 0.0;
    strcpy(parsedData->namespace, "namespace");

    firstNode = xmlDocGetRootElement(fileInfo);
    if (firstNode->ns != NULL) {
        strcpy(parsedData->namespace, (char*)firstNode->ns->href);
    }
    xmlAttr *attr;
    for (attr = firstNode->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);
        if (strcmp(attrName, "creator") == 0) {
            strcpy(parsedData->creator, cont);
        }
        if (strcmp(attrName, "version") == 0) {
            parsedData->version = atof(cont);
        }
    }
    for (xmlNode* child = firstNode->children; child != NULL; child = child->next) {
        char *name = (char*)child->name;
        if (strcmp(name, "wpt") == 0) {
            Waypoint* waypt = WaypointDataTransfer(child);
            insertBack(parsedData->waypoints, (Waypoint*)waypt);
        }
        if (strcmp(name, "rte") == 0) {
            Route* rtept = RouteDataTransfer(child);
            insertBack(parsedData->routes, (Route*)rtept);
        }
        if (strcmp(name, "trk") == 0) {
            Track* trkpt = TrackDataTransfer(child);
            insertBack(parsedData->tracks, (Track*)trkpt);
        }
    }
    xmlFreeDoc(fileInfo);
    xmlCleanupParser();
    return parsedData;
}

/** Function to create a string representation of an GPX object.
 *@pre GPX object exists, is not null, and is valid
 *@post GPX has not been modified in any way, and a string representing the GPX contents has been created
 *@return a string contaning a humanly readable representation of an GPX object
 *@param obj - a pointer to an GPX struct
**/
char* GPXdocToString(GPXdoc* doc) {
    if (doc == NULL) {
        return NULL;
    }
    char *waypoints = toString(doc->waypoints);
    char *tracks = toString(doc->tracks);
    char *routes = toString(doc->routes);
    char *string = calloc(strlen(doc->namespace) + strlen(doc->creator) + strlen(waypoints) + strlen(routes) + strlen(tracks) +256, sizeof(char));
    sprintf(string, "GPXDoc creator: %s\nnamespace: %s\nversion: %0.2f\nwaypoints: %s\nroutes: %s\ntracks: %s\n", doc->creator, doc->namespace, doc->version, waypoints, routes, tracks);
    free(waypoints);
    free(routes);
    free(tracks);
    return string;
}

/** Function to delete doc content and free all the memory.
 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object had been freed
 *@return none
 *@param obj - a pointer to an GPX struct
**/
void deleteGPXdoc(GPXdoc* doc) {
    if (doc == NULL) {
        return;
    }
    free(doc->creator);
    freeList(doc->tracks);
    freeList(doc->routes);
    freeList(doc->waypoints);
    free(doc);
}

/* For the five "get..." functions below, return the count of specified entities from the file.
They all share the same format and only differ in what they have to count.

 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object has not been modified in any way
 *@return the number of entities in the GPXdoc object
 *@param obj - a pointer to an GPXdoc struct
 */


//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc) {
    if (doc == NULL){
        return 0;
    }
    return(doc->waypoints->length);
}

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc) {
    if (doc == NULL){
        return 0;
    }
    return(doc->routes->length);
}

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc) {
    if (doc == NULL){
        return 0;
    }
    return(doc->tracks->length);
}

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc) {
    if (doc == NULL){
        return 0;
    }
    int total = 0;
    for (Node *current = doc->tracks->head; current != NULL; current = current->next) {
        Track* track = current->data;
        total += track->segments->length;
    }
    return total;
}

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc) {
    if (doc == NULL){
        return 0;
    }
    int total = 0;
    for (Node *current = doc->waypoints->head; current != NULL; current = current->next) {
        Waypoint* waypoint = current->data;
        total += waypoint->otherData->length;
        if (strcmp(waypoint->name, "") != 0) {
            total++;
        }
    }
    for (Node *current = doc->routes->head; current != NULL; current = current->next) {
        Route* route = current->data;
        total += route->otherData->length;
        if (strcmp(route->name, "") != 0) {
            total++;
        }
        for (Node *cur = route->waypoints->head; cur != NULL; cur = cur->next) {
            Waypoint* wpt = cur->data;
            total += wpt->otherData->length;
            if (strcmp(wpt->name, "") != 0) {
                total++;
            }
        }
    }
    for (Node *current = doc->tracks->head; current != NULL; current = current->next) {
        Track* track = current->data;
        total += track->otherData->length;
        if (strcmp(track->name, "") != 0) {
            total++;
        }
        for (Node *cur = track->segments->head; cur != NULL; cur = cur->next) {
            TrackSegment* segment = cur->data;
            for (Node *c = segment->waypoints->head; c != NULL; c = c->next) {
                Waypoint* wpt = c->data;
                total += wpt->otherData->length;
                if (strcmp(wpt->name, "") != 0) {
                    total++;
                }
            }
        }
    }
    return total;
}

// Function that returns a waypoint with the given name.  If more than one exists, return the first one.
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return NULL;
    } else {
        Waypoint* wpt = findElement(doc->waypoints, &WaypointCompare, name);
        return wpt;
    }
}
// Function that returns a track with the given name.  If more than one exists, return the first one.
// Return NULL if the track does not exist
Track* getTrack(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return NULL;
    } else {
        Track* trk = findElement(doc->tracks, &TrackCompare, name);
        return trk;
    }
}
// Function that returns a route with the given name.  If more than one exists, return the first one.
// Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return NULL;
    } else {
        Route* rte = findElement(doc->routes, &RouteCompare, name);
        return rte;
    }
}

void deleteGpxData (void* data) {
    if (data == NULL) {
        return;
    }
    free((GPXData*)data);
}

char* gpxDataToString (void* data) {
    if (data == NULL) {
        return NULL;
    }
    GPXData *temp = data;
    char *string = calloc(strlen(temp->name) + strlen(temp->value) + 256, sizeof(char));
    sprintf(string, "GPXData name: %s\nvalue: %s\n", temp->name, temp->value);
    return string;
}

int compareGpxData (const void *first, const void *second) {
    //strncmp(first, second);
    return 0;
}

void deleteWaypoint(void* data) {
    if (data == NULL) {
        return;
    }
    Waypoint* temp = data;
    free(temp->name);
    freeList(temp->otherData);
    free(temp);
}

char* waypointToString(void* data) {
    if (data == NULL) {
        return NULL;
    }
    Waypoint *temp = data;
    char *stringLength = toString(temp->otherData);
    char *string = calloc(strlen(temp->name) + strlen(stringLength) + 256, sizeof(char));
    sprintf(string, "Waypoint name: %s\notherData: %s\nlat: %0.2f\nlong: %0.2f\n", temp->name, stringLength, temp->latitude, temp->longitude);
    free(stringLength);
    return string;
}

int compareWaypoints(const void *first, const void *second) {
    return 0;
}

void deleteRoute(void* data) {
    if (data == NULL) {
        return;
    }
    Route* temp = data;
    free(temp->name);
    freeList(temp->waypoints);
    freeList(temp->otherData);
    free(temp);
}

char* routeToString(void* data) {
    if (data == NULL) {
        return NULL;
    }
    Route *temp = data;
    char *waypoints = toString(temp->waypoints);
    char *otherData = toString(temp->otherData);
    char *string = calloc(strlen(waypoints) + strlen(otherData) + strlen(temp->name) + 256, sizeof(char));
    sprintf(string, "Route name: %s\nwaypoints: %s\notherData: %s\n", temp->name, waypoints, otherData);
    free(waypoints);
    free(otherData);
    return string;
}

int compareRoutes(const void *first, const void *second) {
    return 0;
}

void deleteTrackSegment(void* data) {
    if (data == NULL) {
        return;
    }
    TrackSegment* temp = data;
    freeList(temp->waypoints);
    free(temp);
}

char* trackSegmentToString(void* data) {
    if (data == NULL) {
        return NULL;
    }
    TrackSegment *temp = data;
    char *waypoints = toString(temp->waypoints);
    char *string = calloc(strlen(waypoints) + 256, sizeof(char));
    sprintf(string, "TrackSegment waypoints: %s\n", waypoints);
    free(waypoints);
    return string;
}

int compareTrackSegments(const void *first, const void *second) {
    return 0;
}

void deleteTrack(void* data) {
    if (data == NULL) {
        return;
    }
    Track* temp = data;
    free(temp->name);
    freeList(temp->segments);
    freeList(temp->otherData);
    free(temp);
}

char* trackToString(void* data) {
    if (data == NULL) {
        return NULL;
    }
    Track *temp = data;
    char *segments = toString(temp->segments);
    char *otherData = toString(temp->otherData);
    char *string = calloc(strlen(segments) + strlen(otherData) + strlen(temp->name) + 256, sizeof(char));
    sprintf(string, "Track name: %s\nsegments: %s\notherData : %s\n", temp->name, segments, otherData);
    free(segments);
    free(otherData);
    return string;
}

int compareTracks(const void *first, const void *second) {
    return 0;
}

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile) {
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    char *XMLFileName = fileName;
    char *XSDFileName = gpxSchemaFile;
    GPXdoc *gpxDoc = NULL;

    xmlLineNumbersDefault(1);
    ctxt = xmlSchemaNewParserCtxt(XSDFileName);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    doc = xmlReadFile(XMLFileName, NULL, 0);

    if (doc == NULL) {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
        return gpxDoc;
    } else {
        xmlSchemaValidCtxtPtr ctxt;
        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        int ret = xmlSchemaValidateDoc(ctxt, doc);
        if (ret == 0) {
            printf("%s validates\n", XMLFileName);
            gpxDoc = createGPXdoc(fileName);
        }
        xmlSchemaFreeValidCtxt(ctxt);
        xmlFreeDoc(doc);
    }

    if(schema != NULL) {
        xmlSchemaFree(schema);
    }
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    return gpxDoc;
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile) {
    if (doc == NULL || gpxSchemaFile == NULL) {
        return false;
    }
    bool status = false;
    xmlDocPtr xmlDoc;
    xmlNode *rootNode;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    char *XSDFileName = gpxSchemaFile;

    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    rootNode = gpxDocToXmlNode(doc);
    xmlDocSetRootElement(xmlDoc, rootNode);

    xmlLineNumbersDefault(1);
    ctxt = xmlSchemaNewParserCtxt(XSDFileName);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    xmlSchemaValidCtxtPtr ctxt2;
    ctxt2 = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxt2, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    int ret = xmlSchemaValidateDoc(ctxt2, xmlDoc);
    if (ret == 0) {
        status = true;
        printf("valid gpx: status true\n");
    }
    xmlSchemaFreeValidCtxt(ctxt2);
    xmlFreeDoc(xmlDoc);
    if (schema != NULL) {
        xmlSchemaFree(schema);
        // printf("schema free\n");      // this does print
    }

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
    // printf("heyooooooo\n");            //  also suddenly prints ?????
    return status;
}

bool writeGPXdoc(GPXdoc* doc, char* fileName) {
    if (doc == NULL || fileName == NULL) {
        return false;
    }
    xmlDoc *xmlDoc = NULL;
    xmlNode *rootNode = NULL;
    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    rootNode = gpxDocToXmlNode(doc);
    xmlDocSetRootElement(xmlDoc, rootNode);
    xmlSaveFormatFileEnc(fileName, xmlDoc, "ISO-8859-1", 1);
    return true;
}

float round10(float len) {
    int lowerBound = len;
    float upperBound = 0;

    while (upperBound < len) {
        upperBound += 10;
    }
    lowerBound = upperBound - 10;

    if ((upperBound - len) <= (len - lowerBound)) {
        return upperBound;
    } else {
        return upperBound - 10;
    }
}

float getRouteLen(const Route *rt) {
    if (rt == NULL) {
        return 0;
    }
    float sum = 0;
    for (Node *head = rt->waypoints->head; head->next != NULL; head = head->next) {
        Waypoint *w1 = head->data;
        Waypoint *w2 = head->next->data;
        sum += haversine(w1->latitude, w1->longitude, w2->latitude, w2->longitude);
    }
    return sum;
}

float getTrackLen(const Track *tr) {
    if (tr == NULL) {
        return 0;
    }
    float sum = 0;
    List *waypointList = initializeList(&waypointToString, &dummyDelete, &compareWaypoints);
    for (Node *head = tr->segments->head; head != NULL; head = head->next) {
        TrackSegment *seg1 = head->data;
        for (Node *head2 = seg1->waypoints->head; head2 != NULL; head2 = head2->next) {
            insertBack(waypointList, head2->data);
        }
    }
    for (Node *head = waypointList->head; head->next != NULL; head = head->next) {
        Waypoint *w1 = head->data;
        Waypoint *w2 = head->next->data;
        sum += haversine(w1->latitude, w1->longitude, w2->latitude, w2->longitude);
    }
    freeList(waypointList);
    return sum;
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return 0;
    }
    int sum = 0;
    double upperbound = len + delta;
    double lowerbound = len - delta;
    for (Node *head = doc->routes->head; head != NULL; head = head->next) {
        double distance = getRouteLen(head->data);
        if ((distance > lowerbound) && (distance < upperbound)) {
            sum++;
        }
    }
    return sum;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return 0;
    }
    int sum = 0;
    double upperbound = len + delta;
    double lowerbound = len - delta;
    for (Node *head = doc->tracks->head; head != NULL; head = head->next) {
        double distance = getTrackLen(head->data);
        if ((distance > lowerbound) && (distance < upperbound)) {
            sum++;
        }
    }
    return sum;
}

bool isLoopRoute(const Route* rt, float delta) {
    if (rt == NULL || delta < 0) {
        return false;
    }
    if (rt->waypoints->length < 4) {
        return false;
    }
    Node *head = rt->waypoints->head;
    Node *tail = rt->waypoints->tail;
    Waypoint *first = head->data;
    Waypoint *last = tail->data;
    float distance =  haversine(first->latitude, first->longitude, last->latitude, last->longitude);

    if (distance <= delta) {
        return true;
    } else {
        return false;
    }
}

bool isLoopTrack(const Track *tr, float delta) {
    if (tr == NULL || delta < 0) {
        return false;
    }
    if (tr->segments->length <= 0) {
        return false;
    }
    int totalWaypoints = 0;
    for (Node * head = tr->segments->head; head != NULL; head = head->next) {
        TrackSegment *trkseg1 = head->data;
        totalWaypoints += getLength(trkseg1->waypoints);
    }
    if (totalWaypoints < 4) {
        return false;
    }
    Node *head = tr->segments->head;
    Node *tail = tr->segments->tail;
    TrackSegment *first = head->data;
    TrackSegment *last = tail->data;
    head = first->waypoints->head;
    tail = last->waypoints->tail;
    Waypoint *w1 = head->data;
    Waypoint *w2 = tail->data;
    float distance =  haversine(w1->latitude, w1->longitude, w2->latitude, w2->longitude);

    if (distance <= delta) {
        return true;
    } else {
        return false;
    }
    return true;
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong,
float destLat, float destLong, float delta) {
    if (doc == NULL || delta < 0) {
        return NULL;
    }
    List *deltaRoutes = initializeList(&routeToString, &dummyDelete, &compareRoutes);
    for (Node *head = doc->routes->head; head != NULL; head = head->next) {
        Route *rte = head->data;
        Waypoint *first = rte->waypoints->head->data;
        Waypoint *last = rte->waypoints->tail->data;
        double startDist = haversine(first->latitude, first->longitude, sourceLat, sourceLong);
        double endDist = haversine(last->latitude, last->longitude, destLat, destLong);
        if (startDist <= delta && endDist <= delta) {
            //add rte to the list we are returning
            insertBack(deltaRoutes, rte);
        }
    }
    if (getLength(deltaRoutes) == 0) {
        freeList(deltaRoutes);
        return NULL;
    }
    return deltaRoutes;
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong,
float destLat, float destLong, float delta) {
    if (doc == NULL || delta < 0) {
        return NULL;
    }
    List *deltaRoutes = initializeList(&trackToString, &dummyDelete, &compareTracks);
    // Node *head = tr->segments->head;
    // Node *tail = tr->segments->tail;
    for (Node *head = doc->tracks->head; head != NULL; head = head->next) {
        Track *tr = head->data;
        TrackSegment *first = tr->segments->head->data;
        TrackSegment *last = tr->segments->tail->data;
        Waypoint *wp1 = (Waypoint *)first->waypoints->head;
        Waypoint *wp2 = (Waypoint *)last->waypoints->tail;
        double startDist = haversine(wp1->latitude, wp1->longitude, sourceLat, sourceLong);
        double endDist = haversine(wp2->latitude, wp2->longitude, destLat, destLong);
        if (startDist <= delta && endDist <= delta) {
            //add trk to the list we are returning
            insertBack(deltaRoutes, tr);
        }
    }
    if (getLength(deltaRoutes) == 0) {
        freeList(deltaRoutes);
        return NULL;
    }
    return NULL;
}

char* trackToJSON(const Track *tr) {
    if (tr == NULL) {
        return "{}";
    }
    char *name = tr->name;
    if (name == NULL) {
        name = "None";
    }
    double trackLen = getTrackLen(tr);
    trackLen = round10(trackLen);
    bool loopStat = isLoopTrack(tr, 10);  //  assuming: 'use the tolerance of 10m' for delta
    char *segments = toString(tr->segments);
    char *otherData = toString(tr->otherData);
    char *string = calloc(strlen(tr->name) + strlen(segments) + strlen(otherData) + 256, sizeof(char));
    sprintf(string, "{\"name\":\"%s\",\"len\":%0.0f.0,\"loop\":%s}",
    name, trackLen, boolStatus(loopStat));
    free(segments);
    free(otherData);
    return string;
}

char* routeToJSON(const Route *rt) {
    if (rt == NULL) {
        return "{}";
    }
    char *name = rt->name;
    if (name == NULL) {
        name = "None";
    }
    double routeLen = getRouteLen(rt);
    routeLen = round10(routeLen);
    int numVal = getLength(rt->waypoints);
    bool loopStat = isLoopRoute(rt, 10);    //  assuming: 'use the tolerance of 10m' for delta
    char *otherData = toString(rt->otherData);
    char *waypoints = toString(rt->waypoints);
    char *string = calloc(strlen(otherData) + strlen(waypoints) + strlen(rt->name) + 256, sizeof(char));
    sprintf(string, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.0f.0,\"loop\":%s}",
    name, numVal, routeLen, boolStatus(loopStat));
    free(otherData);
    free(waypoints);
    return string;
}

char* routeListToJSON(const List *list) {
    if (list == NULL || list->head == NULL) {
        return "[]";
    }
    Route *rt = (Route *)list->head;
    if (rt == NULL) {
        return "[]";
    }
    char *otherDataRt = toString(rt->otherData);
    char *waypointRt = toString(rt->waypoints);
    char **routeStr = calloc(strlen(rt->name) + strlen(otherDataRt) + strlen(waypointRt) + 256, sizeof(char *));
    int i = 0;
    for (Node *head = list->head; head != NULL; head = head->next) {
        Route *rte = head->data;
        routeStr[i] = routeToJSON(rte);
        if (head->next != NULL) {
            strcat(routeStr[i+1], ",");
            i++;
        }
        i++;
    }
    char *string = calloc(strlen(*routeStr) + 256, sizeof(char));
    char *temp = calloc(strlen(*routeStr) + 256, sizeof(char));
    for (int j = 0; j < i; j++) {
        strcat(temp, routeStr[j]);
    }
    sprintf(string, "[%s]", temp);
    free(otherDataRt);
    free(waypointRt);
    free(rt);
    free(temp);
    free(routeStr);
    return string;
}

char* trackListToJSON(const List *list) {
    if (list == NULL || list->head == NULL) {
        return "[]";
    }
    Track *tr = (Track *)list->head;
    if (tr == NULL) {
        return "[]";
    }
    char *otherData = toString(tr->otherData);
    char *segments = toString(tr->segments);
    char **trackStr = calloc(strlen(tr->name) + strlen(otherData) + strlen(segments) + 256, sizeof(char *));
    int i = 0;
    for (Node *head = list->head; head != NULL; head = head->next) {
        Track *trk = head->data;
        trackStr[i] = trackToJSON(trk);
        if (head->next != NULL) {
            strcat(trackStr[i+1], ",");
            i++;
        }
        i++;
    }
    char *string = calloc(strlen(*trackStr) + 256, sizeof(char));
    char *temp = calloc(strlen(*trackStr) + 256, sizeof(char));
    for (int j = 0; j < i; j++) {
        strcat(temp, trackStr[j]);
    }
    sprintf(string, "[%s]", temp);
    free(otherData);
    free(segments);
    free(tr);
    free(temp);
    free(trackStr);
    return string;
}

char* GPXtoJSON(const GPXdoc* gpx) {
    if (gpx == NULL) {
        return "{}";
    }
    double version = gpx->version;
    char *creator = (char *)gpx->creator;
    int waypoints = getNumWaypoints(gpx);
    int tracks = getNumTracks(gpx);
    int routes = getNumRoutes(gpx);
    char *string = calloc((int)(version) + waypoints + tracks + routes + 256, sizeof(char));
    sprintf(string, "{\"version\":%0.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}",
    version, creator, waypoints, routes, tracks);
    free(creator);
    return string;
}

//----- bonus functions --------

void addWaypoint(Route *rt, Waypoint *pt) {

}

void addRoute(GPXdoc* doc, Route* rt) {

}

GPXdoc* JSONtoGPX(const char* gpxString) {
    return NULL;
}

Waypoint* JSONtoWaypoint(const char* gpxString) {
    return NULL;
}

Route* JSONtoRoute(const char* gpxString) {
    return NULL;
}

// gcc -I/usr/include/libxml2 GPXParser.c -o GPXParser -lxml2
