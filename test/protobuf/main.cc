#include "test.pb.h"
#include <iostream>
#include <string>

int main() {
    /*
    fixbug::LoginRequest request;
    request.set_name("hpz");
    fixbug::LoginResponse response;
    auto rc = response.mutable_result();
    rc->set_errcode(0);
    rc->set_errmsg("登录成功");
    */
    fixbug::GetFriendListRequest request;
    request.set_userid(1000);

    fixbug::GetFriendListResponse response;

    auto rc = response.mutable_result();
    rc->set_errcode(0);
    rc->set_errmsg("获取好友列表成功");
    auto friend1 = response.add_friendslist();
    friend1->set_id(1001);
    friend1->set_name("friend1");
    friend1->set_gender(fixbug::User::MALE);
    return 0;
}