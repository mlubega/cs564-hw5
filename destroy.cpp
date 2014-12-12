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
	if(( status = getInfo(relation, rd)) != OK ){
    cout << "destroyRel getInfo relation status: " << status << endl;
    return status; }

  
		
	
		
    //remove all attributes with relation as relName
		status = attrCat->dropRelation(relation);
		if( status != OK){
      
    cout << "destroyRel dropRelation relation status: " << status << endl;
      return status;}

    //remove relation from relcatalog
    status = removeInfo(relation); 
		if( status != OK){
      
    cout << "destroyRel removeInfo relation status: " << status << endl;
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
    cout <<  "AttrCat::dropRelation getRelInfo error " << endl;
    return status;
  }

  cout << "Successful retrieve attributes" << endl;

  //remove each attribute
	for( i = 0; i < attrCnt; i++){
		

      cout << " Iter #: " << i << " , attribute: " << string(attrs[i].attrName) << endl;
	  if( (status = attrCat->removeInfo(relation, string(attrs[i].attrName))) != OK) { 

     
       cout << " AttrCat::dropRelation removeInfo error"   << endl; 
 
      delete attrs;
      return status;
    }

	}

  cout << " AttrCat::dropRelation successfull removal of attributes" << endl;
  //clean up reference
  delete attrs;

  cout << "sucessfull delete of attributes array" << endl;
	return OK;


/*
  //Creates a scan object
  HeapFileScan* scan = new HeapFileScan(RELCATNAME, status);

  if(status != OK)
    return status;
  
  //Begin scan of the tuples
  if((status = scan->startScan(0, 0, STRING, NULL, EQ)) != OK)
    return status;

  Record rec;
  RID rid;

  //While tuples still exist (file not finished), check
  //the tuples to find which relName matches relation
  while((status = scan->scanNext(rid)) != FILEEOF){
    if(status != OK)
      return status;

    if((status = scan->getRecord(rec)) != OK)
      return status;

    //Then delete those tuples
    if(strcmp((char *) rec.data, relation.c_str()) == 0){ 
      scan->deleteRecord();
    }
  }

  if((status = scan->endScan()) != OK)
    return status;
  delete scan;

  // Repeat one more time
  scan = new HeapFileScan(ATTRCATNAME, status);

  if(status != OK)
    return status;

  if((status = scan->startScan(0, 0, STRING, NULL, EQ)) != OK)
    return status;

  while((status = scan->scanNext(rid)) != FILEEOF){
    if(status != OK)
      return status;

    if((status = scan->getRecord(rec)) != OK)
      return status;

    if(strcmp((char *) rec.data, relation.c_str()) == 0){ 
      scan->deleteRecord();
    }
  }

  if((status = scan->endScan()) != OK)
    return status;
  delete scan;

  return OK; 
*/
}


