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
  int i;

  if (relation.empty() || 
      relation == string(RELCATNAME) || 
      relation == string(ATTRCATNAME))
    return BADCATPARM;

	//find relation
	status = relCat->getInfo(relation, rd);

	if( status == RELEXISTS){
		
		//remove all attributes with relation as relName
		status = attrCat->dropRelation(relation);
		if( status != OK){return status;}
		
		//destroy the heap file
		status = destroyHeapFile(relation);
		if( status != OK){return status;}

	}

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

	status = attrCat->getRelInfo(relation, attrCnt, attrs);
	if( status != OK) { return status;}

	for( i = 0; i < attrCnt; i++){
		
		AttrDesc attr = attrs[i];

		status = attrCat->removeInfo(relation, string(attr.attrName));
		if( status != OK) { return status;}

	}

	return OK;



}


