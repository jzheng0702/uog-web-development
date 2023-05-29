/*Jingjing Zheng
ID: 1043704
email:jzheng06@uoguelph.ca*/
#include "SVGParser.h"
#include "helperFunction.h"

char* createSVGFileFromJSON(const char* svgString, char* filename) {
  SVGimage* img = JSONtoSVG(svgString);
  if (validateSVGimage(img,"parser/svg.xsd") == false || img == NULL) {
    return "can not be validate";
  }

  if (writeSVGimage(img,filename) == true) {
    return "yes";
  } else {
    return "cannot write";
  }

}
bool validateSVGimage(SVGimage* image, char* schemaFile) {
  if (image == NULL || schemaFile == NULL || strcmp(schemaFile," ") == 0) {
    return false;
  }

  /*Part 1: validates SVGParser.h*/
  /*SVG image*/
  if (image -> rectangles == NULL) {
    return false;
  } else {
    if (rectCheck(image -> rectangles) == false) {
      return false;
    }
  }

  if (image -> circles == NULL) {
    return false;
  } else {
    if (cirCheck(image -> circles) == false) {
      return false;
    }
  }

  if (image -> paths == NULL) {
    return false;
  } else {
    if (pathCheck(image -> paths) == false) {
      return false;
    }
  }

  if (image -> groups == NULL) {
    return false;
  } else {
    if (groupCheck(image -> groups) == false) {
      return false;
    }
  }

  if (image -> otherAttributes == NULL) {
    return false;
  } else {
    if (attrCheck(image -> otherAttributes) == false) {
      return false;
    }
  }


  /*Part 2: validate SVGimage once converted to XML*/
  xmlDocPtr doc = createTrees(image);
  if (doc == NULL) {
    return false;
  }

  xmlSchemaPtr schema = NULL;
  xmlSchemaParserCtxtPtr ctxt;

  xmlLineNumbersDefault(1);

  ctxt = xmlSchemaNewParserCtxt(schemaFile);

  xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  schema = xmlSchemaParse(ctxt);
  xmlSchemaFreeParserCtxt(ctxt);

  if (schema == NULL) {
    xmlFreeDoc(doc);
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    return false;
  }

  xmlSchemaValidCtxtPtr valid_ctxt;
  int ret;

  valid_ctxt = xmlSchemaNewValidCtxt(schema);
  xmlSchemaSetValidErrors(valid_ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  ret = xmlSchemaValidateDoc(valid_ctxt, doc);
  if (ret == 0) {
    //printf("validates!\n");
  }
  else if (ret > 0) {
    //printf("Fails to validate\n");
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    xmlMemoryDump();
    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlSchemaFreeValidCtxt(valid_ctxt);
    return false;
  }
  else {
    //printf("Validation generated an internal error\n");
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    xmlMemoryDump();
    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlSchemaFreeValidCtxt(valid_ctxt);
    return false;
  }
  xmlSchemaFreeValidCtxt(valid_ctxt);


  /*free the document */
  xmlSchemaFree(schema);
  xmlSchemaCleanupTypes();
  xmlMemoryDump();
  xmlFreeDoc(doc);
  xmlCleanupParser();




  return true;
}

SVGimage* createValidSVGimage(char* fileName, char* schemaFile){
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  SVGimage* imagePtr = (SVGimage*)calloc(1,sizeof(SVGimage));
  xmlSchemaPtr schema = NULL;
  xmlSchemaParserCtxtPtr ctxt;

  /*Check to see if the fileName is valid or not*/
  if (fileName == NULL || strcmp(fileName," ") == 0 || schemaFile == NULL || strcmp(schemaFile," ") == 0) {
    free(imagePtr);
    return NULL;
  }

  xmlLineNumbersDefault(1);

  ctxt = xmlSchemaNewParserCtxt(schemaFile);

  xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  schema = xmlSchemaParse(ctxt);
  xmlSchemaFreeParserCtxt(ctxt);

  if (schema == NULL) {
    xmlSchemaCleanupTypes();
    free(imagePtr);
    xmlCleanupParser();
    return NULL;
  }

  doc = xmlReadFile(fileName, NULL, 0);

  if (doc == NULL) {
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    free(imagePtr);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return NULL;
  }

  xmlSchemaValidCtxtPtr valid_ctxt;
  int ret;

  valid_ctxt = xmlSchemaNewValidCtxt(schema);
  xmlSchemaSetValidErrors(valid_ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  ret = xmlSchemaValidateDoc(valid_ctxt, doc);
  xmlSchemaFreeValidCtxt(valid_ctxt);
  if (ret == 0) {
    //printf("%s validates\n", fileName);
  }
  else if (ret > 0) {
    //printf("%s fails to validate\n", fileName);
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    xmlMemoryDump();
    xmlFreeDoc(doc);
    xmlCleanupParser();
    free(imagePtr);
    return NULL;
  }
  else {
    //printf("%s validation generated an internal error\n", fileName);
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    xmlMemoryDump();
    xmlFreeDoc(doc);
    xmlCleanupParser();
    free(imagePtr);
    return NULL;
  }


  /*Get the root element node */
  root_element = xmlDocGetRootElement(doc);
  initializeAllList(imagePtr);

  if (root_element == NULL || root_element->ns == NULL|| root_element->ns->href == NULL) {
    deleteSVGimage(imagePtr);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return NULL;
  }

  char* data = (char*)root_element->ns->href;
  strcpy(imagePtr -> namespace,data);
  initialSVG(root_element,imagePtr);

  /*free the document */
  xmlSchemaFree(schema);
  xmlSchemaCleanupTypes();
  xmlMemoryDump();
  xmlFreeDoc(doc);
  xmlCleanupParser();


  return imagePtr;
}

bool writeSVGimage(SVGimage* image, char* fileName) {
  if (image == NULL || fileName == NULL) {
    return false;
  }

  /*if (validateSVGimage(image,"svg.xsd") == false) {
  return false;
}*/

xmlDocPtr doc = createTrees(image);
if (doc == NULL) {
  return false;
}

/*
* Dumping document to stdio or file
*/
xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1);

/*free the document */
xmlFreeDoc(doc);

/*
*Free the global variables that may
*have been allocated by the parser.
*/
xmlCleanupParser();

/*
* this is to debug memory for regression tests
*/
xmlMemoryDump();
return true;
}

void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
  if (image == NULL || newAttribute == NULL) {
    //printf("setA something is wrong\n");
  } else {
    /*If the type is SVG image*/
    if (elemType == SVG_IMAGE) {
      otherAttr(image -> otherAttributes,newAttribute);
    } else if (elemType == CIRC) {
      updateCircle(image -> circles,newAttribute,elemIndex);
    } else if (elemType == RECT) {
      updateRect(image -> rectangles,newAttribute,elemIndex);
    } else if (elemType == PATH) {
      updatePath(image -> paths,newAttribute,elemIndex);
    } else {
      updateGroup(image -> groups,newAttribute,elemIndex);
    }
  }

}

void addComponent(SVGimage* image, elementType type, void* newElement) {
  if (image == NULL || validateSVGimage(image,"svg.xsd") == false || newElement == NULL) {
    //printf("addC something is wrong\n");
  } else {
    if (type == PATH) {
      insertBack(image -> paths, (Path*)newElement);
    } else if (type == CIRC) {
      insertBack(image -> circles, (Circle*)newElement);
    } else if (type == RECT) {
      insertBack(image -> rectangles, (Rectangle*)newElement);
    }
  }
}

char* attrToJSON(const Attribute *a) {
  if (a == NULL|| a -> name == NULL || a -> value == NULL) {
    char *result = (char*)malloc(5);
    strcpy(result,"{}");
    return result;
  }

  char *result = (char*)malloc(500);
  strcpy(result,"{\"name\":\"");
  strcat(result, a -> name);
  strcat(result,"\",\"value\":\"");
  strcat(result,a -> value);
  strcat(result,"\"}");

  return result;
}


char* circleToJSON(const Circle *c) {
  //{"cx":xVal,"cy":yVal,"r":rVal,"numAttr":attVal,"units":"unitStr"}
  if (c == NULL || c -> otherAttributes == NULL) {
    char *result = (char*)malloc(5);
    strcpy(result,"{}");
    return result;
  }
  char *result = (char*)malloc(500);
  if (strcmp(c -> units, "") != 0) {
    sprintf(result,"{\"cx\":%.2f,\"cy\":%.2f,\"r\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}",c -> cx, c -> cy, c -> r, getLength(c -> otherAttributes), c -> units);
  } else {
    sprintf(result,"{\"cx\":%.2f,\"cy\":%.2f,\"r\":%.2f,\"numAttr\":%d,\"units\":\"\"}",c -> cx, c -> cy, c -> r, getLength(c -> otherAttributes));
  }

  return result;
}

char* rectToJSON(const Rectangle *r) {
  if (r == NULL || r -> otherAttributes == NULL) {
    char *result = (char*)malloc(5);
    strcpy(result,"{}");
    return result;
  }
  char *result = (char*)malloc(500);
  if (strcmp(r -> units, "") != 0) {
    sprintf(result,"{\"x\":%.2f,\"y\":%.2f,\"w\":%.2f,\"h\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}",r -> x, r -> y, r -> width, r -> height, getLength(r -> otherAttributes), r -> units);
  } else {
    sprintf(result,"{\"x\":%.2f,\"y\":%.2f,\"w\":%.2f,\"h\":%.2f,\"numAttr\":%d,\"units\":\"\"}",r -> x, r -> y, r -> width, r -> height, getLength(r -> otherAttributes));
  }

  return result;
}

char* pathToJSON(const Path *p) {
  if (p == NULL || p -> data == NULL|| p -> otherAttributes == NULL) {
    char *result = (char*)malloc(5);
    strcpy(result,"{}");
    return result;
  }
  char *result = (char*)malloc(500);
  sprintf(result,"{\"d\":\"%s\",\"numAttr\":%d}",p -> data, getLength(p -> otherAttributes));
  return result;
}


char* groupToJSON(const Group *g) {
  if (g == NULL || g -> otherAttributes == NULL) {
    char *result = (char*)malloc(5);
    strcpy(result,"{}");
    return result;
  }
  int length = getLength(g -> rectangles)
  + getLength(g -> circles)
  + getLength(g -> paths)
  + getLength(g -> groups);

  char *result = (char*)malloc(500);
  sprintf(result,"{\"children\":%d,\"numAttr\":%d}",length, getLength(g -> otherAttributes));
  return result;
}

char* attrListToJSON(const List *list) {
  if (list == NULL || getLength((List*)list) == 0) {
    char *result = (char*)malloc(5);
    strcpy(result,"[]");
    return result;
  }
  int length = getLength((List*)list);
  char* result = (char*)malloc(500 * length);
  ListIterator iter = createIterator((List*)list);
  Attribute* attr = NULL;
  int count = 0;
  strcpy(result,"[");
  while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
    char* buffer = attrToJSON(attr);
    strcat(result,buffer);
    count++;
    if (count != length){
      strcat(result,",");
    }
    free(buffer);
  }
  strcat(result,"]");

  return result;
}

char* circListToJSON(const List *list) {
  if (list == NULL || getLength((List*)list) == 0) {
    char *result = (char*)malloc(5);
    strcpy(result,"[]");
    return result;
  }
  int length = getLength((List*)list);
  char* result = (char*)malloc(500 * length);
  ListIterator iter = createIterator((List*)list);
  Circle* circle = NULL;
  int count = 0;
  strcpy(result,"[");
  while ((circle = (Circle*)nextElement(&iter)) != NULL) {
    char* buffer = circleToJSON(circle);
    strcat(result,buffer);
    count++;
    if (count != length){
      strcat(result,",");
    }
    free(buffer);
  }
  strcat(result,"]");

  return result;
}

char* rectListToJSON(const List *list) {
  if (list == NULL || getLength((List*)list) == 0) {
    char *result = (char*)malloc(5);
    strcpy(result,"[]");
    return result;
  }
  int length = getLength((List*)list);
  char* result = (char*)malloc(500 * length);
  ListIterator iter = createIterator((List*)list);
  Rectangle* rect = NULL;
  int count = 0;
  strcpy(result,"[");
  while ((rect = (Rectangle*)nextElement(&iter)) != NULL) {
    char* buffer = rectToJSON(rect);
    strcat(result,buffer);
    count++;
    if (count != length){
      strcat(result,",");
    }
    free(buffer);
  }
  strcat(result,"]");

  return result;
}

char* pathListToJSON(const List *list) {
  if (list == NULL || getLength((List*)list) == 0) {
    char *result = (char*)malloc(5);
    strcpy(result,"[]");
    return result;
  }
  int length = getLength((List*)list);
  char* result = (char*)malloc(500 * length);
  ListIterator iter = createIterator((List*)list);
  Path* path = NULL;
  int count = 0;
  strcpy(result,"[");
  while ((path = (Path*)nextElement(&iter)) != NULL) {
    char* buffer = pathToJSON(path);
    strcat(result,buffer);
    count++;
    if (count != length){
      strcat(result,",");
    }
    free(buffer);
  }
  strcat(result,"]");

  return result;
}

char* groupListToJSON(const List *list) {
  if (list == NULL || getLength((List*)list) == 0) {
    char *result = (char*)malloc(5);
    strcpy(result,"[]");
    return result;
  }
  int length = getLength((List*)list);
  char* result = (char*)malloc(500 * length);
  ListIterator iter = createIterator((List*)list);
  Group* group = NULL;
  int count = 0;
  strcpy(result,"[");
  while ((group = (Group*)nextElement(&iter)) != NULL) {
    char* buffer = groupToJSON(group);
    strcat(result,buffer);
    count++;
    if (count != length){
      strcat(result,",");
    }
    free(buffer);
  }
  strcat(result,"]");

  return result;
}

char* SVGtoJSON(const SVGimage* image) {
  if (image == NULL||image -> rectangles == NULL||image -> circles == NULL||image -> paths == NULL||image -> groups == NULL) {
    char *result = (char*)malloc(5);
    strcpy(result,"{}");
    return result;
  }

  List* rects = getRects((SVGimage*)image);
  List* circles = getCircles((SVGimage*)image);
  List* paths = getPaths((SVGimage*)image);
  List* groups = getGroups((SVGimage*)image);
  int numR = getLength(rects);
  int numC = getLength(circles);
  int numP = getLength(paths);
  int numG = getLength(groups);
  freeList(rects);
  freeList(circles);
  freeList(paths);
  freeList(groups);

  char *result = (char*)malloc(500);
  sprintf(result,"{\"numRect\":%d,\"numCirc\":%d,\"numPaths\":%d,\"numGroups\":%d}",numR,numC,numP,numG);
  return result;
}
/* ******************************* Bonus A2 functions - optional for A2 *************************** */

/** Function to converting a JSON string into an SVGimage struct
*@pre JSON string is not NULL
*@post String has not been modified in any way
*@return A newly allocated and initialized SVGimage struct
*@param str - a pointer to a string
**/
SVGimage* JSONtoSVG(const char* svgString){
  if (strcmp(svgString,"") == 0 || svgString == NULL) {
    return NULL;
  }

  SVGimage* imagePtr = (SVGimage*)calloc(1,sizeof(SVGimage));
  initializeAllList(imagePtr);
  char buffer[4][50] = {""};
  int j, count = 0;

  for (int i = 0; i < strlen(svgString); i++) {
    if (svgString[i] == '\"') {
      j = 0;
      i++;
      while (svgString[i] != '\"') {
        buffer[count][j++] = svgString[i++];
      }
      count++;
    }
  }

  strcpy(imagePtr -> namespace,"http://www.w3.org/2000/svg");
  strcpy(imagePtr -> title, buffer[1]);
  strcpy(imagePtr -> description,buffer[3]);

  return imagePtr;
}

/** Function to converting a JSON string into a Rectangle struct
*@pre JSON string is not NULL
*@post Rectangle has not been modified in any way
*@return A newly allocated and initialized Rectangle struct
*@param str - a pointer to a string
**/
Rectangle* JSONtoRect(const char* svgString) {
  int count = 0;

  if (strcmp(svgString,"") == 0 || svgString == NULL) {
    return NULL;
  }

  Rectangle* rect = (Rectangle*)calloc(1,sizeof(Rectangle));
  char buffer[10][50] = {""};
  int i,j;

  count = 0;
  for (i = 0; i < strlen(svgString); i++) {
    if (svgString[i] == '\"') {
      j = 0;
      i++;
      while (svgString[i] != '\"') {
        buffer[count][j++] = svgString[i++];
      }
      count++;
    }
  }
  rect -> x = strtof(buffer[1], NULL);
  rect -> y = strtof(buffer[3], NULL);
  rect -> width = strtof(buffer[5], NULL);
  rect -> height = strtof(buffer[7], NULL);
  strcpy(rect -> units,buffer[9]);
  rect -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);

  return rect;
}

/** Function to converting a JSON string into a Circle struct
*@pre JSON string is not NULL
*@post Circle has not been modified in any way
*@return A newly allocated and initialized Circle struct
*@param str - a pointer to a string
**/
Circle* JSONtoCircle(const char* svgString) {
  int count = 0;

  if (strcmp(svgString,"") == 0 || svgString == NULL) {
    return NULL;
  }

  Circle* circle = (Circle*)calloc(1,sizeof(Circle));
  char buffer[8][50] = {""};
  int i,j;

  count = 0;
  for (i = 0; i < strlen(svgString); i++) {
    if (svgString[i] == '\"') {
      j = 0;
      i++;
      while (svgString[i] != '\"') {
        buffer[count][j++] = svgString[i++];
      }
      count++;
    }
  }
  circle -> cx = strtof(buffer[1], NULL);
  circle -> cy = strtof(buffer[3], NULL);
  circle -> r = strtof(buffer[5], NULL);
  strcpy(circle -> units,buffer[7]);
  circle -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);
  //printf("%f - %f - %f - %s\n",circle -> cx, circle -> cy, circle -> r, circle -> units );

  return circle;
}

xmlDocPtr createTrees(SVGimage* image) {
  xmlDocPtr doc = NULL;       /* document pointer */
  xmlNodePtr root_node = NULL;/* node pointers */
  xmlNsPtr namespace = NULL;

  doc = xmlNewDoc(BAD_CAST "1.0");
  root_node = xmlNewNode(NULL, BAD_CAST "svg");
  xmlDocSetRootElement(doc, root_node);
  namespace = xmlNewNs(root_node,BAD_CAST image -> namespace,NULL);
  xmlSetNs(root_node,namespace);

  /*
  * Creates a DTD declaration. Isn't mandatory.
  */
  xmlCreateIntSubset(doc, BAD_CAST "svg", BAD_CAST "W3C/DTD SVG 1.1 EN", BAD_CAST "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd");

  /*Title and description*/
  ////printf("desc: %s\n",image -> description );
  ////printf("title: %s\n",image -> title );
  if (strcmp(image -> title,"") != 0) {
    xmlNewChild(root_node, NULL, BAD_CAST "title",
    BAD_CAST image -> title);
  }

  if (strcmp(image -> description,"") != 0) {
    xmlNewChild(root_node, NULL, BAD_CAST "desc",
    BAD_CAST image -> description);
  }


  int length = getLength(image -> otherAttributes);
  if (length != 0) {
    ListIterator iter = createIterator(image -> otherAttributes);
    Attribute* attr = NULL;
    while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
      xmlNewProp(root_node, BAD_CAST attr -> name, BAD_CAST attr -> value);
    }
  }

  /*Rectangles*/
  length = getLength(image -> rectangles);
  if (length != 0) {
    ListIterator iter = createIterator(image -> rectangles);
    Rectangle* rect = NULL;
    while ((rect = (Rectangle*)nextElement(&iter)) != NULL) {
      xmlNodePtr rectNode = xmlNewChild(root_node, NULL, BAD_CAST "rect",NULL);
      char floatString[256] = "";
      sprintf(floatString,"%f",rect -> x);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "x", BAD_CAST floatString);
      sprintf(floatString,"%f",rect -> y);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "y", BAD_CAST floatString);
      sprintf(floatString,"%f",rect -> width);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "width", BAD_CAST floatString);
      sprintf(floatString,"%f",rect -> height);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "height", BAD_CAST floatString);
      int length = getLength(rect -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(rect -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(rectNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }

  /*Circles*/
  length = getLength(image -> circles);
  if (length != 0) {
    ListIterator iter = createIterator(image -> circles);
    Circle* circle = NULL;
    while ((circle = (Circle*)nextElement(&iter)) != NULL) {
      xmlNodePtr circleNode = xmlNewChild(root_node, NULL, BAD_CAST "circle",NULL);
      char floatString[256];
      sprintf(floatString,"%f",circle -> cx);
      strcat(floatString,circle -> units);
      xmlNewProp(circleNode, BAD_CAST "cx", BAD_CAST floatString);
      sprintf(floatString,"%f",circle -> cy);
      strcat(floatString,circle -> units);
      xmlNewProp(circleNode, BAD_CAST "cy", BAD_CAST floatString);
      sprintf(floatString,"%f",circle -> r);
      strcat(floatString,circle -> units);
      xmlNewProp(circleNode, BAD_CAST "r", BAD_CAST floatString);
      int length = getLength(circle -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(circle -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(circleNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }

  /*Paths*/
  length = getLength(image -> paths);
  if (length != 0) {
    ListIterator iter = createIterator(image -> paths);
    Path* path = NULL;
    while ((path = (Path*)nextElement(&iter)) != NULL) {
      xmlNodePtr pathNode = xmlNewChild(root_node, NULL, BAD_CAST "path",NULL);
      xmlNewProp(pathNode, BAD_CAST "d", BAD_CAST path -> data);
      int length = getLength(path -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(path -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(pathNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }

  /*Groups*/
  length = getLength(image -> groups);
  if (length != 0) {
    ListIterator iter = createIterator(image -> groups);
    Group* group = NULL;
    while ((group = (Group*)nextElement(&iter)) != NULL) {
      xmlNodePtr groupNode = xmlNewChild(root_node, NULL, BAD_CAST "g",NULL);
      buildGroupNode(groupNode,group);
      //xmlNewProp(groupNode, BAD_CAST "d", BAD_CAST group -> data);
      int length = getLength(group -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(group -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(groupNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }


  return doc;

}

void buildGroupNode(xmlNodePtr root_node,Group* group) {
  /*Rectangles*/
  int length = getLength(group -> rectangles);
  if (length != 0) {
    ListIterator iter = createIterator(group -> rectangles);
    Rectangle* rect = NULL;
    while ((rect = (Rectangle*)nextElement(&iter)) != NULL) {
      xmlNodePtr rectNode = xmlNewChild(root_node, NULL, BAD_CAST "rect",NULL);
      char floatString[256];
      sprintf(floatString,"%f",rect -> x);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "x", BAD_CAST floatString);
      sprintf(floatString,"%f",rect -> y);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "y", BAD_CAST floatString);
      sprintf(floatString,"%f",rect -> width);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "width", BAD_CAST floatString);
      sprintf(floatString,"%f",rect -> height);
      strcat(floatString,rect -> units);
      xmlNewProp(rectNode, BAD_CAST "height", BAD_CAST floatString);
      int length = getLength(rect -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(rect -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(rectNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }

  /*Circles*/
  length = getLength(group -> circles);
  if (length != 0) {
    ListIterator iter = createIterator(group -> circles);
    Circle* circle = NULL;
    while ((circle = (Circle*)nextElement(&iter)) != NULL) {
      xmlNodePtr circleNode = xmlNewChild(root_node, NULL, BAD_CAST "circle",NULL);
      char floatString[256];
      sprintf(floatString,"%f",circle -> cx);
      strcat(floatString,circle -> units);
      xmlNewProp(circleNode, BAD_CAST "cx", BAD_CAST floatString);
      sprintf(floatString,"%f",circle -> cy);
      strcat(floatString,circle -> units);
      xmlNewProp(circleNode, BAD_CAST "cy", BAD_CAST floatString);
      sprintf(floatString,"%f",circle -> r);
      strcat(floatString,circle -> units);
      xmlNewProp(circleNode, BAD_CAST "r", BAD_CAST floatString);
      int length = getLength(circle -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(circle -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(circleNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }

  /*Paths*/
  length = getLength(group -> paths);
  if (length != 0) {
    ListIterator iter = createIterator(group -> paths);
    Path* path = NULL;
    while ((path = (Path*)nextElement(&iter)) != NULL) {
      xmlNodePtr pathNode = xmlNewChild(root_node, NULL, BAD_CAST "path",NULL);
      xmlNewProp(pathNode, BAD_CAST "d", BAD_CAST path -> data);
      int length = getLength(path -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(path -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(pathNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }
  /*Groups*/
  length = getLength(group -> groups);
  if (length != 0) {
    ListIterator iter = createIterator(group -> groups);
    Group* singleGroup = NULL;
    while ((singleGroup = (Group*)nextElement(&iter)) != NULL) {
      xmlNodePtr groupNode = xmlNewChild(root_node, NULL, BAD_CAST "g",NULL);
      buildGroupNode(groupNode,singleGroup);
      //xmlNewProp(groupNode, BAD_CAST "d", BAD_CAST group -> data);
      int length = getLength(singleGroup -> otherAttributes);
      if (length != 0) {
        ListIterator iter = createIterator(singleGroup -> otherAttributes);
        Attribute* attr = NULL;
        while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
          xmlNewProp(groupNode, BAD_CAST attr -> name, BAD_CAST attr -> value);
        }
      }
    }
  }
}
SVGimage* createSVGimage(char* fileName) {
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  SVGimage* imagePtr = (SVGimage*)calloc(1,sizeof(SVGimage));

  /*Check to see if the fileName is valid or not*/
  if (fileName == NULL || strcmp(fileName," ") == 0) {
    free(imagePtr);
    return NULL;
  }

  doc = xmlReadFile(fileName, NULL, 0);

  if (doc == NULL) {
    free(imagePtr);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return NULL;
  }

  /*Get the root element node */
  root_element = xmlDocGetRootElement(doc);
  initializeAllList(imagePtr);

  if (root_element == NULL || root_element->ns == NULL|| root_element->ns->href == NULL) {
    deleteSVGimage(imagePtr);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return NULL;
  }

  char* data = (char*)root_element->ns->href;
  strcpy(imagePtr -> namespace,data);
  initialSVG(root_element,imagePtr);

  /*free the document */
  xmlFreeDoc(doc);
  xmlCleanupParser();


  return imagePtr;
}

char* SVGimageToString(SVGimage* img) {
  /*Output my SVG's information*/
  char* aPtr = toString(img -> otherAttributes);
  char* rPtr = toString(img -> rectangles);
  char* cPtr = toString(img -> circles);
  char* pPtr = toString(img -> paths);
  char* gPtr = toString(img -> groups);
  char *result = (char*)malloc(strlen(aPtr) + strlen(rPtr)+strlen(cPtr)+strlen(pPtr)+strlen(gPtr)+ 5000);
  strcpy(result,"Namespace: ");
  strcat(result,img -> namespace);
  strcat(result,"\ntitle: ");
  strcat(result,img -> title);
  strcat(result,"\ndescription: ");
  strcat(result,img -> description);

  strcat(result,"\nother Attirbute's information: ");
  strcat(result,aPtr);
  strcat(result,"\nRectangle's information: ");
  strcat(result,rPtr);
  strcat(result,"\nCircles's information: ");
  strcat(result,cPtr);
  strcat(result,"\nPath's information: ");
  strcat(result,pPtr);
  strcat(result,"\nGroups's information: \n");
  strcat(result,gPtr);

  /*Free all the pointers*/
  free(aPtr);
  free(rPtr);
  free(cPtr);
  free(pPtr);
  free(gPtr);



  return result;

}

void deleteSVGimage(SVGimage* imagePtr) {
  if (imagePtr == NULL) {
    return;
  }
  freeList(imagePtr -> otherAttributes);
  freeList(imagePtr -> rectangles);
  freeList(imagePtr -> circles);
  freeList(imagePtr -> paths);
  freeList(imagePtr -> groups);
  free(imagePtr);
}

void dummyDelete(void* data) { }

// Function that returns a list of all rectangles in the image.
void getRectsForGroup(Group* group, List* rects) {
  ListIterator iter = createIterator(group -> rectangles);
  Rectangle* rectangle = NULL;
  while ((rectangle = (Rectangle*)nextElement(&iter)) != NULL) {
    insertBack(rects,rectangle);
  }

  Group* g = NULL;
  iter = createIterator(group -> groups);
  while ((g = (Group*)nextElement(&iter)) != NULL) {
    getRectsForGroup(g,rects);
  }
}

List* getRects(SVGimage* img) {
  if (img == NULL) {
    return NULL;
  }
  List* rects = initializeList(&rectangleToString,&dummyDelete,&compareRectangles);
  ListIterator iter = createIterator(img -> rectangles);
  Rectangle* rectangle = NULL;
  while ((rectangle = (Rectangle*)nextElement(&iter)) != NULL) {
    insertBack(rects,rectangle);
  }

  Group* g = NULL;
  iter = createIterator(img -> groups);
  while ((g = (Group*)nextElement(&iter)) != NULL) {
    getRectsForGroup(g,rects);
  }

  return rects;
}

void getCirclesForGroup(Group* group, List* circles) {
  ListIterator iter = createIterator(group -> circles);
  Circle* circle = NULL;
  while ((circle = (Circle*)nextElement(&iter)) != NULL) {
    insertBack(circles,circle);
  }

  Group* g = NULL;
  iter = createIterator(group -> groups);
  while ((g = (Group*)nextElement(&iter)) != NULL) {
    getCirclesForGroup(g,circles);
  }
}

List* getCircles(SVGimage* img) {
  if (img == NULL) {
    return NULL;
  }
  List* circles = initializeList(&circleToString,&dummyDelete,&compareCircles);
  ListIterator iter = createIterator(img -> circles);
  Circle* circle = NULL;
  while ((circle = (Circle*)nextElement(&iter)) != NULL) {
    insertBack(circles,circle);
  }

  Group* g = NULL;
  iter = createIterator(img -> groups);
  while ((g = (Group*)nextElement(&iter)) != NULL) {
    getCirclesForGroup(g,circles);
  }

  return circles;
}

void getPathsForGroup(Group* group, List* paths) {
  ListIterator iter = createIterator(group -> paths);
  Path* path = NULL;
  while ((path = (Path*)nextElement(&iter)) != NULL) {
    insertBack(paths,path);
  }

  Group* g = NULL;
  iter = createIterator(group -> groups);
  while ((g = (Group*)nextElement(&iter)) != NULL) {
    getPathsForGroup(g,paths);
  }
}

List* getPaths(SVGimage* img) {
  if (img == NULL) {
    return NULL;
  }
  List* paths = initializeList(&pathToString,&dummyDelete,&comparePaths);
  ListIterator iter = createIterator(img -> paths);
  Path* path = NULL;
  while ((path = (Path*)nextElement(&iter)) != NULL) {
    insertBack(paths,path);
  }

  Group* g = NULL;
  iter = createIterator(img -> groups);
  while ((g = (Group*)nextElement(&iter)) != NULL) {
    getPathsForGroup(g,paths);
  }

  return paths;
}

void getGroupsForGroup(Group* group, List* groups) {
  ListIterator iter = createIterator(group -> groups);
  Group* g = NULL;
  while ((g = (Group*)nextElement(&iter)) != NULL) {
    insertBack(groups,g);
    getGroupsForGroup(g,groups);
  }
}

List* getGroups(SVGimage* img) {
  if (img == NULL) {
    return NULL;
  }

  List* groups = initializeList(&groupToString,&dummyDelete,&compareGroups);
  ListIterator iter = createIterator(img -> groups);
  Group* group = NULL;
  while ((group = (Group*)nextElement(&iter)) != NULL) {
    insertBack(groups,group);
    getGroupsForGroup(group,groups);
  }

  return groups;
}

// Function that returns the number of all rectangles with the specified area
int numRectsWithArea(SVGimage* img, float area) {
  if (img == NULL) {
    return 0;
  }

  List* rects = getRects(img);
  ListIterator iter = createIterator(rects);
  Rectangle* rectangle = NULL;
  int count = 0;
  while ((rectangle = (Rectangle*)nextElement(&iter)) != NULL) {
    float a = (rectangle -> width) *  (rectangle -> height);
    if (ceil(a) == ceil(area)) {
      count++;
    }
  }
  freeList(rects);
  return count;
}
// Function that returns the number of all circles with the specified area
int numCirclesWithArea(SVGimage* img, float area) {
  if (img == NULL) {
    return 0;
  }

  List* circles = getCircles(img);
  ListIterator iter = createIterator(circles);
  Circle* circle = NULL;
  int count = 0;
  while ((circle = (Circle*)nextElement(&iter)) != NULL) {
    float a = circle -> r * circle -> r * 3.1415926535;
    if (ceil(a) == ceil(area)) {
      count++;
    }
  }
  freeList(circles);
  return count;
}
// Function that returns the number of all paths with the specified data - i.e. Path.data field
int numPathsWithdata(SVGimage* img, char* data) {
  if (img == NULL || data == NULL) {
    return 0;
  }

  List* paths = getPaths(img);
  ListIterator iter = createIterator(paths);
  Path* path = NULL;
  int count = 0;
  while ((path = (Path*)nextElement(&iter)) != NULL) {
    if (strcmp(data,path -> data) == 0) {
      count++;
    }
  }
  freeList(paths);
  return count;
}
// Function that returns the number of all groups with the specified length - see A1 Module 2 for details
int numGroupsWithLen(SVGimage* img, int len) {
  if (img == NULL) {
    return 0;
  }

  List* groups = getGroups(img);
  ListIterator iter = createIterator(groups);
  Group* group = NULL;
  int count = 0;
  while ((group = (Group*)nextElement(&iter)) != NULL) {
    int length = getLength(group -> rectangles)
    + getLength(group -> circles)
    + getLength(group -> paths)
    + getLength(group -> groups);
    if (length == len) {
      count++;
    }
  }

  freeList(groups);
  return count;

}

int numAttr(SVGimage* img) {
  if (img == NULL) {
    return 0;
  }

  int numOfAttr = 0;

  List* groups = getGroups(img);
  ListIterator iter = createIterator(groups);
  Group* group = NULL;
  while ((group = (Group*)nextElement(&iter)) != NULL) {
    numOfAttr += getLength(group -> otherAttributes);
  }
  freeList(groups);

  List* rects = getRects(img);
  iter = createIterator(rects);
  Rectangle* rect = NULL;
  while ((rect = (Rectangle*)nextElement(&iter)) != NULL) {
    numOfAttr += getLength(rect -> otherAttributes);
  }
  freeList(rects);

  List* circles = getCircles(img);
  iter = createIterator(circles);
  Circle* circle = NULL;
  while ((circle = (Circle*)nextElement(&iter)) != NULL) {
    numOfAttr += getLength(circle -> otherAttributes);
  }
  freeList(circles);

  List* paths = getPaths(img);
  iter = createIterator(paths);
  Path* path = NULL;
  while ((path = (Path*)nextElement(&iter)) != NULL) {
    numOfAttr += getLength(path -> otherAttributes);
  }
  freeList(paths);

  numOfAttr += getLength(img -> otherAttributes);

  return numOfAttr;

}


/* ******************************* List helper functions  - MUST be implemented *************************** */

void deleteAttribute( void* data) {
  Attribute* newAttr = (Attribute*)data;
  free(newAttr -> name);
  free(newAttr -> value);
  free(newAttr);
}
char* attributeToString(void* data) {
  Attribute* newAttr = (Attribute*)data;

  char *result = (char*)malloc(500);

  strcpy(result,"other attribute name: ");
  strcat(result, newAttr -> name);
  strcat(result,", attribute value = ");
  strcat(result,newAttr -> value);

  return result;

}
int compareAttributes(const void *first, const void *second) {
  return 0;
}

void deleteGroup(void* data) {
  Group* group = (Group*)data;
  freeList(group -> rectangles);
  freeList(group -> circles);
  freeList(group -> paths);
  freeList(group -> groups);
  freeList(group -> otherAttributes);
  free(group);
}
char* groupToString(void* data) {
  Group* group = (Group*)data;

  char* aPtr = toString(group -> otherAttributes);
  char* rPtr = toString(group -> rectangles);
  char* cPtr = toString(group -> circles);
  char* pPtr = toString(group -> paths);
  char* gPtr = toString(group -> groups);
  char *result = (char*)malloc(strlen(aPtr) + strlen(rPtr)+strlen(cPtr)+strlen(pPtr)+strlen(gPtr)+ 500);
  strcpy(result,"group other Attirbute's information: ");
  strcat(result,aPtr);
  strcat(result,"\ngroup Rectangle's information: ");
  strcat(result,rPtr);
  strcat(result,"\ngroup Circles's information: ");
  strcat(result,cPtr);
  strcat(result,"\ngroup Path's information: ");
  strcat(result,pPtr);
  strcat(result,"\ngroup Groups's information: \n");
  strcat(result,gPtr);

  free(aPtr);
  free(rPtr);
  free(cPtr);
  free(pPtr);
  free(gPtr);

  return result;
}
int compareGroups(const void *first, const void *second) {
  return 0;
}

void deleteRectangle(void* data) {
  Rectangle* rect = (Rectangle*)data;
  if (rect -> otherAttributes != NULL) {
    freeList(rect -> otherAttributes);
  }

  free(rect);
}
char* rectangleToString(void* data) {
  Rectangle* rect = (Rectangle*)data;
  char* stringPtr = toString(rect -> otherAttributes);
  char* ptr = (char*)malloc(strlen(stringPtr) + 500);
  strcpy(ptr,stringPtr);
  free(stringPtr);
  return ptr;
}

int compareRectangles(const void *first, const void *second){
  return 0;
}

void deleteCircle(void* data) {
  Circle* circle = (Circle*)data;
  if (circle -> otherAttributes != NULL) {
    freeList(circle -> otherAttributes);
  }

  free(circle);
}
char* circleToString(void* data) {
  Circle* circle = (Circle*)data;
  char* stringPtr = toString(circle -> otherAttributes);
  char* ptr = (char*)malloc(strlen(stringPtr) + 500);
  strcpy(ptr,stringPtr);
  free(stringPtr);
  return ptr;

}

int compareCircles(const void *first, const void *second) {
  return 0;
}

void deletePath(void* data) {
  Path* path = (Path*)data;
  free(path->data);
  if (path -> otherAttributes != NULL) {
    freeList(path -> otherAttributes);
  }
  free(path);
}
char* pathToString(void* data) {
  Path* path = (Path*)data;

  char* stringPtr = toString(path -> otherAttributes);
  char* sentence = malloc(strlen(path->data) + strlen(stringPtr)+500);
  strcpy(sentence,"path data: ");
  strcat(sentence, path -> data);
  strcat(sentence,stringPtr);
  free(stringPtr);

  return sentence;
}
int comparePaths(const void *first, const void *second) {
  return 0;
}

void initializeAllLists(Group* groupPtr) {
  /*Initilaize all the list*/
  groupPtr -> rectangles = initializeList(&rectangleToString,&deleteRectangle,&compareRectangles);
  groupPtr -> circles = initializeList(&circleToString,&deleteCircle,&compareCircles);
  groupPtr -> paths = initializeList(&pathToString,&deletePath,&comparePaths);
  groupPtr -> groups = initializeList(&groupToString,&deleteGroup,&compareGroups);
  groupPtr -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);

}

void buildRectForGroup(xmlNode* curNode,Group* groupPtr) {
  xmlAttr *rectInfo;

  Rectangle* rect = (Rectangle*)calloc(1,sizeof(Rectangle));
  rect -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);
  char* unit = NULL;

  for (rectInfo = curNode -> properties; rectInfo != NULL; rectInfo = rectInfo -> next) {
    xmlNode *value = rectInfo -> children;
    char *rectName = (char *)rectInfo -> name;
    char *cont = (char *)(value -> content);
    if (strcmp(rectName,"x") == 0) {
      rect -> x = atof(cont);
    } else if (strcmp(rectName,"y") == 0) {
      rect -> y = atof(cont);
    } else if (strcmp(rectName,"width") == 0) {
      rect -> width = atof(cont);
    } else if (strcmp(rectName,"height") == 0) {
      rect -> height = strtof(cont,&unit);
      strncpy(rect->units,unit,49);
    } else {
      Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
      newAttr -> name = (char*)malloc(strlen(rectName) + 1);
      newAttr -> value = (char*)malloc(strlen(cont) + 1);
      strcpy(newAttr -> name,rectName);
      strcpy(newAttr -> value,cont);
      insertBack(rect -> otherAttributes,newAttr);
    }
  }
  insertBack(groupPtr -> rectangles,rect);
}

void buildRect(xmlNode* curNode,SVGimage* imagePtr) {
  /*Declaring my variables*/
  xmlAttr *rectInfo;
  Rectangle* rect = (Rectangle*)calloc(1,sizeof(Rectangle));
  rect -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);
  char* unit = NULL;

  for (rectInfo = curNode -> properties; rectInfo != NULL; rectInfo = rectInfo -> next) {
    xmlNode *value = rectInfo -> children;
    char *rectName = (char *)rectInfo -> name;
    char *cont = (char *)(value -> content);
    if (strcasecmp(rectName,"x") == 0) {
      rect -> x = atof(cont);
    } else if (strcasecmp(rectName,"y") == 0) {
      rect -> y = atof(cont);
    } else if (strcasecmp(rectName,"width") == 0) {
      rect -> width = atof(cont);
    } else if (strcasecmp(rectName,"height") == 0) {
      rect -> height = strtof(cont,&unit);
      strncpy(rect->units,unit,49);
    } else {
      Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
      newAttr -> name = (char*)malloc(strlen(rectName) + 1);
      newAttr -> value = (char*)malloc(strlen(cont) + 1);
      strcpy(newAttr -> name,rectName);
      strcpy(newAttr -> value,cont);
      insertBack(rect -> otherAttributes,newAttr);
    }
  }
  insertBack(imagePtr -> rectangles,rect); //Insert to the list
}

void buildCircle(xmlNode* curNode,SVGimage* imagePtr) {
  xmlAttr *circleInfo;
  Circle* circle = (Circle*)calloc(1,sizeof(Circle));
  circle -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);
  char* unit = NULL;

  for (circleInfo = curNode -> properties; circleInfo != NULL; circleInfo = circleInfo -> next) {
    xmlNode *value = circleInfo -> children;
    char *circleName = (char *)circleInfo -> name;
    char *cont = (char *)(value -> content);
    if (strcmp(circleName,"cx") == 0) {
      circle -> cx = atof(cont);
    } else if (strcmp(circleName,"cy") == 0) {
      circle -> cy = atof(cont);
    } else if (strcmp(circleName,"r") == 0) {
      circle -> r = strtof(cont,&unit);
      strncpy(circle->units,unit,49);//Get the unit
    } else {
      Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
      newAttr -> name = (char*)malloc(strlen(circleName) + 1);
      newAttr -> value = (char*)malloc(strlen(cont) + 1);
      strcpy(newAttr -> name,circleName);
      strcpy(newAttr -> value,cont);
      insertBack(circle -> otherAttributes,newAttr);
    }

  }

  insertBack(imagePtr -> circles,circle);

}

void buildCircleForGroup(xmlNode* curNode,Group* groupPtr) {
  /*Declaring my variables*/
  xmlAttr *circleInfo;

  Circle* circle = (Circle*)calloc(1,sizeof(Circle));
  circle -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);
  char* unit = NULL;//Initilize it

  for (circleInfo = curNode -> properties; circleInfo != NULL; circleInfo = circleInfo -> next) {
    xmlNode *value = circleInfo -> children;
    char *circleName = (char *)circleInfo -> name;
    char *cont = (char *)(value -> content);
    if (strcasecmp(circleName,"cx") == 0) {
      circle -> cx = strtof(cont,NULL);
    } else if (strcasecmp(circleName,"cy") == 0) {
      circle -> cy = strtof(cont,NULL);
    } else if (strcasecmp(circleName,"r") == 0) {
      circle -> r = strtof(cont,&unit);
      strncpy(circle->units,unit,49);//Get the unit
    } else {
      Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
      newAttr -> name = (char*)malloc(strlen(circleName) + 1);
      newAttr -> value = (char*)malloc(strlen(cont) + 1);
      strcpy(newAttr -> name,circleName);
      strcpy(newAttr -> value,cont);
      insertBack(circle -> otherAttributes,newAttr);
    }
  }
  insertBack(groupPtr -> circles,circle);

}

void buildPath(xmlNode* curNode,SVGimage* imagePtr) {
  xmlAttr *pathInfo;

  Path* path = (Path*)calloc(1,sizeof(Path));
  path -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);

  for (pathInfo = curNode -> properties; pathInfo != NULL; pathInfo = pathInfo -> next) {
    xmlNode *value = pathInfo -> children;
    char *pathName = (char *)pathInfo -> name;
    char *cont = (char *)(value -> content);
    if (strcasecmp(pathName,"d") == 0) {
      path -> data = malloc(strlen(cont) + 1);
      strcpy(path -> data,cont);
    } else {
      Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
      newAttr -> name = (char*)malloc(strlen(pathName) + 1);
      newAttr -> value = (char*)malloc(strlen(cont) + 1);
      strcpy(newAttr -> name,pathName);
      strcpy(newAttr -> value,cont);
      insertBack(path -> otherAttributes,newAttr);
    }
  }

  insertBack(imagePtr -> paths,path);

}

void buildPathForGroup(xmlNode* curNode,Group* groupPtr) {
  /*This function is same as build group*/
  xmlAttr *pathInfo;
  Path* path = (Path*)calloc(1,sizeof(Path));
  path -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);

  for (pathInfo = curNode -> properties; pathInfo != NULL; pathInfo = pathInfo -> next) {
    xmlNode *value = pathInfo -> children;
    char *pathName = (char *)pathInfo -> name;
    char *cont = (char *)(value -> content);
    if (strcasecmp(pathName,"d") == 0) {
      path -> data = malloc(strlen(cont) + 1);
      strcpy(path -> data,cont);
    } else {
      Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
      newAttr -> name = (char*)malloc(strlen(pathName) + 1);
      newAttr -> value = (char*)malloc(strlen(cont) + 1);
      strcpy(newAttr -> name,pathName);
      strcpy(newAttr -> value,cont);
      insertBack(path -> otherAttributes,newAttr);
    }
  }

  insertBack(groupPtr -> paths, path);

}

void buildGroupForGroup(xmlNode* myNode,Group* groupPtr) {
  xmlNode *cur_node = NULL;

  Group* group = (Group*)calloc(1,sizeof(Group));
  initializeAllLists(group);

  xmlAttr *attr;
  for (attr = myNode -> properties; attr != NULL; attr = attr->next) {
    Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
    xmlNode *value = attr->children;
    char *attrName = (char *)attr -> name;
    char *cont = (char *)(value -> content);
    newAttr -> name = (char*)malloc(strlen(attrName) + 1);
    newAttr -> value = (char*)malloc(strlen(cont) + 1);
    strcpy(newAttr -> name,attrName);
    strcpy(newAttr -> value,cont);
    insertBack(group -> otherAttributes,newAttr);
  }


  /*Loop through every child of the current node*/
  for (cur_node = myNode -> children; cur_node != NULL; cur_node = cur_node->next) {
    if (cur_node -> type == XML_ELEMENT_NODE) {
      if(strcmp((char*)cur_node->name,"rect") == 0) {
        buildRectForGroup(cur_node,group);
      } else if(strcmp((char*)cur_node->name,"circle") == 0) {
        buildCircleForGroup(cur_node,group);
      } else if(strcmp((char*)cur_node->name,"path") == 0) {
        buildPathForGroup(cur_node,group);
      } else if(strcmp((char*)cur_node->name,"g") == 0) {
        buildGroupForGroup(cur_node,group);//Recursively call the function
      }
    }
  }
  insertBack(groupPtr -> groups, group);
}

void buildGroup(xmlNode* myNode,SVGimage* imagePtr) {
  xmlNode *cur_node = NULL;
  Group* group = (Group*)calloc(1,sizeof(Group));
  initializeAllLists(group);


  /*For other attributes*/
  xmlAttr *attr;
  for (attr = myNode -> properties; attr != NULL; attr = attr->next) {
    Attribute *newAttr = (Attribute*)malloc(sizeof(Attribute));
    xmlNode *value = attr -> children;
    char *attrName = (char *)attr -> name;
    char *cont = (char *)(value -> content);
    newAttr -> name = (char*)malloc(strlen(attrName) + 1);
    newAttr -> value = (char*)malloc(strlen(cont) + 1);
    strcpy(newAttr -> name,attrName);
    strcpy(newAttr -> value,cont);
    insertBack(group -> otherAttributes,newAttr);
  }

  for (cur_node = myNode -> children; cur_node != NULL; cur_node = cur_node -> next) {
    if (cur_node -> type == XML_ELEMENT_NODE) {
      if(strcmp((char*)cur_node->name,"rect") == 0) {
        buildRectForGroup(cur_node,group);
      } else if(strcasecmp((char*)cur_node->name,"circle") == 0) {
        buildCircleForGroup(cur_node,group);
      } else if(strcasecmp((char*)cur_node->name,"path") == 0) {
        buildPathForGroup(cur_node,group);
      } else if(strcasecmp((char*)cur_node->name,"g") == 0) {
        buildGroupForGroup(cur_node,group);
      }
    }
  }
  insertBack(imagePtr -> groups,group);

}



void initialSVG(xmlNode* root_element,SVGimage* imagePtr) {
  xmlNode *cur_node = NULL;
  for (cur_node = root_element; cur_node != NULL; cur_node = cur_node->next) {
    if (cur_node -> type == XML_ELEMENT_NODE) {
      /*Go through every node in the XML*/
      if (strcasecmp((char*)cur_node->name,"title") == 0 && strcasecmp((char*)cur_node->parent->name,"g") != 0) {
        if (cur_node -> children -> content != NULL ){
          strncpy(imagePtr -> title,(char*)cur_node -> children -> content,255);
        }
      } else if(strcasecmp((char*)cur_node->name,"desc") == 0 && strcasecmp((char*)cur_node->parent->name,"g") != 0) {
        if (cur_node -> children -> content != NULL ){
          strncpy(imagePtr -> description,(char*)cur_node -> children -> content,255);
        }
      } else if(strcasecmp((char*)cur_node->name,"rect") == 0 && strcasecmp((char*)cur_node->parent->name,"g") != 0) {
        buildRect(cur_node,imagePtr);
      } else if(strcasecmp((char*)cur_node->name,"circle") == 0 && strcasecmp((char*)cur_node->parent->name,"g") != 0) {
        buildCircle(cur_node,imagePtr);

      } else if(strcasecmp((char*)cur_node->name,"path") == 0 && strcasecmp((char*)cur_node->parent->name,"g") != 0) {
        buildPath(cur_node,imagePtr);
      } else if(strcasecmp((char*)cur_node->name,"g") == 0 && strcasecmp((char*)cur_node->parent->name,"g") != 0) {
        buildGroup(cur_node,imagePtr);
      } else if(strcasecmp((char*)cur_node->name,"svg") == 0 ) {
        xmlAttr *attr;
        Attribute *newAttr;

        for (attr = cur_node -> properties; attr != NULL; attr = attr->next) {
          newAttr = (Attribute*)malloc(sizeof(Attribute));
          xmlNode *value = attr->children;
          char *attrName = (char *)attr -> name;
          char *cont = (char *)(value -> content);
          newAttr -> name = (char*)malloc(strlen(attrName) + 1);
          newAttr -> value = (char*)malloc(strlen(cont) + 1);
          strcpy(newAttr -> name,attrName);
          strcpy(newAttr -> value,cont);
          insertBack(imagePtr -> otherAttributes,newAttr);

        }
      }
    }
    initialSVG(cur_node -> children,imagePtr);
  }
}

void initializeAllList(SVGimage* imagePtr) {
  imagePtr -> rectangles = initializeList(&rectangleToString,&deleteRectangle,&compareRectangles);
  imagePtr -> circles = initializeList(&circleToString,&deleteCircle,&compareCircles);
  imagePtr -> paths = initializeList(&pathToString,&deletePath,&comparePaths);
  imagePtr -> groups = initializeList(&groupToString,&deleteGroup,&compareGroups);
  imagePtr -> otherAttributes = initializeList(&attributeToString,&deleteAttribute,&compareAttributes);
}


bool attrCheck(List* otherAttributes) {
  int length = getLength(otherAttributes);
  if (length != 0) {
    ListIterator iter = createIterator(otherAttributes);
    Attribute* attr = NULL;
    while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
      if (attr -> name == NULL || attr -> value == NULL) {
        return false;
      }
    }
  }

  return true;
}

bool rectCheck(List* rectangles) {
  int length = getLength(rectangles);
  if (length != 0) {
    ListIterator iter = createIterator(rectangles);
    Rectangle* rect = NULL;
    while ((rect = (Rectangle*)nextElement(&iter)) != NULL) {
      if (rect -> width < 0) {
        return false;
      }
      if (rect -> height < 0) {
        return false;
      }
      if (rect -> otherAttributes == NULL) {
        return false;
      } else {
        if (attrCheck(rect -> otherAttributes) == false) {
          return false;
        }
      }
    }
  }
  return true;
}

bool cirCheck(List* circles) {
  int length = getLength(circles);
  if (length != 0) {
    ListIterator iter = createIterator(circles);
    Circle* circle = NULL;
    while ((circle = (Circle*)nextElement(&iter)) != NULL) {
      if (circle -> r < 0) {
        return false;
      }
      if (circle -> otherAttributes == NULL) {
        return false;
      } else {
        if (attrCheck(circle -> otherAttributes) == false) {
          return false;
        }
      }
    }
  }
  return true;
}

bool pathCheck(List* paths) {
  int length = getLength(paths);
  if (length != 0) {
    ListIterator iter = createIterator(paths);
    Path* path = NULL;
    while ((path = (Path*)nextElement(&iter)) != NULL) {
      if (path -> data == NULL) {
        return false;
      }
      if (path -> otherAttributes == NULL) {
        return false;
      } else {
        if (attrCheck(path -> otherAttributes) == false) {
          return false;
        }
      }
    }
  }

  return true;
}

bool groupCheck(List* groups) {
  int length = getLength(groups);
  if (length != 0) {
    ListIterator iter = createIterator(groups);
    Group* group = NULL;
    while ((group = (Group*)nextElement(&iter)) != NULL) {
      if (group -> rectangles == NULL) {
        return false;
      } else {
        if (rectCheck(group -> rectangles) == false) {
          return false;
        }
      }

      if (group -> circles == NULL) {
        return false;
      } else {
        if (cirCheck(group -> circles) == false) {
          return false;
        }
      }

      if (group -> paths == NULL) {
        return false;
      } else {
        if (pathCheck(group -> paths) == false) {
          return false;
        }
      }

      if (group -> groups == NULL) {
        return false;
      } else {
        if (groupCheck(group -> groups) == false) {
          return false;
        }
      }

      if (group -> otherAttributes == NULL) {
        return false;
      } else {
        if (attrCheck(group -> otherAttributes) == false) {
          return false;
        }
      }
    }
  }

  return true;
}

void updateGroup(List* groups,Attribute* newAttribute,int index) {
  int length = getLength(groups);
  int count = 0;
  if (length != 0) {
    ListIterator iter = createIterator(groups);
    Group* group = NULL;
    while ((group = (Group*)nextElement(&iter)) != NULL) {
      if (count == index) {
        otherAttr(group -> otherAttributes,newAttribute);
      }
      count++;
    }
  }
}

void updatePath(List* paths,Attribute* newAttribute,int index) {
  int length = getLength(paths);
  int count = 0;
  if (length != 0) {
    ListIterator iter = createIterator(paths);
    Path* path = NULL;
    while ((path = (Path*)nextElement(&iter)) != NULL) {
      if (count == index) {
        if (strcmp(newAttribute -> name,"d") == 0) {
          path -> data = realloc(path -> data, strlen(newAttribute -> value) + 1);
          strcpy(path -> data, newAttribute -> value);
          deleteAttribute(newAttribute);
        } else {
          otherAttr(path -> otherAttributes,newAttribute);
        }
      }
      count++;
    }
  }
}

void updateRect(List* rects,Attribute* newAttribute,int index) {
  int length = getLength(rects);
  int count = 0;
  if (length != 0) {
    ListIterator iter = createIterator(rects);
    Rectangle* rect = NULL;
    while ((rect = (Rectangle*)nextElement(&iter)) != NULL) {
      if (count == index) {
        if (strcmp(newAttribute -> name,"x") == 0) {
          rect -> x = strtof(newAttribute -> value,NULL);
          deleteAttribute(newAttribute);
        } else if (strcmp(newAttribute -> name,"y") == 0) {
          rect -> y = strtof(newAttribute -> value,NULL);
          deleteAttribute(newAttribute);
        } else if (strcmp(newAttribute -> name,"height") == 0) {
          rect -> height = strtof(newAttribute -> value,NULL);
          deleteAttribute(newAttribute);
        } else if (strcmp(newAttribute -> name,"width") == 0) {
          rect -> width = strtof(newAttribute -> value,NULL);
          deleteAttribute(newAttribute);
        } else {
          otherAttr(rect -> otherAttributes,newAttribute);
        }
      }
      count++;
    }
  }
}

void updateCircle(List* circles,Attribute* newAttribute,int index) {
  int length = getLength(circles);
  int count = 0;
  if (length != 0) {
    ListIterator iter = createIterator(circles);
    Circle* circle = NULL;
    while ((circle = (Circle*)nextElement(&iter)) != NULL) {
      if (count == index) {
        if (strcmp(newAttribute -> name,"cx") == 0) {
          circle -> cx = strtof(newAttribute -> value,NULL);
          deleteAttribute(newAttribute);
        } else if (strcmp(newAttribute -> name,"cy") == 0) {
          circle -> cy = strtof(newAttribute -> value,NULL);
          deleteAttribute(newAttribute);
        } else if (strcmp(newAttribute -> name,"r") == 0) {
          circle -> r = strtof(newAttribute -> value,NULL);
          deleteAttribute(newAttribute);
        } else {
          otherAttr(circle -> otherAttributes,newAttribute);
        }
      }
      count++;
    }
  }
}

void otherAttr(List* otherAttributes,Attribute* newAttribute) {
  int length = getLength(otherAttributes);
  int count = 0;
  if (length != 0) {
    ListIterator iter = createIterator(otherAttributes);
    Attribute* attr = NULL;
    while ((attr = (Attribute*)nextElement(&iter)) != NULL) {
      if (strcmp(newAttribute -> name,attr -> name) == 0) {
        attr -> value = realloc(attr -> value,strlen(newAttribute -> value) + 1);
        strcpy(attr -> value, newAttribute -> value);
        deleteAttribute(newAttribute);
        count++;
        break;
      }
    }
  }
  if (count == 0) {
    insertBack(otherAttributes,newAttribute);
  }
}
