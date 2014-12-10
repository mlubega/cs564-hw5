#include "catalog.h"

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

  if (relation.length() >= sizeof rd.relName)
    return NAMETOOLONG;


	// check for duplicate attributes
	for(i = 0; i < attrCnt; i++){
		int j;
		for(j = i; j < attrCnt; j++){
			
			//skip same index comparison
			if(i == j){
				continue;
			}
			//error if duplicate exists
			if( strcmp(attrList[i].attrName, attrList[j].attrName) == 0 ){
				return DUPLATTR;
			}
		}
	
	}


  	//check if relation exists
	status = relCat->getInfo(relation, rd);

	if(status == OK) {return RELEXISTS;}

	if(status = RELNOTFOUND){

		relation.copy(rd.relName, relation.length(), 0);
		rd.attrCnt = attrCnt;
		status = relCat->addInfo(rd);
		if (status != OK) {return status; }


		attrInfo atr;
		offsetCnt = 0;
		for(i=0; i < attrCnt; i++){
			
			atr = attrList[i];
			memcpy(atr.relName, ad.relName,MAXNAME);
			memcpy(atr.attrName, ad.attrName,MAXNAME);
			ad.attrType = atr.attrType;
		    ad.attrLen = atr.attrLen;	
			ad.attrOffset = offsetCnt;

			status = attrCat->addInfo(ad);
			if (status != OK) {return status; }

			offsetCnt += atr.attrLen; //update offsetCnt for later
		

		}

		status = createHeapFile(relation);
		if(status != OK) {return status; }
	}

	return OK;




}

