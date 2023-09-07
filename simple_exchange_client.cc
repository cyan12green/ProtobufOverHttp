#include <fstream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "cmake/build/simple_exchange.pb.h"

using simpleexchange::SimpleExample;
int main(int argc, char** argv) { 
  
  //create the protobuf message
  std::string data;
  SimpleExample simple_example;
  simple_example.set_id(74);
  simple_example.set_text("sample test sent over http"); 
  //data contains the protobuf message in a string form
  simple_example.SerializeToString(&data);

  std::string url = "http://localhost:8080/temp";

  try {
  
  curlpp::Cleanup cleaner;
  curlpp::Easy request;
  request.setOpt(new curlpp::options::Url(url));
  request.setOpt(new curlpp::options::Verbose(true));
  curlpp::Forms formParts;
  formParts.push_back(new curlpp::FormParts::Content("protobuf", data));
  request.setOpt(new curlpp::options::HttpPost(formParts)); 
  std::cout << request << std::endl << request << std::endl;
  return EXIT_SUCCESS;
  
  }
  catch (curlpp::RuntimeError & e ) {
    std::cout << e.what() << std::endl;
  }

  return EXIT_FAILURE;
} 
