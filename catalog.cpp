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

  cout << "enter get INfo cool"  << endl;
  //create scan
  hfs = new HeapFileScan( RELCATNAME , status);
  
  
  cout << "create Heapfile Scan no prb " << endl;
  
  if(status != OK) {
    cout << "RelCat::getInfo heapfile creation error" << endl;
    delete hfs;
    return status; }
  


  //set filter
  status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  cout << "start scan cool" << endl;
  if(status != OK) {
    delete hfs;
    cout << "RelCat::getInfo heapfile startScan error" << endl;
    return status;
  }
  
  
  if((status = hfs->scanNext(rid)) == FILEEOF){
    cout << " RelCat::getInfo record not found" << endl; 
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

   memcpy(&record, &rec, sizeof(RelDesc));

 


  delete hfs;
  return  OK;

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
  cout <<" addInfo InsertFileScan error "<< endl;
	  
	  delete ifs;
	  return status;
  }

  //insert record
  status = ifs->insertRecord(rec, rid);
  if(status != OK) {
  cout <<" addInfo insertRecord Error "<< endl;
	  
	  delete ifs;
	  return status;
  }
 
 
 cout << " Rid Page, Slot : " << rid.pageNo << ", " << rid.slotNo << endl;
 
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
  if(status != OK) {return status; }
  
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
/*
  if(( status = hfs->endScan()) != OK) {
    delete hfs;
    return status; }
*/
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

  hfs = new HeapFileScan( ATTRCATNAME, status);
  if( status != OK) { return status;}

  if( (status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ)) != OK) {
    cout << "AttrCatalog::getInfo start scan error" << endl;
    return status;
  }

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

     if( strcmp(tmp->attrName, attrName.c_str()) == 0){
	  		memcpy( &record, rec.data, sizeof(AttrDesc));
        delete hfs;
        return OK;
      }
  
  }

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

    cout << "entered RemoveInfo no prob" << endl;
  if (relation.empty() || attrName.empty()) return BADCATPARM;

  cout << "attrCar::removeInfo valid relation: " << relation << ", attrName: " << attrName <<  endl;

  ////ensure relation exists
  //if ( (status = relCat->getInfo(relation, rd)) != OK ){ return status;}


   cout << "AttrCat::removeInfo  getInfo relName: " << rd.relName << endl;
  //create scan
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) {return status;}
   cout << "AttrCat::removeInfo  created HeapFile" << endl;
 
  //set filter
  status = hfs->startScan(0, MAXNAME , STRING, relation.c_str(), EQ);
  if(status != OK) {return status; }
   cout << "AttrCat::removeInfo  started Scan " << endl;

 int o = 0; 
  //loop to find attributes
  bool attrFound = false;
  while(!attrFound){

   if( (status = hfs->scanNext(rid)) == FILEEOF){ break; }
	    cout << "attrCat::removeInfo iter # " << o++ << endl;

	  //clean exit in case of error
	  if( status != OK) { 
	  	hfs->endScan();
	  	delete hfs;
	  	return status;
	  }

  	//get record or clean exit in case of error
  	if( (status = hfs->getRecord(rec)) != OK) { 
  //		hfs->endScan();
  		delete hfs;
  		return status;
  	}

      cout << "rec found" << endl;
	  //compare relation name
    AttrDesc * tmpAttrPtr =  (AttrDesc *) rec.data; 
    char * tmpAttrName;
	  attrName.copy(tmpAttrName, attrName.length(), 0);

  	if( strcmp(tmpAttrName, tmpAttrPtr->attrName) == 0){
      cout << "rec match" << endl;
	  	hfs->deleteRecord();
	  	attrFound = true;
	  }
  }

  //cleap up
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
  if( (status = relCat->getInfo(relation,rd)) != OK) {
    cout << "getRelInfo getInfo error" << endl;
    return status;}; 

  //create array for attributes
  if( rd.attrCnt > 0){
  		attrs = new AttrDesc[attrCnt];
  }else{
      cout << "getRelInfo less than 0 attr  error" << endl;
  		return ATTRNOTFOUND;
  }

   cout << "Successful attrs array creation" << endl;
  
   
   //start scanning for attributes
  hfs = new HeapFileScan( ATTRCATNAME, status);
  if( status != OK ) {
    
    cout << "getRelInfo heapfilescan open  error" << endl;
    delete attrs;
    return status;
  }

  cout << "Sucessful HeapFileScan Creation " << endl;
 
  status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  if(status != OK) {
    cout << "getRelInfo startScan error"  << endl;
    delete attrs;
    delete hfs;
    return status; 
  }

  cout << "AttrCat::getRelInfo attrCnt " << rd.attrCnt << endl;

  //fill attribute array
  i= 0;
  while (  i < rd.attrCnt){
       cout << " Scan iteration # " << i << endl;

    if ((status = hfs->scanNext(rid)) == FILEEOF){
     cout << "reached end of file" << endl;
     break;
    }
       

	  // in case of some other error
	  if( status != OK) { 
      cout << "getRelInfo  scanNExt other  error" << endl;
		  delete hfs;
      delete attrs;
	  	return status; 
	  } 
		
  	// retrieve record
  	if( (status = hfs->getRecord(rec)) != OK ){
      cout << "getRelInfo  getRecord  error" << endl;
  		delete hfs;
      delete attrs;    
  		return status; // INVALIDSLOTNO or something
  	}


     cout << " Pre mem copy " << endl;
	  //copy over to array
	  memcpy(&attrs[i], rec.data, sizeof(AttrDesc));
	 
    cout << "Post mem copy" << endl;
    //increment index
    i++;
	}

  cout << "Post Searching stuff" << endl;
    
   //clean up
  delete hfs;

     cout << " post delete hfs " << endl;
   
   //return values
   attrCnt = rd.attrCnt;

   cout << "post attrCnt assignment " << endl;
  // attrs = attrArray;

   cout << " post attr array assignment " << endl;
   return OK;

}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

