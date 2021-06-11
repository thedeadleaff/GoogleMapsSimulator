#ifndef GPX_HELPER_H
#define GPX_HELPER_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include <stdbool.h>
#include "LinkedListAPI.h"
#include "GPXParser.h"

GPXData* GPXDataTransfer (xmlNode* originalNode);
Waypoint* WaypointDataTransfer (xmlNode* originalNode);
Route* RouteDataTransfer(xmlNode* originalNode);
TrackSegment* TrackSegmentDataTransfer(xmlNode* originalNode);
Track* TrackDataTransfer(xmlNode* originalNode);
bool TrackCompare(const void *first, const void *second);
bool RouteCompare(const void *first, const void *second);
bool WaypointCompare(const void *first, const void *second);

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile);
bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile);
bool writeGPXdoc(GPXdoc* doc, char* fileName);
xmlNode* gpxDataToXmlNode (GPXData *data, xmlNs *namespace);
xmlNode* gpxNameToXmlNode (char *name, xmlNs *namespace);
xmlNode* waypointToXmlNode (Waypoint *wpt, xmlNs *namespace, char *waypointType);
xmlNode* routeToXmlNode (Route *rte, xmlNs *namespace);
xmlNode* trksegToXmlNode (TrackSegment *trkseg, xmlNs *namespace);
xmlNode* gpxDocToXmlNode (GPXdoc *doc);
double haversine (double lat1, double lon1, double lat2, double lon2);
void dummyDelete (void *data);
char *boolStatus (bool stat);

char *uploadToJSON (char *filename);
char *validateDoc (char *filepath, char *schemapath);

#endif
