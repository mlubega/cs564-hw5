#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for
// the user. If no relation is given (relation.empty() is true), then
// it lists all the relations in the database, along with the width in
// bytes of the relation, the number of attributes in the relation,
// and the number of attributes that are indexed.  If a relation is
// given, then it lists all of the attributes of the relation, as well
// as its type, length, and offset, whether it's indexed or not, and
// its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt, i;

  if (relation.empty()) return UT_Print(RELCATNAME);

   //get relation
   if( (status = relCat->getInfo(relation, rd)) != OK){return status;}

   //get list of attributes
   if( (status = attrCat->getRelInfo(relation, attrCnt, attrs)) != OK){return status;}

   cout << relation << endl;
   printf( "NUM | Attribute Name | Type | Length | Offset | Indexed? | Index # ");
   printf( "-------------------------------------------------------------------" );

   for(i = 0; i < attrCnt; i ++){
	   AttrDesc attr = attrs[i];
 	  printf(" %d | %s | %s | %d | %d ", i, attr.attrName, attr.attrType, attr.attrLen, attr.attrOffset);
   
   }

  return OK;
}
