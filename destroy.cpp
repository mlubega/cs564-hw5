#include "catalog.h"
#include <string>
//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::destroyRel(const string & relation)
{
  Status status;
  RelDesc rd;

  if (relation.empty() || 
      relation == string(RELCATNAME) || 
      relation == string(ATTRCATNAME))
    return BADCATPARM;

	//find relation
	if(( status = getInfo(relation, rd)) != OK)
    return status; 

  //remove all attributes with relation as relName
	if( (status = attrCat->dropRelation(relation)) !=  OK)
    return status;

  //remove relation from relcatalog
  if( (status = removeInfo(relation)) != OK)
    return status;
	
  //destroy the heap file
	if( ( status = destroyHeapFile(relation)) != OK)
    return status;

	return OK;

}


//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{
  Status status;
  AttrDesc *attrs;
  int attrCnt, i;

  if (relation.empty()) return BADCATPARM;

  //get list of attributes
	if( (status = attrCat->getRelInfo(relation, attrCnt, attrs)) != OK)
    return status;

  //remove each attribute
	for( i = 0; i < attrCnt; i++){
    if( (status = attrCat->removeInfo(relation, string(attrs[i].attrName))) != OK) { 
      delete attrs;
      return status;
    }
	}

  //clean up references
  delete attrs;
	return OK;

}


