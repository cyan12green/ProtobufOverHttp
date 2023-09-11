#include <cstring>
#include <fstream>
#include <cstdlib>


#include <iostream>
#include <memory>
#include <ostream>
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
const uint64_t KEY_ID = 56;

using simpleexchange::ResponseProto;

//populates the protobuf with the encrypted payload
int generateResponse(const std::string payload, ResponseProto* response_proto ) 
{ 
  //TODO: Fetch key from kms

  //encrypting payload
  int length = payload.length();
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
  input  = (byte*) malloc(sz);
  output = (byte*) malloc(sz);

  std::memcpy((char*) input, payload.c_str(), sz);
  
  for (i = payload.length(); i<length; i++) 
    input[i] = padcounter;
  
  Aes enc;

  if ( wc_AesSetKey(&enc, KEY, AES_BLOCK_SIZE, NULL, AES_ENCRYPTION ) != 0)
    return EXIT_FAILURE; //std::cout<<"failed to set key\n";
  if ( wc_AesCbcEncrypt(&enc, output, input, sz) != 0)
    return EXIT_FAILURE; //std::cout<<"encryption failed\n";
  
  std::string encrypted_payload = std::string( reinterpret_cast<char const* >(output), length);
  
  response_proto->set_encrypted_payload(encrypted_payload);
  response_proto->set_key_number(KEY_ID);
  
  std::cout << "Encryption successful\n";
  return EXIT_SUCCESS;
}

//decrypts the payload contained in protobuf and outputs a string
std::string decryptResponse(const ResponseProto response_proto) 
{ 
  //TODO get key
  
  
  std::string cipher = std::string(response_proto.encrypted_payload());
  //decryption
  Aes dec; 

  byte* input;
  byte* output;

  int i = 0;
  int length = cipher.length();
  int sz = length * sizeof(byte); 


  input = (byte*) malloc(sz);
  output = (byte*) malloc(sz);

  std::memcpy((char *)input, cipher.c_str(), sz);
  
  if (wc_AesSetKey(&dec, KEY, AES_BLOCK_SIZE, NULL, AES_DECRYPTION ) != 0)
    std::cout << "failed to set key\n";

  if (wc_AesCbcDecrypt(&dec, output, input, sz ) != 0) 
  { 
    std::cout << "decryption failed\n";
    return NULL;
  }
  
  //getting rid of padding
  length -= output[length-1];

  std::cout << "decryption successful\n";
  return std::string( reinterpret_cast< char const* >(output), length); 
}

int main(int argc, char** argv) { 
  
  //create the protobuf message
  std::string data, payload;
  ResponseProto response_proto, temp;
  if (generateResponse("asset/url/media.mpeg", &response_proto) != 0)
    return EXIT_FAILURE;
  
  //data contains the protobuf message in a string form
  response_proto.SerializeToString(&data);
  
  //temp is initialized
  temp.ParseFromString(data);
  
  payload = decryptResponse(temp);
  std::cout << payload <<std::endl;  
  

  //Sending over http
  std::string url = "http://localhost:8080/temp";

  try {
  
  curlpp::Cleanup cleaner;
  curlpp::Easy request;
  request.setOpt(new curlpp::options::Url(url));
  request.setOpt(new curlpp::options::Verbose(true));
  curlpp::Forms formParts;
  formParts.push_back(new curlpp::FormParts::Content("protobuf", data));
  request.setOpt(new curlpp::options::HttpPost(formParts)); 
  std::cout << request << std::endl;
  return EXIT_SUCCESS;
  
  }
  catch (curlpp::RuntimeError & e ) {
    std::cout << e.what() << std::endl;
  }

  return EXIT_FAILURE;
} 
