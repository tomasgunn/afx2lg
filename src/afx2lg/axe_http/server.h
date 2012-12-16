// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef AXE_HTTP_SERVER_H_
#define AXE_HTTP_SERVER_H_

#include "axe_http/socket.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

class Server {
 public:
  Server();
  ~Server();

  bool Initialize(uint16_t port);

  // Runs the select loop.
  void Run();

 private:
  void OnAccept();
  void OnRead(const unique_ptr<DataSocket>& s);

  // Called when we've received a full HTTP request (all headers are ready).
  void OnRequest(const unique_ptr<DataSocket>& s);

  void OnQuit(const unique_ptr<DataSocket>& s);

  // Goes through all sockets and deletes the closed ones.
  void GarbageCollect();

  typedef std::vector<unique_ptr<DataSocket> > SocketArray;
  typedef
      std::map<
          std::string,
          std::function<void(const unique_ptr<DataSocket>&)> > RequestMap;

  ListeningSocket listener_;
  SocketArray sockets_;
  RequestMap request_map_;
  bool quit_;
};

#endif  // AXE_HTTP_SERVER_H_
