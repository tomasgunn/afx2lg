// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axe_http/server.h"

#include <algorithm>
#include <iostream>

using std::placeholders::_1;

static const size_t kMaxConnections = (FD_SETSIZE - 2);

Server::Server() : quit_(false) {}

Server::~Server() {}

bool Server::Initialize(uint16_t port) {
  ASSERT(port);

  // TODO: also support favicon.ico!
  request_map_.insert(
      std::make_pair("/quit", std::bind(&Server::OnQuit, this, _1)));

  // Should we bind to only the local IP (127.0.0.1) to avoid connections
  // from other machines?
  if (!listener_.Create() || !listener_.Listen(port))
    return false;

  return true;
}

void Server::Run() {
  quit_ = false;
  while (!quit_) {
    // TODO: Also check for bad sockets.
    fd_set readable;
    FD_ZERO(&readable);
    if (listener_.valid())
      FD_SET(listener_.socket(), &readable);

    for (auto& s: sockets_) {
      FD_SET(s->socket(), &readable);
    }

    timeval timeout = { 10, 0 };
    if (select(FD_SETSIZE, &readable, NULL, NULL, &timeout) == SOCKET_ERROR) {
      std::cerr << "select failed\n";
      break;
    }

    for (auto& s: sockets_) {
      SOCKET sock = s->socket();
      if (FD_ISSET(sock, &readable)) {
        OnRead(s);
        // Check if the socket was closed.
        if (sock != s->socket()) {
          FD_CLR(sock, &readable);
        }
      }
    }

    if (quit_ && listener_.valid()) {
      FD_CLR(listener_.socket(), &readable);
      listener_.Close();
    } else if (FD_ISSET(listener_.socket(), &readable)) {
      OnAccept();
    }

    GarbageCollect();
  }
}

void Server::OnAccept() {
  unique_ptr<DataSocket> s(listener_.Accept());
  if (sockets_.size() >= kMaxConnections) {
    std::cerr << "Error: Connection limit reached\n";
  } else {
    sockets_.push_back(std::move(s));
  }
}

void Server::OnRead(const unique_ptr<DataSocket>& s) {
  if (!s->OnDataAvailable()) {
    s->Close();
  } else if (s->request_received()) {
    OnRequest(s);
  }
}

void Server::OnRequest(const unique_ptr<DataSocket>& s) {
  ASSERT(s->request_received());
  std::cout << s->request_path() << std::endl;
  auto f = request_map_.find(s->request_path());
  if (f == request_map_.end()) {
    s->Send("404 Not Found", true, "text/html", "",
            "<html><body>Sorry, no such thing</body></html>");
  } else {
    f->second(s);
  }
}

void Server::OnQuit(const unique_ptr<DataSocket>& s) {
  s->Send("200 OK", true, "text/html", "",
          "<html><body>Quitting...</body></html>");
  quit_ = true;
}

void Server::GarbageCollect() {
  for (auto it = sockets_.begin(); it != sockets_.end(); ++it) {
    if (!(*it)->valid()) {
      it = sockets_.erase(it);
      if (it == sockets_.end())
        break;
    }
  }
}
