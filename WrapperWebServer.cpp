#include "WrapperWebServer.h"
#include "Hackiebox.h"

#include <SLFS.h>

void WrapperWebServer::begin() {  
  _server = new WebServer(80);

  _server->enableCORS(true); //DEV ONLY!

  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  _server->onNotFound([&](){ WrapperWebServer::handleNotFound(); });
  _server->on("/", HTTP_GET, [&](){ WrapperWebServer::handleRoot(); });
  _server->on("/api/sse", HTTP_GET, [&](){ WrapperWebServer::handleSse(); });
  _server->on("/api/ajax", HTTP_GET, [&](){ WrapperWebServer::handleAjax(); });
  _server->on("/api/upload/file", HTTP_POST, [&](){ }, [&](){ WrapperWebServer::handleUploadFile(); });
  _server->on("/api/upload/flash-file", HTTP_POST, [&](){ WrapperWebServer::handleUploadFlashFile(); });
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
  String cmd = _server->arg("cmd");
  String param1 = _server->arg("param1");
  String param2 = _server->arg("param2");

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
        if (index != -1)
          filename.remove(0, index+1);

        _server->setContentLength(size);
        _server->sendHeader("Content-Disposition", (String("attachment; filename=\"") + filename + String("\"")).c_str());
        _server->send(200, "application/octet-stream", "");

        uint8_t buffer[4096];
        size_t read;

        while (file.curPosition() < size) {
          read = file.read(buffer, sizeof(buffer)); //TODO: may read to much if size is limited
          if (read == 0)
            break; //error or empty
          if (_server->client().write(buffer, read) == 0) 
            break; //error
        }

        file.close();
        return;
      } else {
        Log.error("Could not open %s", (char*)param1.c_str());
      }
    } else if (cmd.equals("get-flash-file")) {
      long size = 0;

      if (!param1)
        param1 = String();
      if (param2)
        size = param2.toInt();

      if (SerFlash.open((char*)param1.c_str(), FS_MODE_OPEN_READ) == SL_FS_OK) {
        if (size == 0) 
          size = SerFlash.size();
        
        String filename = param1;
        int16_t index = filename.lastIndexOf("/");
        if (index != -1) 
          filename.remove(0, index+1);

        _server->setContentLength(size);
        _server->sendHeader("Content-Disposition", (String("attachment; filename=\"") + filename + String("\"")).c_str());
        _server->send(200, "application/octet-stream", "");

        uint8_t buffer[1024];
        size_t read = -1;
        while (SerFlash.available()) {
          read = SerFlash.readBytes(buffer, sizeof(buffer)); //TODO: may read to much if size is limited
          if (read == 0)
            break; //error or empty
          if (_server->client().write(buffer, read) == 0) 
            break; //error
        }

        SerFlash.close();
        return;
      } else {
        Log.error("Could not open %s, error %s", (char*)param1.c_str(), SerFlash.lastErrorString());
      }
    } else if (cmd.equals("set-file")) {
    } else if (cmd.equals("move-file")) {
    }
  }
  handleNotFound();
}

void WrapperWebServer::handleUploadFile() {
  HTTPUpload& upload = _server->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = _server->arg("filename");
    bool overwrite = false;
    if (!_server->arg("overwrite").equals("")) {
      overwrite = true;
    }

    if (!filename.startsWith("/"))
      filename = "/" + filename;
    Log.info("handleUploadFile Name: %s, overwrite: %T", (char*)filename.c_str(), overwrite);

    uint8_t filemode = FA_CREATE_NEW | FA_WRITE;
    if (overwrite)
      filemode = FA_CREATE_ALWAYS | FA_WRITE;

    _uploadFileOpen = _uploadFile.open((char*)filename.c_str(), filemode);
    if (_uploadFileOpen)
      return;
    Log.error("File could not be opened.");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Log.verbose("handleUploadFile Data: %i", upload.currentSize);
    if (_uploadFileOpen) {
      _uploadFile.write(upload.buf, upload.currentSize);
      return;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (_uploadFileOpen) {
      _uploadFile.close();
      Log.info("handleUploadFile Size: %ikB", upload.totalSize / 1024);
      _server->send(200, "text/html", "{\"success\":true}");
      return;
    }
  }
  handleNotFound();
}

void WrapperWebServer::handleUploadFlashFile() {
  handleNotFound(); //TBD
}