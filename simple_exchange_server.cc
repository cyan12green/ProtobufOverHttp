#include <httpserver.hpp>
#include <string>

#include "cmake/build/simple_exchange.pb.h"
using namespace httpserver;
using simpleexchange::SimpleExample;

class test_resource : public http_resource {
  public:
    std::shared_ptr<http_response> render(const http_request& req) {
      SimpleExample simple_example;
      simple_example.ParseFromString(std::string(req.get_arg("protobuf")));
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
