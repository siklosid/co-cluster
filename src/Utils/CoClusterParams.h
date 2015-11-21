#ifndef QUERY_ENGINE_PARAMS_H
#define QUERY_ENGINE_PARAMS_H

// define PARFILE include params.h elott kell, hogy legyen
#define PARFILE     "../Def/co-cluster.def"
#include <string>
#include <vector>

#include "params.h"

using std::string;
using std::vector;

class ParamsFuncs {
 public:
  static void split_parameter(const string &parameter,
			      vector< string > *parameter_vect) {
    
    size_t start_pos = 0;
    size_t end_pos = 0;
    int num = 0;

    while ((end_pos = string(parameter).find("|", start_pos)) != string::npos) {
      log_dbg("parameter: %s, start_pos: %d, end_pos: %d", parameter.c_str(), start_pos, end_pos);
      string input_file = string(parameter).substr(start_pos, 
						   end_pos - start_pos);
      parameter_vect->push_back(input_file);
      
      start_pos = end_pos+1;
      num++;
    }
    
    string input_file = 
      string(parameter).substr(start_pos, string(parameter).length());
    parameter_vect->push_back(input_file);  
  }
};
                                                         
#endif // !QUERY_ENGINE_PARAMS_H
