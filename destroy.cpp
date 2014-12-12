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
	if(( status = relCat->getInfo(relation, rd)) != OK ){
    cout << "destroyRel getInfo relation status: " << status << endl;
    return status; }

  
		
	
    //remove relation from relcatalog
    status = relCat->removeInfo(relation); 
		if( status != OK){
      
    cout << "destroyRel removeInfo relation status: " << status << endl;
      return status;}
		
    //remove all attributes with relation as relName
		status = attrCat->dropRelation(relation);
		if( status != OK){
      
    cout << "destroyRel dropRelation relation status: " << status << endl;
      return status;}

		//destroy the heap file
		return status = destroyHeapFile(relation);
		//if( status != OK){return status;}


//	return OK;


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
	if( (status = attrCat->getRelInfo(relation, attrCnt, attrs)) != OK) {
   
   cout <<  "dropRelation getRelInfo error " << endl;
    return status;}

  //remove each attribute
	for( i = 0; i < attrCnt; i++){
		
		AttrDesc attr = attrs[i];

	  if( (status = attrCat->removeInfo(relation, string(attr.attrName))) != OK) { 
     
   cout << " dropRelation removeInfo error"   << endl; 
 
      delete attrs;
      return status;
    }

	}

  //clean up reference
  delete attrs;

	return OK;



}


