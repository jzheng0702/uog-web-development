/*Jingjing Zheng
ID: 1043704
email:jzheng06@uoguelph.ca*/
#ifndef HELPERFUNCTION_H
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

void initializeAllLists(Group* groupPtr);
void buildRectForGroup(xmlNode* curNode,Group* groupPtr);
void buildRect(xmlNode* curNode,SVGimage* imagePtr);
void buildCircle(xmlNode* curNode,SVGimage* imagePtr);
void buildCircleForGroup(xmlNode* curNode,Group* groupPtr);
void buildPath(xmlNode* curNode,SVGimage* imagePtr);
void buildPathForGroup(xmlNode* curNode,Group* groupPtr) ;
void buildGroupForGroup(xmlNode* myNode,Group* groupPtr);
void buildGroup(xmlNode* myNode,SVGimage* imagePtr);
void initialSVG(xmlNode* root_element,SVGimage* imagePtr);
void initializeAllList(SVGimage* imagePtr);
void dummyDelete(void* data);
void getRectsForGroup(Group* group, List* rects);
void getCirclesForGroup(Group* group, List* circles);
void getPathsForGroup(Group* group, List* paths);
void getGroupsForGroup(Group* group, List* groups);
xmlDocPtr createTrees(SVGimage* image);
void buildGroupNode(xmlNodePtr root_node,Group* group);
bool attrCheck(List* otherAttributes);
bool rectCheck(List* rectangles);
bool cirCheck(List* circles);
bool pathCheck(List* paths);
bool groupCheck(List* groups);
void otherAttr(List* otherAttributes,Attribute* newAttribute);
void updateCircle(List* otherAttributes,Attribute* newAttribute,int index);
void updateRect(List* rects,Attribute* newAttribute,int index);
void updatePath(List* paths,Attribute* newAttribute,int index);
void updateGroup(List* groups,Attribute* newAttribute,int index);

#endif
