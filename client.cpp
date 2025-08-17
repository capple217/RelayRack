#include <iostream>

#include "messenger.hpp"
#include "net_client.hpp"
#include "net_connection.hpp"

enum class CustomMsgTypes : uint32_t {

  FireBullet,
  MovePlayer
};

class CustomClient : public olc::net::client_interface<CustomMsgTypes> {
public:
  bool FireBullet(float x, float y) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::FireBullet;
    msg << x << y;
  }
};

int main() {

  CustomClient c;
  c.Connect("1.1.1.1", 6000);
  c.FireBullet(2.0f, 5.0f);

  return 0;
}
