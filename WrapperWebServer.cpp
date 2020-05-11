#include "WrapperWebServer.h"
#include "Hackiebox.h"

void WrapperWebServer::begin() {  
  _server = new WebServer(80);

  _server->enableCORS(true); //DEV ONLY!
  
  _server->onNotFound([&](){ WrapperWebServer::handleNotFound(); });
  _server->on("/", [&](){ WrapperWebServer::handleRoot(); });
  _server->on("/api/sse", [&](){ WrapperWebServer::handleSse(); });
  _server->on("/api/ajax", [&](){ WrapperWebServer::handleAjax(); });
  _server->begin();
}
void WrapperWebServer::handle(void) {
  _server->handleClient();
}

void WrapperWebServer::handleNotFound(void) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += _server->uri();
  message += "\nMethod: ";
  message += (_server->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += _server->args();
  message += "\n";
  for (uint8_t i=0; i<_server->args(); i++){
    message += " " + _server->argName(i) + ": " + _server->arg(i) + "\n";
  }
  _server->send(404, "text/plain", message);
}

void WrapperWebServer::handleRoot(void) {
  _server->send(200, "text/html", "ROOT");
}
void WrapperWebServer::handleSse(void) {
  _server->send(200, "text/event-stream", "SSE");
  //TODO: Keep alive connection without blocking others
}
void WrapperWebServer::handleAjax(void) {
  String cmd;
  String param1;
  String param2;

  for (uint8_t i=0; i<_server->args(); i++) {
    if (_server->argName(i).equals("cmd")) {
      cmd = _server->arg(i);
    } else if (_server->argName(i).equals("param1")) {
      param1 = _server->arg(i);
    } else if (_server->argName(i).equals("param2")) {
      param2 = _server->arg(i);
    }
  }

  if (cmd) {
    if (cmd.equals("get-config")) {
      _server->send(200, "text/json", Config.getAsJson());
      return;
    } else if (cmd.equals("get-dir")) {
      if (!param1)
        param1 = String();
      _server->send(200, "text/json", Box.boxSD.jsonListDir((char*)param1.c_str()));
      return;
    } else if (cmd.equals("get-file")) {
      long size = 0;

      if (!param1)
        param1 = String();
      if (param2)
        size = param2.toInt();

      FileFs file;
      if (file.open((char*)param1.c_str(), FA_OPEN_EXISTING | FA_READ)) {
        if (size == 0 || file.fileSize() < size) 
          size = file.fileSize();

        String filename = param1;
        int16_t index = filename.lastIndexOf("/");
        if (index != -1) {
          filename.remove(0, index+1);
        }

        _server->setContentLength(size);
        _server->sendHeader("Content-Disposition", (String("attachment; filename=\"") + filename + String("\"")).c_str());
        _server->send(200, "application/octet-stream", "");

        uint8_t buffer[4096];
        size_t read;
        while (file.curPosition() < size) {
          read = file.read(buffer, sizeof(buffer)); //TODO: may read to much if size is limited
          if (read == 0)
            break;
          if (_server->client().write(buffer, read) == 0) 
            break;
        }

        file.close();
        return;
      }
    }
  }

  handleNotFound();
}
