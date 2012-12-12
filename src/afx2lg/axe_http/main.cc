// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/common_types.h"

#include "axe_http/socket.h"

#include <iostream>
#include <vector>

static const size_t kMaxConnections = (FD_SETSIZE - 2);

void HandleBrowserRequest(DataSocket* ds, bool* quit) {
  ASSERT(ds && ds->valid());
  ASSERT(quit);

  const std::string& path = ds->request_path();

  *quit = (path.compare("/quit") == 0);

  if (*quit) {
    ds->Send("200 OK", true, "text/html", "",
             "<html><body>Quitting...</body></html>");
  } else if (ds->method() == DataSocket::OPTIONS) {
    // We'll get this when a browsers do cross-resource-sharing requests.
    // The headers to allow cross-origin script support will be set inside
    // Send.
    ds->Send("200 OK", true, "", "", "");
  } else {
    // Here we could write some useful output back to the browser depending on
    // the path.
    std::cerr << "Invalid request path: " << ds->request_path() << "\n";
    ds->Send("500 Sorry", true, "text/html", "",
             "<html><body>Sorry, not yet implemented</body></html>");
  }
}

int main(int argc, char** argv) {
  // TODO(tommi): make configurable.
  static const unsigned short port = 8888;

  ListeningSocket listener;
  if (!listener.Create()) {
    std::cerr << "Failed to create server socket\n";
    return -1;
  } else if (!listener.Listen(port)) {
    std::cerr << "Failed to listen on server socket\n";
    return -1;
  }

  std::cout << "Server listening on port " << port << std::endl;

  typedef std::vector<DataSocket*> SocketArray;
  SocketArray sockets;
  bool quit = false;
  while (!quit) {
    // TODO: Also check for bad sockets.
    fd_set socket_set;
    FD_ZERO(&socket_set);
    if (listener.valid()) {
      FD_SET(listener.socket(), &socket_set);
    }

    for (SocketArray::iterator i = sockets.begin(); i != sockets.end(); ++i)
      FD_SET((*i)->socket(), &socket_set);

    struct timeval timeout = { 10, 0 };
    if (select(FD_SETSIZE, &socket_set, NULL, NULL, &timeout) == SOCKET_ERROR) {
      std::cerr << "select failed\n";
      break;
    }

    for (SocketArray::iterator i = sockets.begin(); i != sockets.end(); ++i) {
      DataSocket* s = *i;
      bool socket_done = true;
      if (FD_ISSET(s->socket(), &socket_set)) {
        if (s->OnDataAvailable(&socket_done) && s->request_received()) {
          std::cout << s->request_path() << std::endl;
          HandleBrowserRequest(s, &quit);
          if (quit) {
            std::cout << "Quitting...\n";
            FD_CLR(listener.socket(), &socket_set);
            listener.Close();
          }
        }
      } else {
        socket_done = false;
      }

      if (socket_done) {
        ASSERT(s->valid());  // Close must not have been called yet.
        FD_CLR(s->socket(), &socket_set);
        delete (*i);
        i = sockets.erase(i);
        if (i == sockets.end())
          break;
      }
    }

    if (FD_ISSET(listener.socket(), &socket_set)) {
      DataSocket* s = listener.Accept();
      if (sockets.size() >= kMaxConnections) {
        delete s;  // sorry, that's all we can take.
        std::cerr << "Connection limit reached\n";
      } else {
        sockets.push_back(s);
      }
    }
  }

  for (SocketArray::iterator i = sockets.begin(); i != sockets.end(); ++i)
    delete (*i);
  sockets.clear();

  return 0;
}
