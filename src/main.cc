#include <exception>  
#include <iostream>
#include "GBTEngine.h"
#include "TestGeoQuery.h"

int main(int args, char *argv[])
{
    try
    {
        if (args != 4)
        {
      	  std::cerr << "Usage: " << argv[0] << " table_name input_file query_type[point | range | nearest]." << std::endl;
      	  return -1;
        }
        uint32_t query_type = 0;
        if (strcmp(argv[3], "point") == 0) query_type = 0;
        else if (strcmp(argv[3], "range") == 0) query_type = 1;
        else if (strcmp(argv[3], "nearest") == 0) query_type = 2;
        else
        {
      	  std::cerr << "Unknown query type." << std::endl;
      	  return -1;
        }

        if(query_type == 0)
      	  TestPointSelect(argv[1], argv[2]);
        else if(query_type == 1)
      	  TestRangeQuery(argv[1], argv[2]);
        else if(query_type == 2)
      	  TestNearestQuery(argv[1], argv[2]);
    }
    catch (std::exception& e)
    {
        std::cerr << "******ERROR******" << std::endl;
        std::string s = e.what();
        std::cerr << s << std::endl;
        return -1;
    }
    
    return 0;
}
