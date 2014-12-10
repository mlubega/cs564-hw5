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

  hfs = new HeapFileScan( relation , status);
  if(status != OK) {return status; }
   

  status = hfs->startScan(0, int(relation.length()), STRING, relation.c_str(), EQ);
  if(status != OK) {return status; }

  status = hfs->scanNext(rid);

  if( status == FILEEOF) {return RELNOTFOUND;}
  if(status != OK) {return status; }
 

  status = hfs->getRecord(rec);
  if(status != OK) {return status; }

  memcpy(&record, &rec, rec.length);

  status = hfs->endScan();
  if(status != OK) {return status; }

  return OK;

}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;

  Record rec;
  memcpy(rec.data, &record, sizeof(RelDesc));
  rec.length = sizeof(RelDesc); 

  ifs = new InsertFileScan( RELCATNAME , status);  
  if(status != OK) {
	  
	  delete ifs;
	  return status; }

  status = ifs->insertRecord(rec, rid);
  if(status != OK) {
	  
	  delete ifs;
	  return status;
  }
  
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
  
  
  if((status = hfs->startScan(0, int(relation.length()), STRING, relation.c_str(), EQ)) != OK){return status; }

  if((status = hfs->scanNext(rid)) != OK) {return status; }
 
  // if( (status = hfs->getRecord(rec)) != OK) {return status; }
  if( (status = hfs->deleteRecord()) != OK) {return status; }

  if(( status = hfs->endScan()) != OK) {return status; }

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


  ifs = new InsertFileScan(ATTRCATNAME, status);
  if( status != OK) {
	  delete ifs;
	  return status;
  }
 
  //store data
  memcpy(rec.data, &record, sizeof(AttrDesc));
  rec.length = record.attrLen;

  //insert new record
  if( (status = ifs->insertRecord(rec, rid)) != OK) {
	  delete ifs;
	  return OK;
 }
  
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
 if ( (status = relCat->getInfo(relation, rd)) != OK){ return OK;}

 hfs = new HeapFileScan(ATTRCATNAME, status);
 if (status != OK) {return status;}
 
  status = hfs->startScan(0, int(attrName.length()), STRING, attrName.c_str(), EQ);
  if(status != OK) {return status; }

  
  bool attrFound = false;


  //loop to find attributes
  while(!attrFound){

  	status = hfs->scanNext(rid);

  	if( status == FILEEOF){ break; }
	
	//clean exit in case of error
	if( status != OK) { 
		hfs->endScan();
		delete hfs;
		return status;
	}

	//get record or clean exit in case of error
	status = hfs->getRecord(rec);
    if( status != OK) { 
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
  i = 0; //index for attrArray
 
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

