#include <chrono>
#include <httpserver.hpp>
#include <string>

#include <thread>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/pkcs7.h>

#include "cmake/build/simple_exchange.pb.h"
using namespace httpserver;
using simpleexchange::ResponseProto;

typedef unsigned char byte;
typedef unsigned int word32;

const byte KEY[16] = "test";

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

class test_resource : public http_resource {
  public:
    
    std::shared_ptr<http_response> render(const http_request& req) {
      
      std::string data = std::string(req.get_arg("protobuf")); 
      std::cout << data << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(2));
      ResponseProto response_proto; 
      response_proto.ParseFromString(data);
      std::cout << response_proto.DebugString() << std::endl; 
      
      std::string payload = decryptResponse(response_proto);
      std::cout << "decrypted payload : " << payload <<std::endl;
      return std::shared_ptr<http_response>(new string_response("Protobuf text: " + data));
    }
};

int main(int argc, char** argv) {
  webserver ws = create_webserver(8080);

  test_resource hwr;
  ws.register_resource("/temp", &hwr);
  ws.start(true);

  return 0;
}
