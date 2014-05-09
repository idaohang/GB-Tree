 
#include "GBTEngine.h"
#include "TestGeoQuery.h"

int main(int args, char *argv[])
{
  // run the SQL engine taking user commands from standard input (console).
  if(args == 2)
  {
	  if(strcmp(argv[1], "1") == 0)
		  TestPointSelect();
	  else if(strcmp(argv[1], "2") == 0)
		  TestRangeQuery();
	  else if(strcmp(argv[1], "3") == 0)
		  TestNearestQuery();
  }
  return 0;
}
