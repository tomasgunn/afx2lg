// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axe_http/server.h"

#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>

using std::placeholders::_1;

static const size_t kMaxConnections = (FD_SETSIZE - 2);

extern std::string g_process_path;

#ifdef OS_WIN
static const char kPathSeparator = '\\';
#else
static const char kPathSeparator = '/';
#endif

void EnsureTrailingSlash(std::string* path) {
  if (!path->empty() && path->at(path->size() - 1) != kPathSeparator)
    *path += kPathSeparator;
}

void AppendPath(std::string* path, const std::string& sub_path) {
  EnsureTrailingSlash(path);
  path->append(sub_path);
#ifdef OS_WIN
  std::replace(path->begin(), path->end(), '/', kPathSeparator);
#endif
}

void RemoveFilePart(std::string* path) {
  size_t i = path->find_last_of(kPathSeparator);
  if (i != std::string::npos)
    path->erase(i);
}

class FileReader : public AsyncDataReader {
 public:
  FileReader() : bytes_remaining_(0u) {}
  virtual ~FileReader() {}

  bool Initialize(const std::string& path) {
    file_.open(path, std::ios::binary | std::ios::in);
    if (!file_.good())
      return false;

    file_.seekg(0, std::ios::end);
    std::streampos size = file_.tellg();
    file_.seekg(0, std::ios::beg);

    if (size >= INT_MAX)
      return false;

    bytes_remaining_ = static_cast<size_t>(size);

    return file_.good();
  }

  virtual bool GetChunk(uint8_t* buffer, size_t buf_size, size_t* read) {
    if (file_.eof())
      return false;

    file_.read(reinterpret_cast<char*>(buffer), buf_size);

    if (file_.eof()) {
      ASSERT(bytes_remaining_ <= buf_size);
      *read = bytes_remaining_;
      bytes_remaining_ = 0;
    } else {
      *read = buf_size;
      bytes_remaining_ -= buf_size;
    }

    return *read > 0;
  }

  virtual size_t BytesRemaining() {
    return bytes_remaining_;
  }

 private:
  std::ifstream file_;
  size_t bytes_remaining_;
};


Server::Server() : quit_(false) {}

Server::~Server() {}

bool Server::Initialize(uint16_t port) {
  ASSERT(port);

  // TODO: also support favicon.ico!
  request_map_.insert(
      std::make_pair("/quit", std::bind(&Server::OnQuit, this, _1)));

  request_map_.insert(
      std::make_pair(
          "/", std::bind(&Server::OnServeFile, this, "edit.html", _1)));

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
    fd_set readable, writable;
    FD_ZERO(&readable);
    FD_ZERO(&writable);
    if (listener_.valid())
      FD_SET(listener_.socket(), &readable);

    for (auto& s: sockets_) {
      SOCKET sock = s->socket();
      FD_SET(sock, &readable);
      if (s->HasPendingData())
        FD_SET(sock, &writable);
    }

    timeval timeout = { 10, 0 };
    if (select(FD_SETSIZE, &readable, &writable, NULL, &timeout) ==
            SOCKET_ERROR) {
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

      if (FD_ISSET(sock, &writable))
        s->SendPendingData();
    }

    if (!quit_ && FD_ISSET(listener_.socket(), &readable))
      OnAccept();

    GarbageCollect();
  }
}

std::string Server::RequestPathToLocalPath(const std::string& path) {
  // TODO: Support a way to specify the server's working directory?
  std::string ret("C:\\src\\tommi\\src\\afx2lg\\axe_http\\html\\");
  ret += path;
  std::replace(ret.begin(), ret.end(), '/', '\\');
  return ret;
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

void Server::OnServeFile(const std::string& path,
                         const unique_ptr<DataSocket>& s) {
  unique_ptr<FileReader> reader(new FileReader());
  if (!reader->Initialize(RequestPathToLocalPath(path))) {
    s->Send("500 Server Error", true, "text/html", "",
            "<html><body>Failed to open local file.</body></html>");
  } else {
    s->SendHeaders("200 OK", true, reader->BytesRemaining(), "text/html", "");
    s->QueueData(std::move(reader));
  }
}
