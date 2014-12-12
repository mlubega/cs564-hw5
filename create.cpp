#include "catalog.h"
#include "page.h"

const Status RelCatalog::createRel(const string & relation, 
           const int attrCnt,
           const attrInfo attrList[])
{
  Status status;
  RelDesc rd;
  AttrDesc ad;
  int i;
  int offsetCnt;

  if (relation.empty() || attrCnt < 1)
    return BADCATPARM;

  if (relation.length() > MAXNAME )
    return NAMETOOLONG;

  
  // check attribute len
  int attrLenSum = 0;
  for(i = 0; i < attrCnt; i++)
    attrLenSum += attrList[i].attrLen;
  

  if( attrLenSum > PAGESIZE)
    return NOSPACE;
  


  // check for duplicate attributes
  for(i = 0; i < attrCnt; i++){
    int j;
    for(j = i; j < attrCnt; j++){
      
      if(i == j)
        continue;
      
      
      if( strcmp(attrList[i].attrName, attrList[j].attrName) == 0 )
        return DUPLATTR;
      
    }
  
  }
  
  //check if relation exists
  if( (status = relCat->getInfo(relation, rd)) == OK){return RELEXISTS;}
 
  if( status != RELNOTFOUND) { return status;}

  // create a RelDesc for the relation and add to RelCatalog
  strcpy(rd.relName, relation.c_str());
  rd.attrCnt = attrCnt;
  if( (status = relCat->addInfo(rd)) != OK ){return status; }


  // create an AttrDesc for each attribute and add to AttrCatalog
  attrInfo atr;
  offsetCnt = 0;
  for(i=0; i < attrCnt; i++){
     
    atr = attrList[i];
    strcpy( ad.relName, atr.relName);
    strcpy(ad.attrName,atr.attrName);
    ad.attrType = atr.attrType;
    ad.attrLen = atr.attrLen; 
    ad.attrOffset = offsetCnt;

    if( (status = attrCat->addInfo(ad)) != OK ){return status; }
    offsetCnt += atr.attrLen; //update offsetCnt for later
  }

  //create HeapFile
  if( (status = createHeapFile(relation)) != OK){return status; }

  return OK;




}

