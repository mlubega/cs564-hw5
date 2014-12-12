#include "catalog.h"
#include <string>

RelCatalog::RelCatalog(Status &status) :
	 HeapFile(RELCATNAME, status)
{
// nothing should be needed here
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
  if (relation.empty())
    return BADCATPARM;

  Status status;
  Record rec;
  RID rid;
  HeapFileScan* hfs;

  //create scan
  hfs = new HeapFileScan( RELCATNAME , status);
  if(status != OK){
    delete hfs;
    return status; 
  }

  //set filter
  status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  if(status != OK) {
    delete hfs;
    return status;
  }
  
  //scan file for record  
  if((status = hfs->scanNext(rid)) == FILEEOF){
    delete hfs;
    return RELNOTFOUND;
  }
  
  //in case of scan error  
  if(status != OK) {
    delete hfs;
    return status;
  }

  //retrieve record
  status = hfs->getRecord(rec);
  if(status != OK) {
    delete hfs;
    return status;
  }

  //copy return data
  RelDesc * ptr = (RelDesc * ) rec.data;
  memcpy(&record, rec.data, sizeof(RelDesc));



  delete hfs;
  return OK;

}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;

  //create record
  Record rec;
  rec.data = &record;
  rec.length = sizeof(RelDesc); 

  //create scan
  ifs = new InsertFileScan( RELCATNAME , status);  
  if(status != OK) {
	  delete ifs;
	  return status;
  }

  //insert record
  status = ifs->insertRecord(rec, rid);
  if(status != OK) {
	  delete ifs;
	  return status;
  }
 
  //clean up
  delete ifs;
  return OK;

}

const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;

  //create scan
  hfs = new HeapFileScan( RELCATNAME, status);
  if(status != OK) return status;
  
  //set filter
  if((status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK){
    delete hfs;
    return status;
  }

  //find record
  if((status = hfs->scanNext(rid)) != OK) {
    delete hfs;
    return status;
  }
  
  //delete record 
  if((status = hfs->deleteRecord()) != OK) {
    delete hfs;
    return status;
  }
  
  delete hfs;
  return OK;
}


RelCatalog::~RelCatalog()
{
// nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
	 HeapFile(ATTRCATNAME, status)
{
// nothing should be needed here
}


const Status AttrCatalog::getInfo(const string & relation, 
				  const string & attrName,
				  AttrDesc &record)
{

  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;
  RelDesc relRecord;
  
  if (relation.empty() || attrName.empty()) return BADCATPARM;

  //check relation exists
  if( (status = relCat->getInfo(relation, relRecord)) != OK ) return status;

  //create scan
  hfs = new HeapFileScan( ATTRCATNAME, status);
  if( status != OK) return status;

  //set filter
  if( (status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK)
    return status;
  
  //scan attribute catalog for relation
  while( (status = hfs->scanNext(rid)) != FILEEOF){
  
     if(status != OK){
       delete hfs;
       return status;
     }

     if( (status = hfs->getRecord(rec)) != OK) {
       delete hfs;
       return status;
     }
 
     AttrDesc * tmp = (AttrDesc*) rec.data;

     //compare attribute name && copy over if match found
     if( strcmp(tmp->attrName, attrName.c_str()) == 0){
	  		memcpy( &record, rec.data, sizeof(AttrDesc));
        delete hfs;
        return OK;
      }
  
  }

  //if here then attribute was not found
  delete hfs;
  return ATTRNOTFOUND;


}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;
  Record rec;


  //create scan
  ifs = new InsertFileScan(ATTRCATNAME, status);
  if( status != OK) {
	  delete ifs;
	  return status;
  }
 
  //store data
  rec.data = &record;
  rec.length = sizeof(AttrDesc);

  //insert new record
  if( (status = ifs->insertRecord(rec, rid)) != OK) {
	  delete ifs;
	  return OK;
 }
  
  //clean up
  delete ifs;
  return OK;


}


const Status AttrCatalog::removeInfo(const string & relation, 
			       const string & attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record;
  HeapFileScan*  hfs;
  RelDesc rd;

  if (relation.empty() || attrName.empty()) return BADCATPARM;


  ////ensure relation exists
  if ( (status = relCat->getInfo(relation, rd)) != OK ) return status;


  //create scan
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;
 
  //set filter
  status = hfs->startScan(0, MAXNAME , STRING, relation.c_str(), EQ);
  if(status != OK) return status; 

  //loop to find attributes
  bool attrFound = false;
  while( ((status = hfs->scanNext(rid)) != FILEEOF) && (!attrFound)){ 

	  //clean exit in case of error
	  if( status != OK) { 
	  	delete hfs;
	  	return status;
	  }

  	//get record 
  	if( (status = hfs->getRecord(rec)) != OK) { 
  		delete hfs;
  		return status;
  	}

	  //compare relation name & delete if match found
    AttrDesc * recPtr =  (AttrDesc *) rec.data; 
  	if( strcmp(recPtr->attrName, attrName.c_str()) == 0){
	  	hfs->deleteRecord();
	  	attrFound = true;
	  }
  }

  //cleap up
  delete hfs;

  if(!attrFound) return ATTRNOTFOUND;
  
 
	return OK;
 


}


const Status AttrCatalog::getRelInfo(const string & relation, 
				     int &attrCnt,
				     AttrDesc *&attrs)
{
  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;
  RelDesc rd;
  AttrDesc * attrArray;
  int i;

  if (relation.empty()) return BADCATPARM;

  // check that relation exists
  if( (status = relCat->getInfo(relation,rd)) != OK) {
    return status;}; 


  if( rd.attrCnt < 0)	return ATTRNOTFOUND;
   
   //assign stuff 
   attrCnt = rd.attrCnt;
   attrs = new AttrDesc[attrCnt];

  
   
  //start scanning for attributes
  hfs = new HeapFileScan( ATTRCATNAME, status);
  if( status != OK ) {
    delete attrs;
    return status;
  }

  //set filter to match relation name
  status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  if(status != OK) {
    delete attrs;
    delete hfs;
    return status; 
  }


  //fill attribute array
  i = 0;
  while (  i < rd.attrCnt ){

    if ((status = hfs->scanNext(rid)) == FILEEOF){
     break;
    }
       

	  // in case of some other error
	  if( status != OK) { 
		  delete hfs;
      delete attrs;
	  	return status; 
	  } 
		
  	// retrieve record
  	if( (status = hfs->getRecord(rec)) != OK ){
  		delete hfs;
      delete attrs;    
  		return status; // INVALIDSLOTNO or something
  	}


	  //copy over to array
	  memcpy(&attrs[i], rec.data, sizeof(AttrDesc));
	 
    //increment index
    i++;
	}

    
   //clean up
   delete hfs;
   
   return OK;

}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

