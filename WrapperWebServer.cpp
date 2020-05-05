#include "WrapperWebServer.h"

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
  //message += (_server->method() == HTTP_GET)?"GET":"POST";
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
}
void WrapperWebServer::handleAjax(void) {
  _server->send(200, "text/html", "AJAX");
}
