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
  hfs = new HeapFileScan( relation , status);
  if(status != OK) {return status; }
   
  //set filter
  status = hfs->startScan(0, MAXSTRINGLEN, STRING, relation.c_str(), EQ);
  if(status != OK) {
    delete hfs;
    return status;
  }

  //find and retrieve record
  status = hfs->scanNext(rid);

  if( status == FILEEOF) {
    delete hfs;
    return RELNOTFOUND;
  }
  if(status != OK) {
    delete hfs;
    return status;
  }
 

  status = hfs->getRecord(rec);
  if(status != OK) {
    delete hfs;
    return status;
  }

  //copy data
  memcpy(&record, &rec, sizeof(RelDesc));
 
  //clean up
  status = hfs->endScan();
  if(status != OK) {
    delete hfs;
    return status;
  }

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
  //memcpy(rec.data, &record, sizeof(RelDesc));
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
  hfs = new HeapFileScan(relation, status);
  if(status != OK) {return status; }
  
  //set filter
  if((status = hfs->startScan(0, MAXSTRINGLEN, STRING, relation.c_str(), EQ)) != OK){
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

  if(( status = hfs->endScan()) != OK) {
    delete hfs;
    return status; }

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
  if( (status = relCat->getInfo(relation, relRecord)) != OK ) {return status;}

  if( relRecord.attrCnt <= 0 ){ return ATTRNOTFOUND; }

  //get list of records
  AttrDesc * attrList;
  int attrCnt; 
  if( (status = attrCat->getRelInfo(relation, attrCnt, attrList)) != OK ) {return status;}


  //search through list of returned records
  int i; 
  for (i = 0; i < attrCnt; i++){
      
	  char * attrNameCpy;
	  attrName.copy(attrNameCpy, attrName.length(), 0);

	   
	  if(strcmp( attrNameCpy, (attrList[i].attrName) ) == 0 ){
	  		memcpy( &record, &attrList[i], sizeof(AttrDesc));
			delete attrList;
			return OK;
	  }
  }

  //no matches found
  delete attrList;
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
//  memcpy(rec.data, &record, sizeof(AttrDesc));
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
 
  //ensure relation exists
  if ( (status = relCat->getInfo(relation, rd)) != OK ){ return status;}

  //create scan
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) {return status;}
 
  //set filter
  status = hfs->startScan(0, MAXSTRINGLEN , STRING, attrName.c_str(), EQ);
  if(status != OK) {return status; }

  
  //loop to find attributes
  bool attrFound = false;
  while(!attrFound){

   if( (status = hfs->scanNext(rid)) == FILEEOF){ break; }
	
	  //clean exit in case of error
	  if( status != OK) { 
	  	hfs->endScan();
	  	delete hfs;
	  	return status;
	  }

  	//get record or clean exit in case of error
  	if( (status = hfs->getRecord(rec)) != OK) { 
  		hfs->endScan();
  		delete hfs;
  		return status;
  	}

	  //compare relation name
    AttrDesc * tmpAttrPtr =  (AttrDesc *) rec.data; 
    char * tmpRelName;
	  relation.copy(tmpRelName, relation.length(), 0);

  	if( strcmp(tmpRelName, tmpAttrPtr->relName) == 0){
	  	hfs->deleteRecord();
	  	attrFound = true;
	  }
  }

  //cleap up
  hfs->endScan();
  delete hfs;

  if(!attrFound){
    return ATTRNOTFOUND;
  }
 
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
  if( (status = relCat->getInfo(relation,rd)) != OK) {return status;}; 

  //create array for attributes
  if( rd.attrCnt > 0){
  		attrs = new AttrDesc[attrCnt];
  }else{
  		return ATTRNOTFOUND;
  }

  //start scanning for attributes
  hfs = new HeapFileScan( ATTRCATNAME, status);
  if( status != OK ) {return status;}
 
  status = hfs->startScan(0, int(relation.length()), STRING, relation.c_str(), EQ);
  if(status != OK) {return status; }


  //fill attribute array
  i = 0;  
  while ( i < rd.attrCnt){

	  //find a record
    status = hfs->scanNext(rid);
    if( status == FILEEOF){ break; }
	
	  // in case of some other error
	  if( status != OK) { 
		  hfs->endScan();
		  delete hfs;
	  	return status; 
	  } 
		
  	// retrieve record
  	if( (status = hfs->getRecord(rec)) != OK ){
  		hfs->endScan();
  		delete hfs;		
  		return status; // INVALIDSLOTNO or something
  	}

	  //copy over to array
	  memcpy(&attrArray[i], rec.data, rec.length);
	
	  //increment array index
	  i++;
	}

    
   //clean up
   hfs->endScan();
   delete hfs;

   //return values
   attrCnt = rd.attrCnt;
   attrs = attrArray;
   return OK;

}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

