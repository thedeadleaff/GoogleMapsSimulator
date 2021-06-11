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
#include <math.h>
#include <stdbool.h>

GPXData* GPXDataTransfer (xmlNode* originalNode) {
    if (originalNode == NULL) {
        return NULL;
    }
    GPXData *currentXML = calloc(1, sizeof(GPXData)+256);
    char *name = (char *)originalNode->name;
    char *value = (char*)originalNode->children->content;
    if (name != NULL) {
        strcpy(currentXML->name, name);
    } else {
        strcpy(currentXML->name, " ");
    }
    if (value != NULL) {
        strcpy(currentXML->value, value);
    } else {
        strcpy(currentXML->value, " ");
    }
    return currentXML;
}

Waypoint* WaypointDataTransfer (xmlNode* originalNode) {
    if (originalNode == NULL) {
        return NULL;
    }
    char *name = NULL;
    double lat = 0.0;
    double lng = 0.0;

    //accessing the name from the children
    for (xmlNode *child = originalNode->children; child != NULL; child = child->next) {
        char *childName = (char *)child->name;
        if (strcmp(childName, "name") == 0) {
            name = (char *)child->children->content;
        }
    }
    //accessing the attributes (lat and long)
    xmlAttr *attr;
    for (attr = originalNode->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);
        if (strcmp(attrName, "lat") == 0) {
            lat = atof(cont);
        }
        if (strcmp(attrName, "lon") == 0) {
            lng = atof(cont);
        }
    }
    Waypoint *wpt = calloc(1, sizeof(Waypoint));
    wpt->name = calloc(256, sizeof(char));
    if (name != NULL) {
        strcpy(wpt->name, name);
    }
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    // strcpy(wpt->name, name);
    wpt->latitude = lat;
    wpt->longitude = lng;

    //get all other data from children and add to list called otherData
    for (xmlNode *child = originalNode->children; child != NULL; child = child->next) {
        char *curChild = (char *)child->name;
        if ((strcmp(curChild, "name") != 0) && (strcmp(curChild, "text") != 0)) {
            GPXData* curData = GPXDataTransfer(child);
            insertBack(wpt->otherData, (GPXData*)curData);
        }
    }
    return wpt;
}

Route* RouteDataTransfer(xmlNode* originalNode) {
    if ((originalNode == NULL) ) {
        return NULL;
    }
    char *name = NULL;

    for (xmlNode *child = originalNode->children; child != NULL; child = child->next) {
        char *childName = (char *)child->name;
        if (strcmp(childName, "name") == 0) {
            name = (char *)child->children->content;
        }
    }
    Route* rte = calloc(1, sizeof(Route));
    rte->name = calloc(256, sizeof(char));
    if (name != NULL) {
        strcpy(rte->name, name);
    }
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    for(xmlNode *child = originalNode->children; child != NULL; child = child->next) {
        char *curChild = (char *)child->name;
        if ((strcmp(curChild, "name") != 0) && (strcmp(curChild, "text") != 0)) {
            if (strcmp(curChild, "rtept") == 0) {
                Waypoint* wptData = WaypointDataTransfer(child);
                insertBack(rte->waypoints, (Waypoint*)wptData);
            } else {
                GPXData* curData = GPXDataTransfer(child);
                insertBack(rte->otherData, (GPXData*)curData);
            }
        }
    }
    return rte;
}

TrackSegment* TrackSegmentDataTransfer(xmlNode* originalNode) {
    if (originalNode == NULL) {
        return NULL;
    }
    TrackSegment* trkseg = calloc(1, sizeof(TrackSegment));
    trkseg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    for (xmlNode *child = originalNode->children; child != NULL; child = child->next) {
        char *childName = (char *)child->name;
        if (strcmp(childName, "trkpt") == 0) {
            Waypoint* waypt = WaypointDataTransfer(child);
            insertBack(trkseg->waypoints, (Waypoint*)waypt);
        }
    }
    return trkseg;
}

Track* TrackDataTransfer(xmlNode* originalNode) {
    if (originalNode == NULL) {
        return NULL;
    }
    char *name = NULL;

    for (xmlNode *child = originalNode->children; child != NULL; child = child->next) {
        char *childName = (char *)child->name;
        if (strcmp(childName, "name") == 0) {
            name = (char *)child->children->content;
        }
    }

    Track* trk = calloc(1, sizeof(Track));
    trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    trk->name = calloc(256, sizeof(char));
    if (name != NULL) {
        strcpy(trk->name, name);
    }

    for(xmlNode *child = originalNode->children; child != NULL; child = child->next) {
        char *curChild = (char *)child->name;
        if ((strcmp(curChild, "name") != 0) && (strcmp(curChild, "text") != 0)) {
            if (strcmp(curChild, "trkseg") == 0) {
                TrackSegment* trksegData = TrackSegmentDataTransfer(child);
                insertBack(trk->segments, (TrackSegment*)trksegData);
            } else {
                GPXData* curData = GPXDataTransfer(child);
                insertBack(trk->otherData, (GPXData*)curData);
            }
        }
    }
    return trk;
}

bool TrackCompare(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return false;
    }
    Track* track = (Track*)first;
    char* name = (char *)second;
    if (strcmp(track->name, name) == 0) {
        return true;
    } else {
        return false;
    }
}

bool RouteCompare(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return false;
    }
    Route* route = (Route*)first;
    char* name = (char *)second;
    if (strcmp(route->name, name) == 0) {
        return true;
    } else {
        return false;
    }
}

bool WaypointCompare(const void *first, const void *second) {
    if (first == NULL || second == NULL) {
        return false;
    }
    Waypoint* waypoint = (Waypoint*)first;
    char* name = (char *)second;
    if (strcmp(waypoint->name, name) == 0) {
        return true;
    } else {
        return false;
    }
}

xmlNode* gpxDataToXmlNode (GPXData *data, xmlNs *namespace) {
    xmlNode *node = NULL;
    xmlNode *node1 = NULL;
    node = xmlNewNode(NULL, BAD_CAST data->name);
    xmlSetNs(node, namespace);
    node1 = xmlNewText(BAD_CAST data->value);
    xmlAddChild(node, node1);
    return node;
}

xmlNode* gpxNameToXmlNode (char *name, xmlNs *namespace) {
    xmlNode *node = NULL;
    xmlNode *child = NULL;
    node = xmlNewNode(NULL, BAD_CAST "name");
    xmlSetNs(node, namespace);
    child = xmlNewText(BAD_CAST name);
    xmlAddChild(node, child);
    return node;
}

xmlNode* waypointToXmlNode (Waypoint *wpt, xmlNs *namespace, char *waypointType) {
    xmlNode *node = NULL;
    xmlNode *child = NULL;
    node = xmlNewNode(NULL, BAD_CAST waypointType);
    xmlSetNs(node, namespace);
    char arr[50];
    strcpy(arr, "\0");
    sprintf(arr, "%.6f", wpt->latitude);
    xmlNewProp(node, BAD_CAST "lat", BAD_CAST arr);
    strcpy(arr, "\0");
    sprintf(arr, "%.6f", wpt->longitude);
    xmlNewProp(node, BAD_CAST "lon", BAD_CAST arr);
    for (Node *head = wpt->otherData->head; head != NULL; head = head->next) {
        child = gpxDataToXmlNode(head->data, namespace);
        xmlAddChild(node, child);
    }
    if (strlen(wpt->name) > 0) {
        xmlNode *nameNode = gpxNameToXmlNode(wpt->name, namespace);
        xmlAddChild(node, nameNode);
    }
    return node;
}

xmlNode* routeToXmlNode (Route *rte, xmlNs *namespace) {
    xmlNode *node = NULL;
    xmlNode *child = NULL;
    node = xmlNewNode(NULL, BAD_CAST "rte");
    xmlSetNs(node, namespace);
    if (strlen(rte->name) > 0) {
        xmlNode *nameNode = gpxNameToXmlNode(rte->name, namespace);
        xmlAddChild(node, nameNode);
    }
    for (Node *head = rte->otherData->head; head != NULL; head = head->next) {
        child = gpxDataToXmlNode(head->data, namespace);
        xmlAddChild(node, child);
    }
    for (Node *head = rte->waypoints->head; head != NULL; head = head->next) {
        child = waypointToXmlNode(head->data, namespace, "rtept");
        xmlAddChild(node, child);
    }
    return node;
}

xmlNode* trksegToXmlNode (TrackSegment *trkseg, xmlNs *namespace) {
    xmlNode *node = NULL;
    xmlNode *child = NULL;
    node = xmlNewNode(NULL, BAD_CAST "trkseg");
    xmlSetNs(node, namespace);
    for (Node *head = trkseg->waypoints->head; head != NULL; head = head->next) {
        child = waypointToXmlNode(head->data, namespace, "trkpt");
        xmlAddChild(node, child);
    }
    return node;
}

xmlNode* trkToXmlNode (Track *trk, xmlNs *namespace) {
    xmlNode *node = NULL;
    xmlNode *child = NULL;
    node = xmlNewNode(NULL, BAD_CAST "trk");
    xmlSetNs(node, namespace);
    if (strlen(trk->name) > 0) {
        xmlNode *nameNode = gpxNameToXmlNode(trk->name, namespace);
        xmlAddChild(node, nameNode);
    }
    for (Node *head = trk->otherData->head; head != NULL; head = head->next) {
        child = gpxDataToXmlNode(head->data, namespace);
        xmlAddChild(node, child);
    }
    for (Node *head = trk->segments->head; head != NULL; head = head->next) {
        child = trksegToXmlNode(head->data, namespace);
        xmlAddChild(node, child);
    }
    return node;
}

xmlNode* gpxDocToXmlNode (GPXdoc *doc) {
    xmlNode *node = NULL;
    xmlNode *child = NULL;
    node = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlNs *namespace = xmlNewNs(node, BAD_CAST doc->namespace, NULL);
    char arr[50];
    strcpy(arr, "\0");
    sprintf(arr, "%.1f", doc->version);
    xmlNewProp(node, BAD_CAST "version", BAD_CAST arr);
    xmlNewProp(node, BAD_CAST "creator", BAD_CAST doc->creator);
    xmlSetNs(node, namespace);
    for (Node *head = doc->waypoints->head; head != NULL; head = head->next) {
        child = waypointToXmlNode(head->data, namespace, "wpt");
        xmlAddChild(node, child);
    }
    for (Node *head = doc->routes->head; head != NULL; head = head->next) {
        child = routeToXmlNode(head->data, namespace);
        xmlAddChild(node, child);
    }
    for (Node *head = doc->tracks->head; head != NULL; head = head->next) {
        child = trkToXmlNode(head->data, namespace);
        xmlAddChild(node, child);
    }
    return node;
}

double haversine (double lat1, double lon1, double lat2, double lon2) {
    double r = 6371000;    //to make the output represent m
    double phi1 = lat1 * M_PI / 180;
    double phi2 = lat2 * M_PI / 180;
    double deltaphi = (lat2-lat1) * M_PI/180;
    double deltalambda = (lon2-lon1) * M_PI/180;

    double a = pow(sin(deltaphi / 2), 2) + cos(phi1) * cos(phi2) * pow(sin(deltalambda / 2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return c * r;
}

void dummyDelete (void *data) {

}

char *boolStatus (bool stat) {
    if (stat == true) {
        return "true";
    } else if (stat == false) {
        return "false";
    }
    return "true";
}

char *uploadToJSON (char *filename) {
    //char *name = filename + 8;
    GPXdoc *gpx = createGPXdoc(filename);
    if (gpx == NULL) {
        return filename;
    }
    char *output = GPXtoJSON(gpx);
    return output;
}

char *validateDoc (char *filepath, char *schemapath) {
    if (filepath == NULL || schemapath == NULL) {
        return filepath;
    }
    //char *name = filepath + 8;
    GPXdoc *gpx = createGPXdoc(filepath);
    //char *schema = schemapath + 11;
    bool status = validateGPXDoc(gpx, schemapath);
    if (status == false) {
        return "false";
    }
    return "true";
}

int waypoints () {
    return 0;
}

int routes () {
    return 0;
}

int tracks () {
    return 0;
}

char *createGPXFileFromJSON () {
    return NULL;
}
