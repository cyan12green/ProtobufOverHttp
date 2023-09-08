#include <httpserver.hpp>
#include <string>

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/pkcs7.h>

#include "cmake/build/simple_exchange.pb.h"
using namespace httpserver;
using simpleexchange::SimpleExample;

typedef unsigned char byte;
typedef unsigned int word32;

const byte KEY[16] = "test";


class test_resource : public http_resource {
  public:
    
    std::shared_ptr<http_response> render(const http_request& req) {
      
      std::string cipher = std::string(req.get_arg("protobuf")); 
      
      //decryption
      Aes dec; 
      
      byte* input;
      byte* output;

      int i = 0;
      int length = cipher.length();
      int sz = length * sizeof(byte); 
      
      
      input = (byte*) malloc(sz);
      output = (byte*) malloc(sz);
      
      std::strcpy((char *)input, cipher.c_str());
      
      if (wc_AesSetKey(&dec, KEY, AES_BLOCK_SIZE, NULL, AES_DECRYPTION ) != 0)
        std::cout << "failed to set key\n";
      
      if (wc_AesCbcDecrypt(&dec, output, input, sz ) != 0)
        //std::cout << "decryption failed\n";
      //else    
        //std::cout <<"decryption successful\n";
      ;
      
      //getting rid of padding
      length -= output[length-1];
      std::string decryptedData = std::string( reinterpret_cast< char const* >(output), length);

      SimpleExample simple_example;
      simple_example.ParseFromString(decryptedData);
      return std::shared_ptr<http_response>(new string_response("Protobuf text: " + std::string(simple_example.text())));
    }
};

int main(int argc, char** argv) {
  webserver ws = create_webserver(8080);

  test_resource hwr;
  ws.register_resource("/temp", &hwr);
  ws.start(true);

  return 0;
}
