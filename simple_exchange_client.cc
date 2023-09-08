#include <cstring>
#include <fstream>
#include <cstdlib>


#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/pkcs7.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "cmake/build/simple_exchange.pb.h"

typedef unsigned char byte;
typedef unsigned int word32;

const byte KEY[16] = "test";

using simpleexchange::SimpleExample;
int main(int argc, char** argv) { 
  
  //create the protobuf message
  std::string data;
  SimpleExample simple_example;
  simple_example.set_id(74);
  simple_example.set_text("sample test sent over http and then a lot of random text to check if this works"); 
  //data contains the protobuf message in a string form
  simple_example.SerializeToString(&data);
  
  
  //encrypting data
  int length = data.length();
  int sz;  
  int padcounter = 0;
  int i = 0;

  byte* input;
  byte* output;
  
  while (length % AES_BLOCK_SIZE !=0) { 
    length++;
    padcounter++;
  }
  
  sz = length * sizeof(byte);
  input = (byte*) malloc(sz);
  output = (byte*) malloc(sz);

  std::strcpy((char*) input, data.c_str());
  
  for (i = data.length(); i<length; i++) 
    input[i] = padcounter;
  
  Aes enc;

  if ( wc_AesSetKey(&enc, KEY, AES_BLOCK_SIZE, NULL, AES_ENCRYPTION ) != 0)
    std::cout<<"failed to set key\n";
  if ( wc_AesCbcEncrypt(&enc, output, input, sz) != 0)
  /*  std::cout<<"encryption failed\n";
  else
    std::cout << "encryption successful\n";
  */;
  std::string encryptedData = std::string( reinterpret_cast<char const* >(output), length);
  std::cout << "encrypted data sent over http: " << encryptedData << std::endl; 
  std::string url = "http://localhost:8080/temp";

  try {
  
  curlpp::Cleanup cleaner;
  curlpp::Easy request;
  request.setOpt(new curlpp::options::Url(url));
  request.setOpt(new curlpp::options::Verbose(true));
  curlpp::Forms formParts;
  formParts.push_back(new curlpp::FormParts::Content("protobuf", encryptedData));
  request.setOpt(new curlpp::options::HttpPost(formParts)); 
  std::cout << request << std::endl;
  return EXIT_SUCCESS;
  
  }
  catch (curlpp::RuntimeError & e ) {
    std::cout << e.what() << std::endl;
  }

  return EXIT_FAILURE;
} 
